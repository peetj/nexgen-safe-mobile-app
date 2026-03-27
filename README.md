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

## Firmware CLI
- Use the wrapper from the repo root:
  - `.\fw.ps1 build`
  - `.\fw.ps1 build ble`
  - `.\fw.ps1 build wifi -PartitionScheme huge_app`
  - `.\fw.ps1 upload wifi -Port COM5`
  - `.\fw.ps1 monitor -Port COM5`
  - `.\fw.ps1 list-ports`
- Bash wrapper for Git Bash / WSL-on-Windows:
  - `bash ./fw.sh build`
  - `bash ./fw.sh build wifi -PartitionScheme huge_app`
  - `bash ./fw.sh build ble -DeviceName NexgenSafe-02`
  - `bash ./fw.sh upload wifi -Port COM5`
  - `bash ./fw.sh list-ports`
- Defaults:
  - target = `wifi`
  - board = `esp32:esp32:esp32`
  - partition scheme = `huge_app` for `wifi`, `default` for `ble`
- Optional device name override at build/upload time:
  - `.\fw.ps1 build wifi -DeviceName NexgenSafe-02`
- Optional partition scheme override at build/upload time:
  - `.\fw.ps1 upload wifi -Port COM5 -PartitionScheme no_ota`
- Build artifacts and temporary sketch folders are written under `build/firmware/`.

## Notes
- Physical keypad + LCD remain first-class.
- Wi-Fi AP mode is the preferred path for school deployment.
- `wifi` is the browser-first firmware target.
- BLE remains available in the separate `ble` firmware target.
- PIN is stored on the ESP32 (EEPROM / NVS).
