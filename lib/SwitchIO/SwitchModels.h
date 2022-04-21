#ifndef SwitchModels_h
#define SwitchModels_h

#include <Arduino.h>

namespace Victor {

  struct SwitchSetting {
    int8_t inputPin = -1;
    int8_t outputPin = -1;
    uint8_t inputTrueValue = 0;  // LOW
    uint8_t outputTrueValue = 0; // LOW
    bool outputOn = false;
    bool saveOutput = false;
  };

} // namespace Victor

#endif // SwitchModels_h
