# Nexgen Safe Web (Wi-Fi AP mode)

This is a simple web UI that controls the ESP32 safe over HTTP while connected to the safe’s Wi‑Fi hotspot.

## How it works
- ESP32 runs an open Wi‑Fi AP (SSID = DEVICE_NAME)
- ESP32 serves an API at `http://192.168.4.1`
- This page calls the API endpoints.

## Use
1. Flash `firmware/Nexgen_Safe_BLE_WIFI.ino` to the ESP32.
2. On your phone, join the Wi‑Fi network named like `NexgenSafe-07`.
3. Open Safari/Chrome and visit this page (host it anywhere, or just open it locally).

If you host the page somewhere (GitHub Pages), it will still work because the firmware enables CORS (`Access-Control-Allow-Origin: *`).

## API
- `GET /status`
- `POST /lock` `{ "pin": "1234" }`
- `POST /unlock` `{ "pin": "1234" }`
- `POST /setpin` `{ "pin1":"1234", "pin2":"1234" }`
- `POST /lcd` `{ "line1":"...", "line2":"..." }`
