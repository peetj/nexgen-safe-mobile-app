import 'package:flutter/material.dart';

/// Nexgen branding (placeholder).
/// If you have exact hex codes, we’ll lock them in here.
class NexgenBrand {
  static const Color primary = Color(0xFF111827); // near-black
  static const Color accent = Color(0xFF22C55E); // green
  static const Color background = Color(0xFF0B1220);

  static ThemeData theme() {
    final scheme = ColorScheme.fromSeed(
      seedColor: accent,
      brightness: Brightness.dark,
    );

    return ThemeData(
      useMaterial3: true,
      colorScheme: scheme.copyWith(
        primary: primary,
        secondary: accent,
        surface: background,
      ),
      scaffoldBackgroundColor: background,
    );
  }
}
