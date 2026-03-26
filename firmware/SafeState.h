#ifndef SAFESTATE_H
#define SAFESTATE_H

#include <Arduino.h>

class SafeState {
  public:
    SafeState();

    // Call once in setup() before using other methods
    void begin(uint16_t eepromSize = 64);

    void lock();
    bool unlock(const String& code);
    bool locked() const;
    bool hasCode() const;
    void setCode(const String& newCode);

  private:
    void setLock(bool locked);
    void loadFromEeprom();

    bool _locked = false;
    bool _begun = false;
};

#endif /* SAFESTATE_H */
