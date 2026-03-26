import 'dart:async';
import 'dart:convert';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:http/http.dart' as http;

import '../ble/nexgen_ble.dart';
import '../ble/nexgen_controller.dart';

class NexgenHttpController implements NexgenController {
  NexgenHttpController({
    required String baseUrl,
    http.Client? client,
  })  : _client = client ?? http.Client(),
        _baseUri = _normalizeBaseUri(baseUrl);

  final http.Client _client;
  final Uri _baseUri;

  final _connState = StreamController<NexgenConnState>.broadcast();
  final _lockState = StreamController<SafeLockState>.broadcast();
  final _statusText = StreamController<String>.broadcast();

  Timer? _poller;
  bool _disposed = false;

  @override
  Stream<NexgenConnState> get connState => _connState.stream;

  @override
  Stream<SafeLockState> get lockState => _lockState.stream;

  @override
  Stream<String> get statusText => _statusText.stream;

  @override
  Future<void> startScan({Duration timeout = const Duration(seconds: 6)}) async {
    _emitConn(NexgenConnState.disconnected);
  }

  @override
  Stream<ScanResult> scanResults() => const Stream<ScanResult>.empty();

  @override
  Future<void> stopScan() async {}

  @override
  Future<void> connect([BluetoothDevice? device]) async {
    _emitConn(NexgenConnState.connecting);

    try {
      final status = await _getJson('status');
      _applyStatus(status);
      _emitConn(NexgenConnState.connected);
      _statusText.add(
        'Connected to ${status['name'] ?? 'Nexgen Safe'} over Wi-Fi',
      );
      _startPolling();
    } catch (e) {
      _poller?.cancel();
      _emitConn(NexgenConnState.disconnected);
      _emitLock(SafeLockState.unknown);
      rethrow;
    }
  }

  @override
  Future<void> disconnect() async {
    _poller?.cancel();
    _poller = null;
    _emitConn(NexgenConnState.disconnected);
    _emitLock(SafeLockState.unknown);
    if (!_disposed) {
      _statusText.add('Disconnected from Wi-Fi safe');
    }
  }

  @override
  Future<void> ping() async {
    final status = await _getJson('status');
    _applyStatus(status);
  }

  @override
  Future<void> lock(String pin) => _postAction('lock', {'pin': pin});

  @override
  Future<void> unlock(String pin) => _postAction('unlock', {'pin': pin});

  @override
  Future<void> setPin(String newPin, String confirmPin) =>
      _postAction('setpin', {'pin1': newPin, 'pin2': confirmPin});

  @override
  Future<void> lcdLine1(String text) =>
      _postAction('lcd', {'line1': text}, refreshStatus: false);

  @override
  Future<void> lcdLine2(String text) =>
      _postAction('lcd', {'line2': text}, refreshStatus: false);

  Future<void> _postAction(
    String path,
    Map<String, String> body, {
    bool refreshStatus = true,
  }) async {
    final response = await _client.post(
      _endpoint(path),
      headers: const {'Content-Type': 'application/json'},
      body: jsonEncode(body),
    );

    final payload = _decodeJson(response);
    if (response.statusCode < 200 || response.statusCode >= 300) {
      throw Exception(_formatError(payload, fallback: 'HTTP_${response.statusCode}'));
    }

    if (payload case {'ok': false}) {
      throw Exception(_formatError(payload));
    }

    _statusText.add('OK');
    if (refreshStatus) {
      await ping();
    }
  }

  Future<Map<String, dynamic>> _getJson(String path) async {
    final response = await _client.get(_endpoint(path));
    final payload = _decodeJson(response);
    if (response.statusCode < 200 || response.statusCode >= 300) {
      throw Exception(_formatError(payload, fallback: 'HTTP_${response.statusCode}'));
    }
    return payload;
  }

  void _applyStatus(Map<String, dynamic> status) {
    final locked = status['locked'];
    if (locked is bool) {
      _emitLock(locked ? SafeLockState.locked : SafeLockState.unlocked);
    } else {
      _emitLock(SafeLockState.unknown);
    }
  }

  void _startPolling() {
    _poller?.cancel();
    _poller = Timer.periodic(const Duration(seconds: 2), (_) async {
      try {
        await ping();
      } catch (_) {
        await disconnect();
        if (!_disposed) {
          _statusText.add(
            'Wi-Fi link lost. Join the ESP32 hotspot again, then reconnect.',
          );
        }
      }
    });
  }

  Uri _endpoint(String path) => _baseUri.resolve(path);

  void _emitConn(NexgenConnState state) {
    if (!_disposed) {
      _connState.add(state);
    }
  }

  void _emitLock(SafeLockState state) {
    if (!_disposed) {
      _lockState.add(state);
    }
  }

  @override
  void dispose() {
    _disposed = true;
    _poller?.cancel();
    _client.close();
    _connState.close();
    _lockState.close();
    _statusText.close();
  }

  static Uri _normalizeBaseUri(String rawBaseUrl) {
    final trimmed = rawBaseUrl.trim();
    final withScheme = trimmed.startsWith('http')
        ? trimmed
        : 'http://$trimmed';
    final normalized = withScheme.endsWith('/')
        ? withScheme
        : '$withScheme/';
    return Uri.parse(normalized);
  }

  static Map<String, dynamic> _decodeJson(http.Response response) {
    if (response.body.isEmpty) {
      return const {};
    }

    final decoded = jsonDecode(response.body);
    if (decoded is Map<String, dynamic>) {
      return decoded;
    }

    throw const FormatException('Expected a JSON object response');
  }

  static String _formatError(
    Map<String, dynamic> payload, {
    String fallback = 'REQUEST_FAILED',
  }) {
    final code = payload['err']?.toString() ?? fallback;
    return 'Safe request failed: $code';
  }
}
