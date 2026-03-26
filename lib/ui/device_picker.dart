import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import '../ble/nexgen_ble.dart';
import '../ble/nexgen_controller.dart';

class DevicePickerScreen extends StatefulWidget {
  const DevicePickerScreen({super.key, required this.ble});

  final NexgenController ble;

  @override
  State<DevicePickerScreen> createState() => _DevicePickerScreenState();
}

class _DevicePickerScreenState extends State<DevicePickerScreen> {
  final Map<DeviceIdentifier, ScanResult> _results = {};
  StreamSubscription<ScanResult>? _sub;
  bool _scanning = false;
  String _error = '';

  @override
  void initState() {
    super.initState();
    _start();
  }

  @override
  void dispose() {
    _sub?.cancel();
    super.dispose();
  }

  Future<void> _start() async {
    setState(() {
      _scanning = true;
      _error = '';
      _results.clear();
    });

    try {
      await widget.ble.startScan(timeout: const Duration(seconds: 8));

      _sub = widget.ble.scanResults().listen((r) {
        final uuids = r.advertisementData.serviceUuids
            .map((u) => u.str.toLowerCase())
            .toList();
        final isNexgenSafe = uuids.contains(
          NexgenBleUuids.service.str.toLowerCase(),
        );
        if (!isNexgenSafe) return;
        setState(() {
          _results[r.device.remoteId] = r;
        });
      });

      await Future<void>.delayed(const Duration(seconds: 8));
      await widget.ble.stopScan();
    } catch (e) {
      setState(() => _error = e.toString());
    } finally {
      setState(() => _scanning = false);
    }
  }

  Future<void> _connect(ScanResult r) async {
    try {
      await widget.ble.stopScan();
      await widget.ble.connect(r.device);
      if (!mounted) return;
      Navigator.of(context).pop(true);
    } catch (e) {
      if (!mounted) return;
      showDialog(
        context: context,
        builder: (_) => AlertDialog(
          title: const Text('Connection failed'),
          content: Text(e.toString()),
          actions: [
            TextButton(
              onPressed: () => Navigator.pop(context),
              child: const Text('OK'),
            ),
          ],
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    final items = _results.values.toList()
      ..sort((a, b) => b.rssi.compareTo(a.rssi));

    return Scaffold(
      appBar: AppBar(
        title: const Text('Select a safe'),
        actions: [
          IconButton(
            onPressed: _scanning ? null : _start,
            icon: const Icon(Icons.refresh),
            tooltip: 'Rescan',
          ),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(12),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            if (_error.isNotEmpty)
              Padding(
                padding: const EdgeInsets.only(bottom: 12),
                child: Text(
                  _error,
                  style: const TextStyle(color: Colors.redAccent),
                ),
              ),
            if (_scanning)
              const Padding(
                padding: EdgeInsets.only(bottom: 12),
                child: LinearProgressIndicator(),
              ),
            Text(
              'Tip: each safe should be named in firmware (e.g. NexgenSafe-07).',
              style: TextStyle(color: Colors.white.withOpacity(0.7)),
            ),
            const SizedBox(height: 12),
            Expanded(
              child: items.isEmpty
                  ? Center(
                      child: Text(
                        _scanning
                            ? 'Scanning...'
                            : 'No Nexgen Safe devices found',
                        style: TextStyle(color: Colors.white.withOpacity(0.8)),
                      ),
                    )
                  : ListView.separated(
                      itemCount: items.length,
                      separatorBuilder: (_, __) => const Divider(height: 1),
                      itemBuilder: (context, i) {
                        final r = items[i];
                        final name = r.advertisementData.advName.isNotEmpty
                            ? r.advertisementData.advName
                            : (r.device.platformName.isNotEmpty
                                ? r.device.platformName
                                : 'Unnamed');
                        return ListTile(
                          title: Text(name),
                          subtitle: Text(r.device.remoteId.str),
                          trailing: Text('RSSI ${r.rssi}'),
                          onTap: () => _connect(r),
                        );
                      },
                    ),
            ),
          ],
        ),
      ),
    );
  }
}
