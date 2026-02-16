#include <Arduino.h>
#include "icons.h"



const byte iconLocked[8]PROGMEM = {
  0b00000,
  0b01110,
  0b10101,
  0b11011,
  0b01110,
  0b01110,
  0b00000,
  0b00000
};

const byte iconUnlocked[8]PROGMEM = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000,
};

void init_icons(LiquidCrystal_I2C &lcd) {
  byte icon[8];
  memcpy_P(icon, iconLocked, sizeof(icon));
  lcd.createChar(ICON_LOCKED_CHAR, icon);
  memcpy_P(icon, iconUnlocked, sizeof(icon));
  lcd.createChar(ICON_UNLOCKED_CHAR, icon);
}
