import 'package:flutter/material.dart';

import '../ble/nexgen_controller.dart';

class SettingsScreen extends StatefulWidget {
  const SettingsScreen({
    super.key,
    required this.demoMode,
    required this.onDemoModeChanged,
    required this.ble,
  });

  final bool demoMode;
  final ValueChanged<bool> onDemoModeChanged;
  final NexgenController ble;

  @override
  State<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen> {
  final line1 = TextEditingController(text: 'Nexgen Safe');
  final line2 = TextEditingController(text: 'Use the app');

  @override
  void dispose() {
    line1.dispose();
    line2.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final canWrite = !widget.demoMode;

    return Scaffold(
      appBar: AppBar(title: const Text('Settings')),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          Card(
            child: Padding(
              padding: const EdgeInsets.all(16),
              child: Row(
                children: [
                  const Expanded(
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text('Demo Mode', style: TextStyle(fontWeight: FontWeight.w700)),
                        SizedBox(height: 4),
                        Text('Test the app with no hardware', style: TextStyle(fontSize: 12)),
                      ],
                    ),
                  ),
                  Switch(
                    value: widget.demoMode,
                    onChanged: widget.onDemoModeChanged,
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
                            await widget.ble.lcdLine1(line1.text);
                            await widget.ble.lcdLine2(line2.text);
                            if (!context.mounted) return;
                            ScaffoldMessenger.of(context).showSnackBar(
                              const SnackBar(content: Text('Sent to LCD')),
                            );
                          }
                        : null,
                    child: Text(canWrite ? 'Write to LCD' : 'LCD write disabled in Demo Mode'),
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
                      Image.asset('assets/branding/logo_mark.png', width: 32, height: 32),
                      const SizedBox(width: 12),
                      const Expanded(
                        child: Text('Nexgen Safe', style: TextStyle(fontWeight: FontWeight.w600)),
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
