# Nexgen Safe Web (Wi-Fi AP mode)

This is a browser UI for controlling the ESP32 safe over HTTP while connected to the safe Wi-Fi hotspot.

## How it works
- ESP32 runs an open Wi-Fi AP (`DEVICE_NAME`)
- ESP32 serves the UI and API from `http://192.168.4.1`
- The browser page calls the same HTTP endpoints directly

## Use
1. Flash `firmware/Nexgen_Safe_BLE_WIFI.ino` to the ESP32.
2. On your phone, join the Wi-Fi network named like `NexgenSafe-07`.
3. Open Safari/Chrome and go to `http://192.168.4.1`.

If you host the page somewhere else, it still works because the firmware enables CORS (`Access-Control-Allow-Origin: *`).

## API
- `GET /status`
- `POST /lock` `{ "pin": "1234" }`
- `POST /unlock` `{ "pin": "1234" }`
- `POST /setpin` `{ "pin1":"1234", "pin2":"1234" }`
- `POST /lcd` `{ "line1":"...", "line2":"..." }`
