import 'dart:async';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import 'nexgen_ble.dart';
import 'nexgen_controller.dart';

/// Demo Mode controller.
///
/// Lets us test the full UI + flows with *no* ESP32 present.
class FakeNexgenController implements NexgenController {
  final _connState = StreamController<NexgenConnState>.broadcast();
  final _lockState = StreamController<SafeLockState>.broadcast();
  final _statusText = StreamController<String>.broadcast();

  NexgenConnState _conn = NexgenConnState.disconnected;
  SafeLockState _lock = SafeLockState.unlocked;
  String _pin = '1234';

  FakeNexgenController() {
    _connState.add(_conn);
    _lockState.add(_lock);
  }

  @override
  Stream<NexgenConnState> get connState => _connState.stream;

  @override
  Stream<SafeLockState> get lockState => _lockState.stream;

  @override
  Stream<String> get statusText => _statusText.stream;

  void _setConn(NexgenConnState s) {
    _conn = s;
    _connState.add(s);
  }

  void _setLock(SafeLockState s) {
    _lock = s;
    _lockState.add(s);
    _statusText.add(s == SafeLockState.locked ? 'STATE:LOCKED' : 'STATE:UNLOCKED');
  }

  @override
  Future<void> startScan({Duration timeout = const Duration(seconds: 6)}) async {
    _setConn(NexgenConnState.scanning);
    await Future<void>.delayed(const Duration(milliseconds: 400));
  }

  @override
  Stream<ScanResult> scanResults() async* {
    // No real scan results in demo mode.
  }

  @override
  Future<void> stopScan() async {
    if (_conn == NexgenConnState.scanning) {
      _setConn(NexgenConnState.disconnected);
    }
  }

  @override
  Future<void> connect([BluetoothDevice? device]) async {
    _setConn(NexgenConnState.connecting);
    await Future<void>.delayed(const Duration(milliseconds: 600));
    _setConn(NexgenConnState.connected);
    await ping();
  }

  @override
  Future<void> disconnect() async {
    _setConn(NexgenConnState.disconnected);
  }

  @override
  Future<void> ping() async {
    _statusText.add(_lock == SafeLockState.locked ? 'STATE:LOCKED' : 'STATE:UNLOCKED');
    _statusText.add('OK');
  }

  bool _is4(String s) => RegExp(r'^\d{4}$').hasMatch(s);

  @override
  Future<void> lock(String pin) async {
    if (!_is4(pin)) {
      _statusText.add('ERR:BAD_FORMAT');
      return;
    }
    if (pin != _pin) {
      _statusText.add('ERR:BAD_PIN');
      return;
    }
    _setLock(SafeLockState.locked);
    _statusText.add('OK');
  }

  @override
  Future<void> unlock(String pin) async {
    if (!_is4(pin)) {
      _statusText.add('ERR:BAD_FORMAT');
      return;
    }
    if (pin != _pin) {
      _statusText.add('ERR:BAD_PIN');
      return;
    }
    _setLock(SafeLockState.unlocked);
    _statusText.add('OK');
  }

  @override
  Future<void> setPin(String newPin, String confirmPin) async {
    if (!_is4(newPin) || !_is4(confirmPin)) {
      _statusText.add('ERR:BAD_FORMAT');
      return;
    }
    if (newPin != confirmPin) {
      _statusText.add('ERR:PIN_MISMATCH');
      return;
    }
    _pin = newPin;
    _statusText.add('OK');
  }

  @override
  Future<void> lcdLine1(String text) async {
    _statusText.add('OK');
  }

  @override
  Future<void> lcdLine2(String text) async {
    _statusText.add('OK');
  }

  @override
  void dispose() {
    _connState.close();
    _lockState.close();
    _statusText.close();
  }
}
