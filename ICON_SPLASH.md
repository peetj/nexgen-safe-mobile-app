# Icons + Splash

This repo includes Nexgen branding assets in `assets/branding/`.

## App icon (launcher)
We use `assets/branding/nexgen-new-logo-round.png` as the base.

After running `flutter pub get`, generate icons:

```bash
flutter pub run flutter_launcher_icons
```

## Splash screen
We use a dark background + the Nexgen mark.

Generate splash:

```bash
flutter pub run flutter_native_splash:create
```

