# Nexgen Safe Mobile App

**Product name:** Nexgen Safe  
**Repo:** `nexgen-safe-mobile-app`

Cross-platform (Android + iOS) app that connects to the Nexgen Safe **ESP32** over **Bluetooth Low Energy (BLE)**.

## MVP
- Connect to the safe (scan + connect)
- Set PIN (4 digits, confirm)
- Lock / unlock (servo rotates 90°)
- Show state clearly (connected + locked/unlocked)
- Optional: write short messages to the safe’s LCD

## Tech
- **Flutter** (best cross-platform UI/UX for free)

## Branding
Brand assets are stored in:
- `assets/branding/`

## Docs
- `docs/ble-protocol.md` (draft GATT + payload spec)

## Notes
- Physical keypad + LCD remain first-class (not a backup).
- BLE is an additional control path.
- PIN is stored on the ESP32 (EEPROM / NVS).

