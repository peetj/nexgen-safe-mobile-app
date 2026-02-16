import 'package:flutter/material.dart';

import 'ble/fake_nexgen_controller.dart';
import 'ble/nexgen_ble.dart';
import 'ble/nexgen_controller.dart';
import 'nexgen_brand.dart';

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

class _HomeScreenState extends State<HomeScreen> {
  // Demo mode: lets you test UI without hardware.
  bool demoMode = true;

  late NexgenController ble;

  SafeLockState lockState = SafeLockState.unknown;
  BleConnState connState = BleConnState.disconnected;
  String status = '';

  final pinController = TextEditingController();

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
    pinController.dispose();
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

            TextField(
              controller: pinController,
              keyboardType: TextInputType.number,
              maxLength: 4,
              decoration: const InputDecoration(
                labelText: 'PIN (4 digits)',
                border: OutlineInputBorder(),
              ),
            ),
            const SizedBox(height: 12),

            Row(
              children: [
                Expanded(
                  child: FilledButton(
                    onPressed: isConnected
                        ? () => ble.lock(pinController.text)
                        : null,
                    child: const Text('Lock'),
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: FilledButton.tonal(
                    onPressed: isConnected
                        ? () => ble.unlock(pinController.text)
                        : null,
                    child: const Text('Unlock'),
                  ),
                ),
              ],
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
