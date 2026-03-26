# Nexgen Safe Mobile App

**Product name:** Nexgen Safe  
**Repo:** `nexgen-safe-mobile-app`

Cross-platform (Android + iOS) app that connects to the Nexgen Safe ESP32 from a phone.

Recommended transport for school use:
- ESP32-hosted Wi-Fi AP at `http://192.168.4.1`

Also supported:
- Bluetooth Low Energy (BLE)

## MVP
- Connect to the safe
- Connect over the ESP32 hotspot with no school network dependency
- Set PIN (4 digits, confirm)
- Lock / unlock (servo rotates 90 degrees)
- Show state clearly (connected + locked/unlocked)
- Optional: write short messages to the safe LCD

## Tech
- Flutter

## Branding
Brand assets are stored in:
- `assets/branding/`

## Docs
- `docs/ble-protocol.md` (draft GATT + payload spec)
- `docs/README.md` (ESP32 Wi-Fi AP mode)

## Notes
- Physical keypad + LCD remain first-class.
- Wi-Fi AP mode is the preferred path for school deployment.
- BLE remains available as an additional control path.
- PIN is stored on the ESP32 (EEPROM / NVS).
