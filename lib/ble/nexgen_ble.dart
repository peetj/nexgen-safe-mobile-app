import 'dart:async';
import 'dart:convert';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';

class NexgenBleUuids {
  static final Guid service = Guid('6e78676e-7361-6665-0000-000000000001');
  static final Guid command = Guid('6e78676e-7361-6665-0000-000000000002');
  static final Guid status = Guid('6e78676e-7361-6665-0000-000000000003');
  static final Guid lcd = Guid('6e78676e-7361-6665-0000-000000000004');
}

enum SafeLockState { locked, unlocked, unknown }

enum BleConnState { disconnected, scanning, connecting, connected }

class NexgenBleController {
  NexgenBleController();

  BluetoothDevice? _device;
  BluetoothCharacteristic? _cmd;
  BluetoothCharacteristic? _status;
  BluetoothCharacteristic? _lcd;

  final _connState = StreamController<BleConnState>.broadcast();
  final _lockState = StreamController<SafeLockState>.broadcast();
  final _statusText = StreamController<String>.broadcast();

  Stream<BleConnState> get connState => _connState.stream;
  Stream<SafeLockState> get lockState => _lockState.stream;
  Stream<String> get statusText => _statusText.stream;

  BluetoothDevice? get device => _device;

  Future<void> startScan({Duration timeout = const Duration(seconds: 6)}) async {
    _connState.add(BleConnState.scanning);
    await FlutterBluePlus.startScan(timeout: timeout);
  }

  Stream<ScanResult> scanResults() => FlutterBluePlus.scanResults.expand((x) => x);

  Future<void> stopScan() async {
    await FlutterBluePlus.stopScan();
  }

  Future<void> connect(BluetoothDevice device) async {
    _connState.add(BleConnState.connecting);
    _device = device;

    await device.connect(timeout: const Duration(seconds: 12));

    final services = await device.discoverServices();
    final svc = services.where((s) => s.uuid == NexgenBleUuids.service).toList();
    if (svc.isEmpty) {
      throw Exception('Nexgen Safe BLE service not found');
    }

    for (final s in svc) {
      for (final c in s.characteristics) {
        if (c.uuid == NexgenBleUuids.command) _cmd = c;
        if (c.uuid == NexgenBleUuids.status) _status = c;
        if (c.uuid == NexgenBleUuids.lcd) _lcd = c;
      }
    }

    if (_cmd == null || _status == null) {
      throw Exception('Missing required characteristics');
    }

    await _status!.setNotifyValue(true);
    _status!.onValueReceived.listen((data) {
      final text = utf8.decode(data, allowMalformed: true).trim();
      _statusText.add(text);
      if (text.startsWith('STATE:LOCKED')) _lockState.add(SafeLockState.locked);
      if (text.startsWith('STATE:UNLOCKED')) _lockState.add(SafeLockState.unlocked);
    });

    _connState.add(BleConnState.connected);
    await ping();
  }

  Future<void> disconnect() async {
    final d = _device;
    _device = null;
    _cmd = null;
    _status = null;
    _lcd = null;
    if (d != null) {
      await d.disconnect();
    }
    _connState.add(BleConnState.disconnected);
    _lockState.add(SafeLockState.unknown);
  }

  Future<void> _writeCmd(String text) async {
    if (_cmd == null) throw Exception('Not connected');
    await _cmd!.write(utf8.encode(text), withoutResponse: false);
  }

  Future<void> ping() => _writeCmd('PING');

  Future<void> lock(String pin) => _writeCmd('LOCK:$pin');
  Future<void> unlock(String pin) => _writeCmd('UNLOCK:$pin');
  Future<void> setPin(String newPin, String confirmPin) => _writeCmd('SETPIN:$newPin:$confirmPin');

  Future<void> lcdLine1(String text) async {
    if (_lcd == null) throw Exception('LCD characteristic not available');
    await _lcd!.write(utf8.encode('LINE1:$text'), withoutResponse: true);
  }

  Future<void> lcdLine2(String text) async {
    if (_lcd == null) throw Exception('LCD characteristic not available');
    await _lcd!.write(utf8.encode('LINE2:$text'), withoutResponse: true);
  }

  void dispose() {
    _connState.close();
    _lockState.close();
    _statusText.close();
  }
}
