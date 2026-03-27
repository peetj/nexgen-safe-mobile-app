# Nexgen Safe Web (Wi-Fi AP mode)

This is a browser UI for controlling the ESP32 safe over HTTP while connected to the safe Wi-Fi hotspot.

## How it works
- ESP32 runs a WPA2 Wi-Fi AP (`DEVICE_NAME`)
- Phone password = `WIFI_PASSWORD` from the sketch
- ESP32 serves the UI and API from `http://192.168.4.1`
- The browser page calls the same HTTP endpoints directly

## Phone Quick Start
1. Power on the safe.
2. On your phone, open Wi-Fi settings and join the network named like `NexgenSafe-07`.
3. Enter the hotspot password shown in the sketch or on the safe LCD during boot.
4. If the phone says the network has no internet, stay connected anyway.
5. Open Safari/Chrome and go to `http://192.168.4.1` if the page does not open automatically.
6. Wait for the page to show the safe state as `LOCKED` or `UNLOCKED`.

## Using The Page
1. Tap `Unlock`, enter the current 4-digit PIN, then tap `Go`.
2. Tap `Lock`, enter the current 4-digit PIN, then tap `Go`.
3. Tap `Set PIN` to change the safe PIN, then enter the new 4-digit PIN twice.

## Setup
1. Flash `firmware/Nexgen_Safe_BLE_WIFI.ino` to the ESP32.
2. Give each safe a different `DEVICE_NAME` if several safes will be used in the same room.
3. Set a WPA2 hotspot password in `WIFI_PASSWORD` before flashing.

If you host the page somewhere else, it still works because the firmware enables CORS (`Access-Control-Allow-Origin: *`).

## Troubleshooting
- If the page shows `OFFLINE`, make sure the phone is still connected to the safe Wi-Fi.
- If the phone switches back to mobile data or another Wi-Fi network, reconnect to the safe and reopen `http://192.168.4.1`.
- If multiple safes are nearby, check that you joined the correct Wi-Fi name before entering a PIN.

## API
- `GET /status`
- `POST /lock` `{ "pin": "1234" }`
- `POST /unlock` `{ "pin": "1234" }`
- `POST /setpin` `{ "pin1":"1234", "pin2":"1234" }`
