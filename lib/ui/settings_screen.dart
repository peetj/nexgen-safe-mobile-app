import 'package:flutter/material.dart';

import '../ble/nexgen_controller.dart';
import '../nexgen_transport.dart';

class SettingsScreen extends StatefulWidget {
  const SettingsScreen({
    super.key,
    required this.transport,
    required this.wifiBaseUrl,
    required this.onConnectionSettingsChanged,
    required this.controller,
  });

  final NexgenTransport transport;
  final String wifiBaseUrl;
  final Future<void> Function(NexgenTransport transport, String wifiBaseUrl)
      onConnectionSettingsChanged;
  final NexgenController controller;

  @override
  State<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen> {
  final line1 = TextEditingController(text: 'Nexgen Safe');
  final line2 = TextEditingController(text: 'Use the app');
  late final TextEditingController wifiBaseUrl;
  late NexgenTransport transport;

  @override
  void initState() {
    super.initState();
    transport = widget.transport;
    wifiBaseUrl = TextEditingController(text: widget.wifiBaseUrl);
  }

  @override
  void dispose() {
    line1.dispose();
    line2.dispose();
    wifiBaseUrl.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final canWrite = transport != NexgenTransport.demo;

    return Scaffold(
      appBar: AppBar(title: const Text('Settings')),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          Card(
            child: Padding(
              padding: const EdgeInsets.all(16),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  const Text(
                    'Connection',
                    style: TextStyle(fontWeight: FontWeight.w700),
                  ),
                  const SizedBox(height: 8),
                  ...NexgenTransport.values.map(
                    (option) => RadioListTile<NexgenTransport>(
                      value: option,
                      groupValue: transport,
                      contentPadding: EdgeInsets.zero,
                      title: Text(option.title),
                      subtitle: Text(option.description),
                      onChanged: (value) {
                        if (value == null) return;
                        setState(() => transport = value);
                      },
                    ),
                  ),
                  if (transport == NexgenTransport.wifiAp) ...[
                    const SizedBox(height: 8),
                    TextField(
                      controller: wifiBaseUrl,
                      decoration: const InputDecoration(
                        labelText: 'Safe API URL',
                        hintText: 'http://192.168.4.1',
                        border: OutlineInputBorder(),
                      ),
                    ),
                  ],
                  const SizedBox(height: 12),
                  FilledButton(
                    onPressed: () async {
                      await widget.onConnectionSettingsChanged(
                        transport,
                        wifiBaseUrl.text.trim(),
                      );
                      if (!context.mounted) return;
                      ScaffoldMessenger.of(context).showSnackBar(
                        const SnackBar(
                          content: Text('Connection settings applied'),
                        ),
                      );
                    },
                    child: const Text('Apply connection settings'),
                  ),
                ],
              ),
            ),
          ),
          const SizedBox(height: 16),
          Card(
            child: Padding(
              padding: const EdgeInsets.all(16),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  const Text('LCD', style: TextStyle(fontWeight: FontWeight.w700)),
                  const SizedBox(height: 8),
                  TextField(
                    controller: line1,
                    decoration: const InputDecoration(
                      labelText: 'Line 1 (16 chars max)',
                      border: OutlineInputBorder(),
                    ),
                  ),
                  const SizedBox(height: 10),
                  TextField(
                    controller: line2,
                    decoration: const InputDecoration(
                      labelText: 'Line 2 (16 chars max)',
                      border: OutlineInputBorder(),
                    ),
                  ),
                  const SizedBox(height: 12),
                  FilledButton(
                    onPressed: canWrite
                        ? () async {
                            await widget.controller.lcdLine1(line1.text);
                            await widget.controller.lcdLine2(line2.text);
                            if (!context.mounted) return;
                            ScaffoldMessenger.of(context).showSnackBar(
                              const SnackBar(content: Text('Sent to LCD')),
                            );
                          }
                        : null,
                    child: Text(
                      canWrite
                          ? 'Write to LCD'
                          : 'LCD write disabled in Demo Mode',
                    ),
                  ),
                ],
              ),
            ),
          ),
          const SizedBox(height: 16),
          Card(
            child: Padding(
              padding: const EdgeInsets.all(16),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  const Text('Branding', style: TextStyle(fontWeight: FontWeight.w700)),
                  const SizedBox(height: 8),
                  Row(
                    children: [
                      Image.asset(
                        'assets/branding/logo_mark.png',
                        width: 32,
                        height: 32,
                      ),
                      const SizedBox(width: 12),
                      const Expanded(
                        child: Text(
                          'Nexgen Safe',
                          style: TextStyle(fontWeight: FontWeight.w600),
                        ),
                      ),
                    ],
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }
}
