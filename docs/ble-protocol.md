# BLE protocol (draft) — Nexgen Safe

Goal: simple BLE control for the ESP32 safe that works on **Android + iPhone**.

## Requirements
- Physical keypad + LCD remain fully functional.
- Mobile app is an *additional* control path.
- PIN stored on ESP32 (EEPROM / NVS).
- App must be able to:
  - Set new 4-digit PIN (enter + confirm)
  - Lock
  - Unlock
  - Read/subscribe to state (locked/unlocked)
  - Write messages to LCD (short text)

## GATT
### Service
- UUID: `6e78676e-7361-6665-0000-000000000001`

### Characteristics
1) COMMAND (Write)
- UUID: `6e78676e-7361-6665-0000-000000000002`
- Write / WriteWithoutResponse

2) STATUS (Notify + Read)
- UUID: `6e78676e-7361-6665-0000-000000000003`
- Notify, Read

3) LCD (Write)
- UUID: `6e78676e-7361-6665-0000-000000000004`
- Write / WriteWithoutResponse

## Payloads (UTF-8 text)
### Commands → COMMAND
- `PING`
- `LOCK:1234`
- `UNLOCK:1234`
- `SETPIN:1234:1234`

### Status ← STATUS
- `STATE:LOCKED`
- `STATE:UNLOCKED`
- `OK`
- `ERR:BAD_PIN`
- `ERR:BAD_FORMAT`
- `ERR:PIN_MISMATCH`

### LCD writes → LCD
- `LINE1:Hello`
- `LINE2:Enter PIN`
- (ESP32 should truncate to 16 chars / pad as needed.)

