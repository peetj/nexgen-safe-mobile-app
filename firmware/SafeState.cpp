#include <Arduino.h>
#include <EEPROM.h>
#include "SafeState.h"

/* Safe state layout in EEPROM */
#define EEPROM_ADDR_LOCKED   0
#define EEPROM_ADDR_CODE_LEN 1
#define EEPROM_ADDR_CODE     2
#define EEPROM_EMPTY         0xff

#define SAFE_STATE_OPEN   (uint8_t)0
#define SAFE_STATE_LOCKED (uint8_t)1

SafeState::SafeState() {
  // Do not touch EEPROM here on ESP32.
  // We'll load state after EEPROM.begin() in begin().
}

void SafeState::begin(uint16_t eepromSize) {
  if (_begun) return;

  EEPROM.begin(eepromSize);
  _begun = true;
  loadFromEeprom();
}

void SafeState::loadFromEeprom() {
  uint8_t v = EEPROM.read(EEPROM_ADDR_LOCKED);
  _locked = (v == SAFE_STATE_LOCKED);
}

void SafeState::lock() {
  setLock(true);
}

bool SafeState::locked() const {
  return _locked;
}

bool SafeState::hasCode() const {
  uint8_t codeLength = EEPROM.read(EEPROM_ADDR_CODE_LEN);
  return codeLength != EEPROM_EMPTY;
}

void SafeState::setCode(const String& newCode) {
  uint8_t len = (uint8_t)newCode.length();

  EEPROM.write(EEPROM_ADDR_CODE_LEN, len);
  for (uint8_t i = 0; i < len; i++) {
    EEPROM.write(EEPROM_ADDR_CODE + i, (uint8_t)newCode[i]);
  }

  EEPROM.commit(); // REQUIRED on ESP32
}

bool SafeState::unlock(const String& code) {
  uint8_t codeLength = EEPROM.read(EEPROM_ADDR_CODE_LEN);

  if (codeLength == EEPROM_EMPTY) {
    // No code set, unlock always succeeds
    setLock(false);
    return true;
  }

  if (code.length() != codeLength) {
    return false;
  }

  for (uint8_t i = 0; i < codeLength; i++) {
    uint8_t stored = EEPROM.read(EEPROM_ADDR_CODE + i);
    if (stored != (uint8_t)code[i]) {
      return false;
    }
  }

  setLock(false);
  return true;
}

void SafeState::setLock(bool locked) {
  _locked = locked;
  EEPROM.write(EEPROM_ADDR_LOCKED, locked ? SAFE_STATE_LOCKED : SAFE_STATE_OPEN);
  EEPROM.commit(); // REQUIRED on ESP32
}
