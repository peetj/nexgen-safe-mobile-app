/*
  Nexgen Safe (ESP32 + Keypad + Servo + I2C LCD) + BLE + Wi-Fi AP + HTTP API

  Why Wi-Fi AP:
  - Works on iPhone immediately (Safari can do HTTP; iOS Web Bluetooth is not available)
  - No school Wi-Fi dependency
  - Security-by-proximity: must join the safe's Wi-Fi to control it

  Network:
  - Open AP (no password) as requested
  - SSID = DEVICE_NAME
  - IP = 192.168.4.1

  HTTP API:
    GET  /status
    POST /lock    {"pin":"1234"}
    POST /unlock  {"pin":"1234"}
    POST /setpin  {"pin1":"1234","pin2":"1234"}
    POST /lcd     {"line1":"...","line2":"..."}

  Minimal LCD text:
  - Shows WiFi on boot and leaves normal keypad UI afterwards.

  Libraries:
  - NimBLE-Arduino
  - WebServer (built-in ESP32 Arduino core)
*/

#include <Wire.h>
#include <Keypad.h>
#include <ESP32Servo.h>

#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>

#include <NimBLEDevice.h>

#include "SafeState.h"
#include "icons.h"
#include "web_ui.h"

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
// Wi-Fi + HTTP
// -----------------------------
WebServer server(80);

static inline void serviceRemoteClients() {
  server.handleClient();
  delay(2);
}

static inline void lcdCenterPrint(uint8_t row, const char* text) {
  size_t len = strlen(text);
  uint8_t col = (len >= LCD_COLS) ? 0 : (LCD_COLS - len) / 2;
  lcd.setCursor(col, row);
  lcd.print(text);
}

static void lcdSetLine(uint8_t line, const String& text) {
  String t = text;
  if (t.length() > LCD_COLS) t = t.substring(0, LCD_COLS);
  lcd.setCursor(0, line);
  lcd.print("                ");
  lcd.setCursor(0, line);
  lcd.print(t);
}

static bool is4Digits(const String& s) {
  if (s.length() != 4) return false;
  for (uint8_t i = 0; i < 4; i++) {
    if (s[i] < '0' || s[i] > '9') return false;
  }
  return true;
}

static void notifyState() {
  if (!chStatus) return;
  chStatus->setValue(safeState.locked() ? "STATE:LOCKED" : "STATE:UNLOCKED");
  chStatus->notify();
}

static void servoLock() {
  lockServo.write(SERVO_LOCK_POS);
  safeState.lock();
  notifyState();
}

static void servoUnlock() {
  lockServo.write(SERVO_UNLOCK_POS);
}

static void statusOk() {
  if (chStatus) {
    chStatus->setValue("OK");
    chStatus->notify();
  }
}

static void statusErr(const char* code) {
  if (chStatus) {
    String msg = String("ERR:") + code;
    chStatus->setValue(msg);
    chStatus->notify();
  }
}

static bool verifyPinNoSideEffects(const String& code) {
  const uint8_t EEPROM_ADDR_CODE_LEN = 1;
  const uint8_t EEPROM_ADDR_CODE = 2;
  const uint8_t EEPROM_EMPTY = 0xff;

  uint8_t codeLength = EEPROM.read(EEPROM_ADDR_CODE_LEN);
  if (codeLength == EEPROM_EMPTY) return true;
  if (code.length() != codeLength) return false;
  for (uint8_t i = 0; i < codeLength; i++) {
    uint8_t stored = EEPROM.read(EEPROM_ADDR_CODE + i);
    if (stored != (uint8_t)code[i]) return false;
  }
  return true;
}

// Minimal JSON parsing (KISS). Extracts "pin":"1234" etc.
static String jsonGet(const String& body, const char* key) {
  String k = String('"') + key + '"';
  int i = body.indexOf(k);
  if (i < 0) return "";
  i = body.indexOf(':', i);
  if (i < 0) return "";
  int q1 = body.indexOf('"', i);
  if (q1 < 0) return "";
  int q2 = body.indexOf('"', q1 + 1);
  if (q2 < 0) return "";
  return body.substring(q1 + 1, q2);
}

static void httpJson(int code, const String& json) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(code, "application/json", json);
}

static void httpOkStatus() {
  String json = String("{\"name\":\"") + DEVICE_NAME + "\",\"locked\":" + (safeState.locked() ? "true" : "false") + "}";
  httpJson(200, json);
}

static void setupHttp() {
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
  });

  server.on("/status", HTTP_GET, []() { httpOkStatus(); });

  server.on("/lock", HTTP_POST, []() {
    String body = server.arg("plain");
    String pin = jsonGet(body, "pin");
    if (!is4Digits(pin)) return httpJson(400, "{\"ok\":false,\"err\":\"BAD_FORMAT\"}");
    if (safeState.hasCode() && !verifyPinNoSideEffects(pin)) return httpJson(403, "{\"ok\":false,\"err\":\"BAD_PIN\"}");
    servoLock();
    httpJson(200, "{\"ok\":true}");
  });

  server.on("/unlock", HTTP_POST, []() {
    String body = server.arg("plain");
    String pin = jsonGet(body, "pin");
    if (!is4Digits(pin)) return httpJson(400, "{\"ok\":false,\"err\":\"BAD_FORMAT\"}");
    bool ok = safeState.unlock(pin);
    if (!ok) return httpJson(403, "{\"ok\":false,\"err\":\"BAD_PIN\"}");
    servoUnlock();
    notifyState();
    httpJson(200, "{\"ok\":true}");
  });

  server.on("/setpin", HTTP_POST, []() {
    String body = server.arg("plain");
    String p1 = jsonGet(body, "pin1");
    String p2 = jsonGet(body, "pin2");
    if (!is4Digits(p1) || !is4Digits(p2)) return httpJson(400, "{\"ok\":false,\"err\":\"BAD_FORMAT\"}");
    if (p1 != p2) return httpJson(400, "{\"ok\":false,\"err\":\"PIN_MISMATCH\"}");
    if (safeState.locked()) return httpJson(409, "{\"ok\":false,\"err\":\"LOCKED\"}");
    safeState.setCode(p1);
    httpJson(200, "{\"ok\":true}");
  });

  server.on("/lcd", HTTP_POST, []() {
    String body = server.arg("plain");
    String l1 = jsonGet(body, "line1");
    String l2 = jsonGet(body, "line2");
    if (l1.length()) lcdSetLine(0, l1);
    if (l2.length()) lcdSetLine(1, l2);
    httpJson(200, "{\"ok\":true}");
  });

  server.onNotFound([]() {
    httpJson(404, "{\"ok\":false,\"err\":\"NOT_FOUND\"}");
  });

  // CORS preflight
  server.on("/status", HTTP_OPTIONS, []() { httpJson(204, "{}"); });
  server.on("/lock", HTTP_OPTIONS, []() { httpJson(204, "{}"); });
  server.on("/unlock", HTTP_OPTIONS, []() { httpJson(204, "{}"); });
  server.on("/setpin", HTTP_OPTIONS, []() { httpJson(204, "{}"); });
  server.on("/lcd", HTTP_OPTIONS, []() { httpJson(204, "{}"); });

  server.begin();
  Serial.println("HTTP server ready at http://192.168.4.1");
}

static bool setupWifiAp() {
  const IPAddress apIp(192, 168, 4, 1);
  const IPAddress apGateway(192, 168, 4, 1);
  const IPAddress apSubnet(255, 255, 255, 0);

  Serial.println();
  Serial.println("== WiFi AP setup ==");
  Serial.print("Requested SSID: ");
  Serial.println(DEVICE_NAME);

  WiFi.persistent(false);
  WiFi.disconnect(true, true);
  delay(100);

  bool modeOk = WiFi.mode(WIFI_MODE_AP);
  bool configOk = WiFi.softAPConfig(apIp, apGateway, apSubnet);
  bool apOk = WiFi.softAP(DEVICE_NAME); // open network
  delay(250);

  IPAddress currentIp = WiFi.softAPIP();

  Serial.print("WiFi.mode(AP): ");
  Serial.println(modeOk ? "OK" : "FAIL");
  Serial.print("WiFi.softAPConfig: ");
  Serial.println(configOk ? "OK" : "FAIL");
  Serial.print("WiFi.softAP: ");
  Serial.println(apOk ? "OK" : "FAIL");
  Serial.print("Active SSID: ");
  Serial.println(WiFi.softAPSSID());
  Serial.print("AP IP: ");
  Serial.println(currentIp);

  lcd.clear();
  if (apOk) {
    lcdSetLine(0, "WiFi: " + String(DEVICE_NAME));
    lcdSetLine(1, "IP: " + currentIp.toString());
    delay(1500);
  } else {
    lcdSetLine(0, "WiFi AP failed");
    lcdSetLine(1, "Check serial log");
    delay(2500);
  }

  return apOk;
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

    if (s.startsWith("LOCK:")) {
      String pin = s.substring(5);
      pin.trim();
      if (!is4Digits(pin)) { statusErr("BAD_FORMAT"); return; }
      if (safeState.hasCode() && !verifyPinNoSideEffects(pin)) { statusErr("BAD_PIN"); return; }
      servoLock();
      statusOk();
      return;
    }

    if (s.startsWith("UNLOCK:")) {
      String pin = s.substring(7);
      pin.trim();
      if (!is4Digits(pin)) { statusErr("BAD_FORMAT"); return; }
      bool ok = safeState.unlock(pin);
      if (!ok) { statusErr("BAD_PIN"); return; }
      servoUnlock();
      notifyState();
      statusOk();
      return;
    }

    if (s.startsWith("SETPIN:")) {
      String rest = s.substring(7);
      int idx = rest.indexOf(':');
      if (idx < 0) { statusErr("BAD_FORMAT"); return; }
      String a = rest.substring(0, idx);
      String b = rest.substring(idx + 1);
      a.trim(); b.trim();
      if (!is4Digits(a) || !is4Digits(b)) { statusErr("BAD_FORMAT"); return; }
      if (a != b) { statusErr("PIN_MISMATCH"); return; }
      if (safeState.locked()) { statusErr("LOCKED"); return; }
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

    if (s.startsWith("LINE1:")) { lcdSetLine(0, s.substring(6)); statusOk(); return; }
    if (s.startsWith("LINE2:")) { lcdSetLine(1, s.substring(6)); statusOk(); return; }
    statusErr("BAD_FORMAT");
  }
};

static void setupBle() {
  NimBLEDevice::init(DEVICE_NAME);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  NimBLEServer* serverBle = NimBLEDevice::createServer();
  NimBLEService* svc = serverBle->createService(UUID_SVC);

  NimBLECharacteristic* chCmd = svc->createCharacteristic(UUID_CMD, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  chStatus = svc->createCharacteristic(UUID_STATUS, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  NimBLECharacteristic* chLcd = svc->createCharacteristic(UUID_LCD, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);

  chCmd->setCallbacks(new CmdCallbacks());
  chLcd->setCallbacks(new LcdCallbacks());

  chStatus->setValue("STATE:UNKNOWN");

  svc->start();

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(UUID_SVC);
  adv->enableScanResponse(true);
  adv->start();
}

// -----------------------------
// Existing keypad UI logic
// -----------------------------
static String readNumericCodeMasked(uint8_t length) {
  lcd.setCursor(5, 1);
  lcd.print("[____]");
  lcd.setCursor(6, 1);

  String code;
  code.reserve(length);

  while (code.length() < length) {
    serviceRemoteClients();
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
    return true;
  }

  lcd.clear();
  lcdCenterPrint(0, "Code mismatch");
  lcd.setCursor(0, 1);
  lcd.print("Safe not locked!");
  delay(2000);
  return false;
}

static char waitForKey(char a, char b) {
  char k = keypad.getKey();
  while (k != a && k != b) {
    serviceRemoteClients();
    k = keypad.getKey();
  }
  return k;
}

static void showProgressBar(uint16_t stepDelayMs) {
  lcd.setCursor(2, 1);
  lcd.print("[..........]");
  lcd.setCursor(3, 1);
  for (uint8_t i = 0; i < 10; i++) {
    uint16_t waited = 0;
    while (waited < stepDelayMs) {
      serviceRemoteClients();
      waited += 2;
    }
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

void setup() {
  Serial.begin(115200);
  safeState.begin(64);

  initLcdOrHalt();
  init_icons(lcd);

  lockServo.setPeriodHertz(50);
  lockServo.attach(SERVO_PIN, 500, 2400);

  if (safeState.locked()) {
    servoLock();
  } else {
    servoUnlock();
  }

  bool wifiReady = setupWifiAp();
  if (wifiReady) {
    setupHttp();
  } else {
    Serial.println("HTTP server skipped because WiFi AP did not start.");
  }

  setupBle();
  notifyState();
}

void loop() {
  serviceRemoteClients();

  if (safeState.locked()) {
    runLockedState();
  } else {
    runUnlockedState();
  }
}
