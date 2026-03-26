import 'package:flutter/material.dart';

import 'ble/fake_nexgen_controller.dart';
import 'ble/nexgen_ble.dart';
import 'ble/nexgen_controller.dart';
import 'network/nexgen_http.dart';
import 'nexgen_brand.dart';
import 'nexgen_transport.dart';
import 'ui/device_picker.dart';
import 'ui/keypad.dart';
import 'ui/settings_screen.dart';

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
  NexgenTransport transport = NexgenTransport.wifiAp;
  String wifiBaseUrl = 'http://192.168.4.1';

  late NexgenController controller;

  SafeLockState lockState = SafeLockState.unknown;
  NexgenConnState connState = NexgenConnState.disconnected;
  String status = '';

  PendingAction action = PendingAction.none;
  String pin = '';
  String newPinFirst = '';

  void _initController() {
    controller = switch (transport) {
      NexgenTransport.demo => FakeNexgenController(),
      NexgenTransport.ble => RealNexgenController(NexgenBleController()),
      NexgenTransport.wifiAp => NexgenHttpController(baseUrl: wifiBaseUrl),
    };

    controller.connState.listen((s) => setState(() => connState = s));
    controller.lockState.listen((s) => setState(() => lockState = s));
    controller.statusText.listen((t) => setState(() => status = t));
  }

  @override
  void initState() {
    super.initState();
    _initController();
  }

  @override
  void dispose() {
    controller.dispose();
    super.dispose();
  }

  Future<void> _connectFlow() async {
    if (transport == NexgenTransport.demo) {
      await controller.connect();
      return;
    }

    if (transport == NexgenTransport.ble) {
      await Navigator.of(context).push(
        MaterialPageRoute(
          builder: (_) => DevicePickerScreen(ble: controller),
        ),
      );
      return;
    }

    try {
      await controller.connect();
    } catch (e) {
      if (!mounted) return;
      await showDialog<void>(
        context: context,
        builder: (_) => AlertDialog(
          title: const Text('Wi-Fi connection failed'),
          content: Text(
            'Join the ESP32 hotspot first, then try Connect again.\n\n'
            'Expected API URL: $wifiBaseUrl\n\n'
            'Details: $e',
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.of(context).pop(),
              child: const Text('OK'),
            ),
          ],
        ),
      );
    }
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
    if (connState != NexgenConnState.connected) {
      return transport == NexgenTransport.wifiAp
          ? 'Join the safe Wi-Fi, then tap Connect'
          : 'Connect to a safe';
    }

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
          await controller.lock(pin);
          _resetFlow();
          return;
        case PendingAction.unlock:
          await controller.unlock(pin);
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
          await controller.setPin(newPinFirst, confirm);
          _resetFlow();
          return;
        case PendingAction.none:
          return;
      }
    } catch (e) {
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
    final isConnected = connState == NexgenConnState.connected;

    return Scaffold(
      appBar: AppBar(
        title: Row(
          children: [
            Image.asset('assets/branding/logo_mark.png', width: 24, height: 24),
            const SizedBox(width: 10),
            const Text('Nexgen Safe'),
          ],
        ),
        actions: [
          IconButton(
            tooltip: 'Settings',
            icon: const Icon(Icons.settings),
            onPressed: () async {
              await Navigator.of(context).push(
                MaterialPageRoute(
                  builder: (_) => SettingsScreen(
                    transport: transport,
                    wifiBaseUrl: wifiBaseUrl,
                    controller: controller,
                    onConnectionSettingsChanged:
                        (nextTransport, nextWifiBaseUrl) async {
                      await controller.disconnect();
                      controller.dispose();
                      if (!mounted) return;
                      setState(() {
                        transport = nextTransport;
                        wifiBaseUrl = nextWifiBaseUrl.isEmpty
                            ? 'http://192.168.4.1'
                            : nextWifiBaseUrl;
                        connState = NexgenConnState.disconnected;
                        lockState = SafeLockState.unknown;
                        status = '';
                        _initController();
                      });
                    },
                  ),
                ),
              );
            },
          ),
          TextButton(
            onPressed: isConnected ? () => controller.disconnect() : _connectFlow,
            child: Text(
              isConnected ? 'Disconnect' : 'Connect',
              style: const TextStyle(color: Colors.white),
            ),
          ),
        ],
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Text(
              'Transport: ${transport.title}',
              style: TextStyle(color: Colors.white.withOpacity(0.7)),
            ),
            if (transport == NexgenTransport.wifiAp)
              Padding(
                padding: const EdgeInsets.only(top: 4),
                child: Text(
                  wifiBaseUrl,
                  style: TextStyle(color: Colors.white.withOpacity(0.55)),
                ),
              ),
            const SizedBox(height: 12),
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
                      await controller.lcdLine1('Nexgen Safe');
                      await controller.lcdLine2('Use the app');
                    }
                  : null,
              child: const Text('Write to LCD'),
            ),
            const SizedBox(height: 24),
            Text(
              transport == NexgenTransport.wifiAp
                  ? 'Phone must join the ESP32 hotspot. No school Wi-Fi required.'
                  : 'BLE remains available if you want direct Bluetooth control.',
              style: TextStyle(color: Colors.white.withOpacity(0.65)),
            ),
          ],
        ),
      ),
    );
  }
}
