import 'package:flutter/material.dart';

/// Nexgen branding (placeholder).
/// If you have exact hex codes, we’ll lock them in here.
class NexgenBrand {
  // Brand colours
  static const Color orange = Color(0xFFFF5E19); // main
  static const Color charcoal = Color(0xFF15181E);
  static const Color offWhite = Color(0xFFF2F2F0);
  static const Color powerPink = Color(0xFFFF2D95);

  static ThemeData theme() {
    final scheme = ColorScheme.fromSeed(
      seedColor: orange,
      brightness: Brightness.dark,
    );

    return ThemeData(
      useMaterial3: true,
      colorScheme: scheme.copyWith(
        primary: orange,
        secondary: powerPink,
        surface: charcoal,
        onSurface: offWhite,
      ),
      scaffoldBackgroundColor: charcoal,
      appBarTheme: const AppBarTheme(
        backgroundColor: charcoal,
        foregroundColor: offWhite,
      ),
      dividerColor: offWhite.withOpacity(0.12),
    );
  }
}
