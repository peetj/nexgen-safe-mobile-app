/*
  Nexgen Safe (ESP32 + Keypad + Servo + I2C LCD) + BLE control

  - Keeps keypad/LCD workflow intact
  - Adds BLE GATT service so the mobile app can:
      - LOCK/UNLOCK with a 4-digit PIN
      - SETPIN with confirm
      - Read/notify state
      - Write to LCD lines

  Student edit point:
    - DEVICE_NAME below (so multiple safes in a room are easy to identify)

  BLE library:
    - Recommended: NimBLE-Arduino
      https://github.com/h2zero/NimBLE-Arduino

  UUIDs: see docs/ble-protocol.md in the mobile app repo.
*/

#include <Wire.h>
#include <Keypad.h>
#include <ESP32Servo.h>

#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

#include <NimBLEDevice.h>
#include <EEPROM.h>

#include "SafeState.h"
#include "icons.h"

// -----------------------------
// Config
// -----------------------------
static const char* DEVICE_NAME = "NexgenSafe-01"; // <-- students change this

static const uint8_t LCD_COLS = 16;
static const uint8_t LCD_ROWS = 2;

static const uint8_t SERVO_PIN        = 19;
static const uint8_t SERVO_LOCK_POS   = 0;
static const uint8_t SERVO_UNLOCK_POS = 90;

static const uint8_t CODE_LEN = 4;

// Keypad pins (R1..R4, C1..C3)
static const uint8_t PIN_R1 = 14;
static const uint8_t PIN_R2 = 32;
static const uint8_t PIN_R3 = 33;
static const uint8_t PIN_R4 = 26;

static const uint8_t PIN_C1 = 27;
static const uint8_t PIN_C2 = 12;
static const uint8_t PIN_C3 = 25;

// -----------------------------
// BLE UUIDs
// -----------------------------
static NimBLEUUID UUID_SVC("6e78676e-7361-6665-0000-000000000001");
static NimBLEUUID UUID_CMD("6e78676e-7361-6665-0000-000000000002");
static NimBLEUUID UUID_STATUS("6e78676e-7361-6665-0000-000000000003");
static NimBLEUUID UUID_LCD("6e78676e-7361-6665-0000-000000000004");

static NimBLECharacteristic* chStatus = nullptr;

// -----------------------------
// Hardware objects
// -----------------------------
Servo lockServo;
hd44780_I2Cexp lcd;

static const byte ROWS = 4;
static const byte COLS = 3;

char keymap[ROWS][COLS] = {
  { '1','2','3' },
  { '4','5','6' },
  { '7','8','9' },
  { '*','0','#' }
};

byte rowPins[ROWS] = { PIN_R1, PIN_R2, PIN_R3, PIN_R4 };
byte colPins[COLS] = { PIN_C1, PIN_C2, PIN_C3 };

Keypad keypad = Keypad(makeKeymap(keymap), rowPins, colPins, ROWS, COLS);

SafeState safeState;

// -----------------------------
// Helpers
// -----------------------------
static inline void lcdCenterPrint(uint8_t row, const char* text) {
  size_t len = strlen(text);
  uint8_t col = (len >= LCD_COLS) ? 0 : (LCD_COLS - len) / 2;
  lcd.setCursor(col, row);
  lcd.print(text);
}

static void notifyState() {
  if (!chStatus) return;
  if (safeState.locked()) {
    chStatus->setValue("STATE:LOCKED");
  } else {
    chStatus->setValue("STATE:UNLOCKED");
  }
  chStatus->notify();
}

static void servoLock() {
  lockServo.write(SERVO_LOCK_POS);
  safeState.lock();
  notifyState();
}

static void servoUnlock() {
  lockServo.write(SERVO_UNLOCK_POS);
  // safeState.unlock(pin) will flip state; we notify there.
}

static void lcdSetLine(uint8_t line, const String& text) {
  String t = text;
  if (t.length() > LCD_COLS) t = t.substring(0, LCD_COLS);
  lcd.setCursor(0, line);
  lcd.print("                ");
  lcd.setCursor(0, line);
  lcd.print(t);
}

// Simple string split helpers
static bool is4Digits(const String& s) {
  if (s.length() != 4) return false;
  for (uint8_t i = 0; i < 4; i++) {
    if (s[i] < '0' || s[i] > '9') return false;
  }
  return true;
}

static void statusOk() {
  if (!chStatus) return;
  chStatus->setValue("OK");
  chStatus->notify();
}

static void statusErr(const char* code) {
  if (!chStatus) return;
  String msg = String("ERR:") + code;
  chStatus->setValue(msg);
  chStatus->notify();
}

// Verify PIN against EEPROM *without* changing lock state.
// Mirrors SafeState::unlock logic but does not call setLock(false).
static bool verifyPinNoSideEffects(const String& code) {
  const uint8_t EEPROM_ADDR_CODE_LEN = 1;
  const uint8_t EEPROM_ADDR_CODE = 2;
  const uint8_t EEPROM_EMPTY = 0xff;

  uint8_t codeLength = EEPROM.read(EEPROM_ADDR_CODE_LEN);
  if (codeLength == EEPROM_EMPTY) {
    // No code set => accept any PIN
    return true;
  }

  if (code.length() != codeLength) return false;

  for (uint8_t i = 0; i < codeLength; i++) {
    uint8_t stored = EEPROM.read(EEPROM_ADDR_CODE + i);
    if (stored != (uint8_t)code[i]) return false;
  }
  return true;
}

// -----------------------------
// BLE callbacks
// -----------------------------
class CmdCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c, NimBLEConnInfo& connInfo) override {
    std::string v = c->getValue();
    String s(v.c_str());
    s.trim();

    if (s == "PING") {
      notifyState();
      statusOk();
      return;
    }

    // LOCK:1234
    if (s.startsWith("LOCK:")) {
      String pin = s.substring(5);
      pin.trim();
      if (!is4Digits(pin)) {
        statusErr("BAD_FORMAT");
        return;
      }

      // For lock we require the correct PIN if a PIN exists.
      // If no code is set, allow lock after setting one at keypad or via SETPIN.
      if (safeState.hasCode() && !verifyPinNoSideEffects(pin)) {
        statusErr("BAD_PIN");
        return;
      }

      servoLock();
      statusOk();
      return;
    }

    // UNLOCK:1234
    if (s.startsWith("UNLOCK:")) {
      String pin = s.substring(7);
      pin.trim();
      if (!is4Digits(pin)) {
        statusErr("BAD_FORMAT");
        return;
      }

      bool ok = safeState.unlock(pin);
      if (ok) {
        servoUnlock();
        notifyState();
        statusOk();
      } else {
        statusErr("BAD_PIN");
      }
      return;
    }

    // SETPIN:1234:1234
    if (s.startsWith("SETPIN:")) {
      String rest = s.substring(7);
      int idx = rest.indexOf(':');
      if (idx < 0) {
        statusErr("BAD_FORMAT");
        return;
      }
      String a = rest.substring(0, idx);
      String b = rest.substring(idx + 1);
      a.trim();
      b.trim();
      if (!is4Digits(a) || !is4Digits(b)) {
        statusErr("BAD_FORMAT");
        return;
      }
      if (a != b) {
        statusErr("PIN_MISMATCH");
        return;
      }
      safeState.setCode(a);
      statusOk();
      return;
    }

    statusErr("BAD_FORMAT");
  }
};

class LcdCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c, NimBLEConnInfo& connInfo) override {
    std::string v = c->getValue();
    String s(v.c_str());
    s.trim();

    if (s.startsWith("LINE1:")) {
      lcdSetLine(0, s.substring(6));
      statusOk();
      return;
    }

    if (s.startsWith("LINE2:")) {
      lcdSetLine(1, s.substring(6));
      statusOk();
      return;
    }

    statusErr("BAD_FORMAT");
  }
};

static void setupBle() {
  NimBLEDevice::init(DEVICE_NAME);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  NimBLEServer* server = NimBLEDevice::createServer();
  NimBLEService* svc = server->createService(UUID_SVC);

  NimBLECharacteristic* chCmd = svc->createCharacteristic(UUID_CMD, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  chStatus = svc->createCharacteristic(UUID_STATUS, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  NimBLECharacteristic* chLcd = svc->createCharacteristic(UUID_LCD, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);

  chCmd->setCallbacks(new CmdCallbacks());
  chLcd->setCallbacks(new LcdCallbacks());

  chStatus->setValue("STATE:UNKNOWN");

  svc->start();

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(UUID_SVC);
  adv->setScanResponse(true);
  adv->start();
}

// -----------------------------
// Existing UI logic (keypad)
// -----------------------------
static String readNumericCodeMasked(uint8_t length) {
  lcd.setCursor(5, 1);
  lcd.print("[____]");
  lcd.setCursor(6, 1);

  String code;
  code.reserve(length);

  while (code.length() < length) {
    char k = keypad.getKey();
    if (k >= '0' && k <= '9') {
      lcd.print('*');
      code += k;
    }
  }
  return code;
}

static bool promptSetNewCode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter new code:");
  String newCode = readNumericCodeMasked(CODE_LEN);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Confirm new code");
  String confirm = readNumericCodeMasked(CODE_LEN);

  if (newCode == confirm) {
    safeState.setCode(newCode);
    statusOk();
    return true;
  }

  lcd.clear();
  lcdCenterPrint(0, "Code mismatch");
  lcd.setCursor(0, 1);
  lcd.print("Safe not locked!");
  delay(2000);
  statusErr("PIN_MISMATCH");
  return false;
}

static char waitForKey(char a, char b) {
  char k = keypad.getKey();
  while (k != a && k != b) {
    k = keypad.getKey();
  }
  return k;
}

static void showProgressBar(uint16_t stepDelayMs) {
  lcd.setCursor(2, 1);
  lcd.print("[..........]");
  lcd.setCursor(3, 1);
  for (uint8_t i = 0; i < 10; i++) {
    delay(stepDelayMs);
    lcd.print('=');
  }
}

static void showUnlockedBanner() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(ICON_UNLOCKED_CHAR);
  lcdCenterPrint(0, "Unlocked!");
  lcd.setCursor(15, 0);
  lcd.write(ICON_UNLOCKED_CHAR);
  delay(900);
}

static void showLockedBanner() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(ICON_LOCKED_CHAR);
  lcd.print(" Safe Locked! ");
  lcd.write(ICON_LOCKED_CHAR);
}

static void runUnlockedState() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(ICON_UNLOCKED_CHAR);
  lcd.setCursor(2, 0);
  lcd.print("# to lock");
  lcd.setCursor(15, 0);
  lcd.write(ICON_UNLOCKED_CHAR);

  bool mustSetCode = !safeState.hasCode();
  if (!mustSetCode) {
    lcd.setCursor(0, 1);
    lcd.print("* = new code");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Set code to lock");
  }

  char k = waitForKey('*', '#');

  bool readyToLock = true;
  if (k == '*' || mustSetCode) {
    readyToLock = promptSetNewCode();
  }

  if (!readyToLock) return;

  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.write(ICON_UNLOCKED_CHAR);
  lcd.print(" ");
  lcd.write(ICON_RIGHT_ARROW);
  lcd.print(" ");
  lcd.write(ICON_LOCKED_CHAR);

  servoLock();
  showProgressBar(100);
}

static void runLockedState() {
  showLockedBanner();

  String userCode = readNumericCodeMasked(CODE_LEN);

  bool ok = safeState.unlock(userCode);
  showProgressBar(200);

  if (ok) {
    showUnlockedBanner();
    servoUnlock();
    notifyState();
  } else {
    lcd.clear();
    lcdCenterPrint(0, "Access Denied!");
    showProgressBar(400);
  }
}

static void initLcdOrHalt() {
  int status = lcd.begin(LCD_COLS, LCD_ROWS);
  if (status) {
    Serial.print("LCD begin failed, status=");
    Serial.println(status);
    while (true) delay(1000);
  }
  lcd.backlight();
  lcd.clear();
}

static void showStartup() {
  lcd.clear();
  lcdCenterPrint(0, "Welcome!");
  delay(500);
  lcdSetLine(1, DEVICE_NAME);
  delay(900);
}

void setup() {
  Serial.begin(115200);
  safeState.begin(64);

  initLcdOrHalt();

  lockServo.setPeriodHertz(50);
  lockServo.attach(SERVO_PIN, 500, 2400);

  if (safeState.locked()) {
    servoLock();
  } else {
    servoUnlock();
  }

  setupBle();
  showStartup();
  notifyState();
}

void loop() {
  if (safeState.locked()) {
    runLockedState();
  } else {
    runUnlockedState();
  }
}
