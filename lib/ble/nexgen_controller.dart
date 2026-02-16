import 'dart:async';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import 'nexgen_ble.dart';

/// Shared interface so we can run the app in Demo Mode (no hardware) or Real BLE.
abstract class NexgenController {
  Stream<BleConnState> get connState;
  Stream<SafeLockState> get lockState;
  Stream<String> get statusText;

  BluetoothDevice? get device;

  Future<void> startScan({Duration timeout = const Duration(seconds: 6)});
  Stream<ScanResult> scanResults();
  Future<void> stopScan();
  Future<void> connect(BluetoothDevice device);
  Future<void> disconnect();

  Future<void> ping();
  Future<void> lock(String pin);
  Future<void> unlock(String pin);
  Future<void> setPin(String newPin, String confirmPin);
  Future<void> lcdLine1(String text);
  Future<void> lcdLine2(String text);

  void dispose();
}

/// Wrap existing real BLE implementation.
class RealNexgenController implements NexgenController {
  RealNexgenController(this._inner);

  final NexgenBleController _inner;

  @override
  Stream<BleConnState> get connState => _inner.connState;
  @override
  Stream<SafeLockState> get lockState => _inner.lockState;
  @override
  Stream<String> get statusText => _inner.statusText;

  @override
  BluetoothDevice? get device => _inner.device;

  @override
  Future<void> startScan({Duration timeout = const Duration(seconds: 6)}) =>
      _inner.startScan(timeout: timeout);

  @override
  Stream<ScanResult> scanResults() => _inner.scanResults();

  @override
  Future<void> stopScan() => _inner.stopScan();

  @override
  Future<void> connect(BluetoothDevice device) => _inner.connect(device);

  @override
  Future<void> disconnect() => _inner.disconnect();

  @override
  Future<void> ping() => _inner.ping();

  @override
  Future<void> lock(String pin) => _inner.lock(pin);

  @override
  Future<void> unlock(String pin) => _inner.unlock(pin);

  @override
  Future<void> setPin(String newPin, String confirmPin) =>
      _inner.setPin(newPin, confirmPin);

  @override
  Future<void> lcdLine1(String text) => _inner.lcdLine1(text);

  @override
  Future<void> lcdLine2(String text) => _inner.lcdLine2(text);

  @override
  void dispose() => _inner.dispose();
}
