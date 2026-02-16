import 'package:flutter/material.dart';

import 'ble/fake_nexgen_controller.dart';
import 'ble/nexgen_ble.dart';
import 'ble/nexgen_controller.dart';
import 'nexgen_brand.dart';
import 'ui/keypad.dart';

void main() {
  runApp(const NexgenSafeApp());
}

class NexgenSafeApp extends StatelessWidget {
  const NexgenSafeApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Nexgen Safe',
      theme: NexgenBrand.theme(),
      home: const HomeScreen(),
      debugShowCheckedModeBanner: false,
    );
  }
}

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

enum PendingAction { none, lock, unlock, setPinEnter, setPinConfirm }

class _HomeScreenState extends State<HomeScreen> {
  // Demo mode: lets you test UI without hardware.
  bool demoMode = true;

  late NexgenController ble;

  SafeLockState lockState = SafeLockState.unknown;
  BleConnState connState = BleConnState.disconnected;
  String status = '';

  PendingAction action = PendingAction.none;
  String pin = '';
  String newPinFirst = '';

  void _initController() {
    ble = demoMode
        ? FakeNexgenController()
        : RealNexgenController(NexgenBleController());

    ble.connState.listen((s) => setState(() => connState = s));
    ble.lockState.listen((s) => setState(() => lockState = s));
    ble.statusText.listen((t) => setState(() => status = t));
  }

  @override
  void initState() {
    super.initState();
    _initController();
  }

  @override
  void dispose() {
    ble.dispose();
    super.dispose();
  }

  Future<void> _connectFlow() async {
    if (demoMode) {
      // Fake connect (no hardware)
      await ble.connect(BluetoothDevice(remoteId: const DeviceIdentifier('DEMO')));
      return;
    }

    await ble.startScan();

    // naive: take first device that advertises our service
    final sub = ble.scanResults().listen((r) async {
      if (r.advertisementData.serviceUuids
          .map((u) => u.toLowerCase())
          .contains(NexgenBleUuids.service.str.toLowerCase())) {
        await ble.stopScan();
        await ble.connect(r.device);
      }
    });

    await Future<void>.delayed(const Duration(seconds: 7));
    await ble.stopScan();
    await sub.cancel();
  }

  Color _stateColor() {
    switch (lockState) {
      case SafeLockState.locked:
        return Colors.redAccent;
      case SafeLockState.unlocked:
        return Colors.greenAccent;
      case SafeLockState.unknown:
        return Colors.amberAccent;
    }
  }

  String _promptText() {
    if (connState != BleConnState.connected) return 'Connect to a safe';

    switch (action) {
      case PendingAction.none:
        return 'Choose an action';
      case PendingAction.lock:
        return 'Enter PIN to LOCK';
      case PendingAction.unlock:
        return 'Enter PIN to UNLOCK';
      case PendingAction.setPinEnter:
        return 'Enter NEW PIN';
      case PendingAction.setPinConfirm:
        return 'Confirm NEW PIN';
    }
  }

  void _clearEntry() {
    setState(() {
      pin = '';
    });
  }

  void _resetFlow() {
    setState(() {
      action = PendingAction.none;
      pin = '';
      newPinFirst = '';
    });
  }

  Future<void> _submitPinIfReady() async {
    if (pin.length != 4) return;

    try {
      switch (action) {
        case PendingAction.lock:
          await ble.lock(pin);
          _resetFlow();
          return;
        case PendingAction.unlock:
          await ble.unlock(pin);
          _resetFlow();
          return;
        case PendingAction.setPinEnter:
          setState(() {
            newPinFirst = pin;
            pin = '';
            action = PendingAction.setPinConfirm;
          });
          return;
        case PendingAction.setPinConfirm:
          final confirm = pin;
          await ble.setPin(newPinFirst, confirm);
          _resetFlow();
          return;
        case PendingAction.none:
          return;
      }
    } catch (e) {
      // leave flow as-is; status stream will likely show error too
      setState(() {
        status = e.toString();
      });
    }
  }

  String _stateText() {
    switch (lockState) {
      case SafeLockState.locked:
        return 'LOCKED';
      case SafeLockState.unlocked:
        return 'UNLOCKED';
      case SafeLockState.unknown:
        return 'UNKNOWN';
    }
  }

  @override
  Widget build(BuildContext context) {
    final isConnected = connState == BleConnState.connected;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Nexgen Safe'),
        actions: [
          Row(
            children: [
              const Text('Demo', style: TextStyle(fontSize: 12)),
              Switch(
                value: demoMode,
                onChanged: (v) async {
                  await ble.disconnect();
                  ble.dispose();
                  setState(() {
                    demoMode = v;
                    connState = BleConnState.disconnected;
                    lockState = SafeLockState.unknown;
                    status = '';
                    _initController();
                  });
                },
              ),
            ],
          ),
          TextButton(
            onPressed: isConnected ? () => ble.disconnect() : _connectFlow,
            child: Text(
              isConnected ? 'Disconnect' : 'Connect',
              style: const TextStyle(color: Colors.white),
            ),
          ),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Row(
              children: [
                CircleAvatar(radius: 6, backgroundColor: _stateColor()),
                const SizedBox(width: 8),
                Text(
                  _stateText(),
                  style: const TextStyle(fontWeight: FontWeight.w700),
                ),
                const Spacer(),
                Text(connState.name.toUpperCase()),
              ],
            ),
            const SizedBox(height: 12),
            if (status.isNotEmpty)
              Text(
                status,
                style: TextStyle(color: Colors.white.withOpacity(0.75)),
              ),
            const SizedBox(height: 24),

            Container(
              padding: const EdgeInsets.all(16),
              decoration: BoxDecoration(
                borderRadius: BorderRadius.circular(16),
                color: Colors.white.withOpacity(0.06),
                border: Border.all(color: Colors.white.withOpacity(0.08)),
              ),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  Text(
                    _promptText(),
                    style: TextStyle(color: Colors.white.withOpacity(0.85)),
                  ),
                  const SizedBox(height: 10),

                  // Action buttons (tap first)
                  Row(
                    children: [
                      Expanded(
                        child: FilledButton(
                          onPressed: isConnected
                              ? () {
                                  setState(() {
                                    action = PendingAction.lock;
                                    pin = '';
                                  });
                                }
                              : null,
                          child: const Text('Lock'),
                        ),
                      ),
                      const SizedBox(width: 10),
                      Expanded(
                        child: FilledButton.tonal(
                          onPressed: isConnected
                              ? () {
                                  setState(() {
                                    action = PendingAction.unlock;
                                    pin = '';
                                  });
                                }
                              : null,
                          child: const Text('Unlock'),
                        ),
                      ),
                    ],
                  ),
                  const SizedBox(height: 10),
                  SizedBox(
                    width: double.infinity,
                    child: OutlinedButton(
                      onPressed: isConnected
                          ? () {
                              setState(() {
                                action = PendingAction.setPinEnter;
                                pin = '';
                                newPinFirst = '';
                              });
                            }
                          : null,
                      child: const Text('Set PIN'),
                    ),
                  ),

                  const Divider(height: 24),

                  // PIN display
                  Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: List.generate(4, (i) {
                      final filled = i < pin.length;
                      return Container(
                        width: 16,
                        height: 16,
                        margin: const EdgeInsets.symmetric(horizontal: 10),
                        decoration: BoxDecoration(
                          shape: BoxShape.circle,
                          color: filled
                              ? Colors.white
                              : Colors.white.withOpacity(0.18),
                        ),
                      );
                    }),
                  ),
                  const SizedBox(height: 16),

                  NexgenKeypad(
                    onDigit: (d) {
                      if (action == PendingAction.none) return;
                      if (pin.length >= 4) return;
                      setState(() => pin += d);
                    },
                    onBackspace: () {
                      if (pin.isEmpty) return;
                      setState(() => pin = pin.substring(0, pin.length - 1));
                    },
                    onClear: _clearEntry,
                    onSubmit: () async {
                      await _submitPinIfReady();
                    },
                  ),
                  const SizedBox(height: 10),
                  SizedBox(
                    width: double.infinity,
                    child: TextButton(
                      onPressed: action == PendingAction.none ? null : _resetFlow,
                      child: const Text('Cancel'),
                    ),
                  ),
                ],
              ),
            ),
            const SizedBox(height: 16),

            OutlinedButton(
              onPressed: isConnected
                  ? () async {
                      // quick demo: overwrite LCD.
                      await ble.lcdLine1('Nexgen Safe');
                      await ble.lcdLine2('Use the app');
                    }
                  : null,
              child: const Text('Write to LCD'),
            ),

            const Spacer(),

            Text(
              'Next: proper keypad UI + set PIN flow + device picker.',
              style: TextStyle(color: Colors.white.withOpacity(0.65)),
            ),
          ],
        ),
      ),
    );
  }
}
