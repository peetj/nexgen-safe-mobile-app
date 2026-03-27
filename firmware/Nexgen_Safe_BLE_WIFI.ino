/*
  Nexgen Safe (ESP32 + Keypad + Servo + I2C LCD) + Wi-Fi AP + HTTP API

  Why Wi-Fi AP:
  - Works on iPhone immediately (Safari can do HTTP; iOS Web Bluetooth is not available)
  - No school Wi-Fi dependency
  - WPA2 hotspot security: must join the safe's Wi-Fi with the shared password to control it
  - Browser-first target; BLE lives in Nexgen_Safe_BLE.ino if needed

  Network:
  - WPA2-PSK AP
  - SSID = DEVICE_NAME
  - Password = WIFI_PASSWORD
  - IP = 192.168.4.1

  HTTP API:
    GET  /status
    POST /lock    {"pin":"1234"}
    POST /unlock  {"pin":"1234"}
    POST /setpin  {"pin1":"1234","pin2":"1234"}

  Minimal LCD text:
  - Shows WiFi on boot and leaves normal keypad UI afterwards.

  Libraries:
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
#include <ESPmDNS.h>

#include "SafeState.h"
#include "icons.h"
#include "web_ui.h"

// -----------------------------
// Config
// -----------------------------
static const char* DEVICE_NAME = "NexgenSafe-01"; // <-- students change this
static constexpr char WIFI_PASSWORD[] = "nexgensafe"; // <-- students change this, 8-63 chars
static_assert(sizeof(WIFI_PASSWORD) - 1 >= 8 && sizeof(WIFI_PASSWORD) - 1 <= 63, "WIFI_PASSWORD must be 8-63 characters");

static const uint8_t LCD_COLS = 16;
static const uint8_t LCD_ROWS = 2;

static const uint8_t SERVO_PIN        = 19;
static const uint8_t SERVO_LOCK_POS   = 0;
static const uint8_t SERVO_UNLOCK_POS = 90;

static const uint8_t CODE_LEN = 4;
static const uint8_t DEVICE_NAME_MAX_LEN = 31;
static const uint8_t EEPROM_ADDR_DEVICE_NAME_LEN = 32;
static const uint8_t EEPROM_ADDR_DEVICE_NAME = 33;
static const uint8_t EEPROM_EMPTY = 0xff;

// Keypad pins (R1..R4, C1..C3)
static const uint8_t PIN_R1 = 14;
static const uint8_t PIN_R2 = 32;
static const uint8_t PIN_R3 = 33;
static const uint8_t PIN_R4 = 26;

static const uint8_t PIN_C1 = 27;
static const uint8_t PIN_C2 = 12;
static const uint8_t PIN_C3 = 25;

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
static String deviceNameValue = DEVICE_NAME;
static String deviceHostLabel;
static String wifiPasswordValue = WIFI_PASSWORD;
static bool restartPending = false;
static unsigned long restartAtMs = 0;

enum UiMirrorEvent : uint8_t {
  UI_MIRROR_NONE = 0,
  UI_MIRROR_REMOTE_LOCKED,
  UI_MIRROR_REMOTE_UNLOCKED,
  UI_MIRROR_REMOTE_DENIED,
  UI_MIRROR_REMOTE_PIN_UPDATED
};

static UiMirrorEvent pendingUiMirrorEvent = UI_MIRROR_NONE;

// -----------------------------
// Wi-Fi + HTTP
// -----------------------------
WebServer server(80);

static inline void serviceRemoteClients() {
  server.handleClient();
  delay(2);
}

static bool hasPendingUiMirrorEvent() {
  return pendingUiMirrorEvent != UI_MIRROR_NONE;
}

static void queueUiMirrorEvent(UiMirrorEvent eventType, const char* reason) {
  pendingUiMirrorEvent = eventType;
  Serial.print("Queued UI mirror event: ");
  Serial.println(reason);
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

static String normalizeDeviceName(const String& raw) {
  String name = raw;
  name.trim();
  return name;
}

static bool isValidDeviceName(const String& name) {
  if (name.length() == 0 || name.length() > DEVICE_NAME_MAX_LEN) return false;
  for (size_t i = 0; i < name.length(); i++) {
    char c = name[i];
    bool ok = (c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == ' ' || c == '-' || c == '_';
    if (!ok) return false;
  }
  return true;
}

static String makeHostLabel(const char* source) {
  String host;
  bool lastWasDash = false;

  for (size_t i = 0; source[i] != '\0'; i++) {
    char c = source[i];
    if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');

    bool isAlphaNum = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
    if (isAlphaNum) {
      host += c;
      lastWasDash = false;
    } else if (!lastWasDash && host.length() > 0) {
      host += '-';
      lastWasDash = true;
    }

    if (host.length() >= 48) break;
  }

  while (host.endsWith("-")) {
    host.remove(host.length() - 1);
  }

  if (host.length() == 0) {
    host = "nexgen-safe";
  }

  return host;
}

static String localAccessUrl() {
  if (deviceHostLabel.length() == 0) return "";
  return String("http://") + deviceHostLabel + ".local";
}

static String jsonEscape(const String& raw) {
  String out;
  out.reserve(raw.length() + 8);

  for (size_t i = 0; i < raw.length(); i++) {
    char c = raw[i];
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '"': out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default: out += c; break;
    }
  }

  return out;
}

static void applyDeviceName(const String& name) {
  deviceNameValue = name;
  deviceHostLabel = makeHostLabel(deviceNameValue.c_str());
}

static void loadDeviceNameFromEeprom() {
  uint8_t len = EEPROM.read(EEPROM_ADDR_DEVICE_NAME_LEN);
  if (len == EEPROM_EMPTY || len == 0 || len > DEVICE_NAME_MAX_LEN) {
    applyDeviceName(DEVICE_NAME);
    return;
  }

  String stored;
  stored.reserve(len);
  for (uint8_t i = 0; i < len; i++) {
    char c = (char)EEPROM.read(EEPROM_ADDR_DEVICE_NAME + i);
    if (c == '\0' || c == (char)EEPROM_EMPTY) break;
    stored += c;
  }

  stored = normalizeDeviceName(stored);
  if (!isValidDeviceName(stored)) {
    applyDeviceName(DEVICE_NAME);
    return;
  }

  applyDeviceName(stored);
}

static void saveDeviceNameToEeprom(const String& name) {
  uint8_t len = (uint8_t)name.length();
  EEPROM.write(EEPROM_ADDR_DEVICE_NAME_LEN, len);
  for (uint8_t i = 0; i < DEVICE_NAME_MAX_LEN; i++) {
    uint8_t value = (i < len) ? (uint8_t)name[i] : 0;
    EEPROM.write(EEPROM_ADDR_DEVICE_NAME + i, value);
  }
  EEPROM.commit();
}

static void scheduleRestart(const char* reason, unsigned long delayMs = 1500) {
  restartPending = true;
  restartAtMs = millis() + delayMs;
  Serial.print("Restart scheduled: ");
  Serial.println(reason);
}

static bool shouldAbortInteractiveWait() {
  return restartPending || hasPendingUiMirrorEvent();
}

static char readLoggedKey(const char* context) {
  char k = keypad.getKey();
  if (k) {
    Serial.print("[KEYPAD] ");
    Serial.print(context);
    Serial.print(" key='");
    Serial.print(k);
    Serial.println("'");
  }
  return k;
}

static bool is4Digits(const String& s) {
  if (s.length() != 4) return false;
  for (uint8_t i = 0; i < 4; i++) {
    if (s[i] < '0' || s[i] > '9') return false;
  }
  return true;
}

static void servoLock() {
  lockServo.write(SERVO_LOCK_POS);
  safeState.lock();
}

static void servoUnlock() {
  lockServo.write(SERVO_UNLOCK_POS);
}

static bool verifyPinNoSideEffects(const String& code) {
  const uint8_t EEPROM_ADDR_CODE_LEN = 1;
  const uint8_t EEPROM_ADDR_CODE = 2;

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
  String json = String("{\"name\":\"") + jsonEscape(deviceNameValue) +
                "\",\"host\":\"" + jsonEscape(deviceHostLabel + ".local") +
                "\",\"url\":\"" + jsonEscape(localAccessUrl()) +
                "\",\"password\":\"" + jsonEscape(wifiPasswordValue) +
                "\",\"locked\":" + (safeState.locked() ? "true" : "false") + "}";
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
    if (safeState.hasCode() && !verifyPinNoSideEffects(pin)) {
      queueUiMirrorEvent(UI_MIRROR_REMOTE_DENIED, "remote lock denied");
      return httpJson(403, "{\"ok\":false,\"err\":\"BAD_PIN\"}");
    }
    servoLock();
    queueUiMirrorEvent(UI_MIRROR_REMOTE_LOCKED, "remote lock");
    httpJson(200, "{\"ok\":true}");
  });

  server.on("/unlock", HTTP_POST, []() {
    String body = server.arg("plain");
    String pin = jsonGet(body, "pin");
    if (!is4Digits(pin)) return httpJson(400, "{\"ok\":false,\"err\":\"BAD_FORMAT\"}");
    bool ok = safeState.unlock(pin);
    if (!ok) {
      queueUiMirrorEvent(UI_MIRROR_REMOTE_DENIED, "remote unlock denied");
      return httpJson(403, "{\"ok\":false,\"err\":\"BAD_PIN\"}");
    }
    servoUnlock();
    queueUiMirrorEvent(UI_MIRROR_REMOTE_UNLOCKED, "remote unlock");
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
    queueUiMirrorEvent(UI_MIRROR_REMOTE_PIN_UPDATED, "remote pin update");
    httpJson(200, "{\"ok\":true}");
  });

  server.on("/rename", HTTP_POST, []() {
    String body = server.arg("plain");
    String requestedName = normalizeDeviceName(jsonGet(body, "name"));
    if (!isValidDeviceName(requestedName)) return httpJson(400, "{\"ok\":false,\"err\":\"BAD_NAME\"}");

    String newHost = makeHostLabel(requestedName.c_str());
    String newUrl = String("http://") + newHost + ".local";
    bool restartRequired = requestedName != deviceNameValue;

    saveDeviceNameToEeprom(requestedName);

    Serial.print("Device rename requested: ");
    Serial.println(requestedName);

    String json = String("{\"ok\":true,\"restart_required\":") + (restartRequired ? "true" : "false") +
                  ",\"name\":\"" + jsonEscape(requestedName) +
                  "\",\"host\":\"" + jsonEscape(newHost + ".local") +
                  "\",\"url\":\"" + jsonEscape(newUrl) +
                  "\",\"password\":\"" + jsonEscape(wifiPasswordValue) + "\"}";
    httpJson(200, json);
  });

  server.on("/restart", HTTP_POST, []() {
    httpJson(200, "{\"ok\":true,\"restarting\":true}");
    scheduleRestart("user requested restart", 350);
  });

  server.onNotFound([]() {
    httpJson(404, "{\"ok\":false,\"err\":\"NOT_FOUND\"}");
  });

  // CORS preflight
  server.on("/status", HTTP_OPTIONS, []() { httpJson(204, "{}"); });
  server.on("/lock", HTTP_OPTIONS, []() { httpJson(204, "{}"); });
  server.on("/unlock", HTTP_OPTIONS, []() { httpJson(204, "{}"); });
  server.on("/setpin", HTTP_OPTIONS, []() { httpJson(204, "{}"); });
  server.on("/rename", HTTP_OPTIONS, []() { httpJson(204, "{}"); });
  server.on("/restart", HTTP_OPTIONS, []() { httpJson(204, "{}"); });

  server.begin();
  Serial.println("HTTP server ready at http://192.168.4.1");
  if (deviceHostLabel.length()) {
    Serial.print("HTTP server also advertised at ");
    Serial.println(localAccessUrl());
  }
}

static bool setupWifiAp() {
  const IPAddress apIp(192, 168, 4, 1);
  const IPAddress apGateway(192, 168, 4, 1);
  const IPAddress apSubnet(255, 255, 255, 0);

  Serial.println();
  Serial.println("== WiFi AP setup ==");
  Serial.print("Requested SSID: ");
  Serial.println(deviceNameValue);

  WiFi.persistent(false);
  WiFi.disconnect(true, true);
  delay(100);

  bool modeOk = WiFi.mode(WIFI_MODE_AP);
  bool configOk = WiFi.softAPConfig(apIp, apGateway, apSubnet);
  bool hostOk = WiFi.softAPsetHostname(deviceHostLabel.c_str());
  bool apOk = WiFi.softAP(deviceNameValue.c_str(), wifiPasswordValue.c_str());
  delay(250);

  bool mdnsOk = false;
  if (apOk) {
    mdnsOk = MDNS.begin(deviceHostLabel.c_str());
    if (mdnsOk) {
      MDNS.setInstanceName(deviceNameValue);
      MDNS.enableWorkstation(ESP_IF_WIFI_AP);
      MDNS.addService("http", "tcp", 80);
    }
  }

  IPAddress currentIp = WiFi.softAPIP();

  Serial.print("WiFi.mode(AP): ");
  Serial.println(modeOk ? "OK" : "FAIL");
  Serial.print("WiFi.softAPConfig: ");
  Serial.println(configOk ? "OK" : "FAIL");
  Serial.print("WiFi.softAPsetHostname: ");
  Serial.println(hostOk ? "OK" : "FAIL");
  Serial.print("WiFi.softAP: ");
  Serial.println(apOk ? "OK" : "FAIL");
  Serial.print("Active SSID: ");
  Serial.println(WiFi.softAPSSID());
  Serial.print("WPA2 password: ");
  Serial.println(wifiPasswordValue);
  Serial.print("Active device name: ");
  Serial.println(deviceNameValue);
  Serial.print("AP hostname: ");
  Serial.println(deviceHostLabel);
  Serial.print("mDNS: ");
  Serial.println(mdnsOk ? "OK" : "FAIL");
  Serial.print("AP IP: ");
  Serial.println(currentIp);
  if (mdnsOk) {
    Serial.print("Open via: ");
    Serial.println(localAccessUrl());
  }

  lcd.clear();
  if (apOk) {
    lcdSetLine(0, "WiFi: " + deviceNameValue);
    lcdSetLine(1, "IP: " + currentIp.toString());
    delay(1400);
    lcdSetLine(0, "WiFi password");
    lcdSetLine(1, wifiPasswordValue);
    delay(1400);
  } else {
    lcdSetLine(0, "WiFi AP failed");
    lcdSetLine(1, "Check serial log");
    delay(2500);
  }

  return apOk;
}

// -----------------------------
// Existing keypad UI logic
// -----------------------------
static bool readNumericCodeMasked(uint8_t length, const char* context, String& code) {
  lcd.setCursor(5, 1);
  lcd.print("[____]");
  lcd.setCursor(6, 1);

  code.reserve(length);

  while (code.length() < length) {
    serviceRemoteClients();
    if (shouldAbortInteractiveWait()) return false;
    char k = readLoggedKey(context);
    if (k >= '0' && k <= '9') {
      lcd.print('*');
      code += k;
    }
  }
  return true;
}

static bool promptSetNewCode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter new code:");
  String newCode;
  if (!readNumericCodeMasked(CODE_LEN, "new-pin", newCode)) return false;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Confirm new code");
  String confirm;
  if (!readNumericCodeMasked(CODE_LEN, "confirm-pin", confirm)) return false;

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

static char waitForKey(char a, char b, const char* context) {
  char k = readLoggedKey(context);
  while (k != a && k != b) {
    serviceRemoteClients();
    if (shouldAbortInteractiveWait()) return '\0';
    k = readLoggedKey(context);
  }
  return k;
}

static void waitWithRemoteService(uint16_t totalMs) {
  unsigned long startedAt = millis();
  while ((uint32_t)(millis() - startedAt) < totalMs) {
    serviceRemoteClients();
  }
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
  waitWithRemoteService(900);
}

static void showLockedBanner() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(ICON_LOCKED_CHAR);
  lcd.print(" Safe Locked! ");
  lcd.write(ICON_LOCKED_CHAR);
}

static void showAccessDeniedBanner() {
  lcd.clear();
  lcdCenterPrint(0, "Access Denied!");
  showProgressBar(400);
}

static void showRemoteLockTransition() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.write(ICON_UNLOCKED_CHAR);
  lcd.print(" ");
  lcd.write(ICON_RIGHT_ARROW);
  lcd.print(" ");
  lcd.write(ICON_LOCKED_CHAR);
  showProgressBar(100);
}

static void showPinUpdatedBanner() {
  lcd.clear();
  lcdCenterPrint(0, "Code updated");
  lcdCenterPrint(1, "Ready to lock");
  waitWithRemoteService(1100);
}

static bool runPendingUiMirrorEvent() {
  UiMirrorEvent eventType = pendingUiMirrorEvent;
  if (eventType == UI_MIRROR_NONE) return false;

  pendingUiMirrorEvent = UI_MIRROR_NONE;

  switch (eventType) {
    case UI_MIRROR_REMOTE_LOCKED:
      showRemoteLockTransition();
      return true;
    case UI_MIRROR_REMOTE_UNLOCKED:
      showProgressBar(200);
      showUnlockedBanner();
      return true;
    case UI_MIRROR_REMOTE_DENIED:
      showAccessDeniedBanner();
      return true;
    case UI_MIRROR_REMOTE_PIN_UPDATED:
      showPinUpdatedBanner();
      return true;
    case UI_MIRROR_NONE:
    default:
      return false;
  }
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

  char k = waitForKey('*', '#', "unlocked-menu");
  if (!k) return;

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

  String userCode;
  if (!readNumericCodeMasked(CODE_LEN, "unlock-pin", userCode)) return;

  bool ok = safeState.unlock(userCode);
  showProgressBar(200);

  if (ok) {
    showUnlockedBanner();
    servoUnlock();
  } else {
    showAccessDeniedBanner();
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
  loadDeviceNameFromEeprom();

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
}

void loop() {
  serviceRemoteClients();

  if (restartPending && millis() >= restartAtMs) {
    Serial.println("Restarting ESP32 now...");
    delay(80);
    ESP.restart();
  }

  if (runPendingUiMirrorEvent()) {
    return;
  }

  if (safeState.locked()) {
    runLockedState();
  } else {
    runUnlockedState();
  }
}
