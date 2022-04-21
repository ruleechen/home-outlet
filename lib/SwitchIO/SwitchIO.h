#ifndef SwitchIO_h
#define SwitchIO_h

#include <DigitalInput.h>
#include <DigitalOutput.h>
#include "SwitchModels.h"
#include "SwitchStorage.h"

namespace Victor::Components {
  class SwitchIO {
   public:
    SwitchIO();
    void loop();
    bool getInputState();
    void setOutputState(bool on);
    // events
    typedef std::function<void(bool on)> TStateHandler;
    TStateHandler onInputChange = nullptr;

   private:
    DigitalInput* _input = nullptr;
    DigitalOutput* _output = nullptr;
    bool _inputState = false;
    unsigned long _lastLoop = 0;
  };

} // namespace Victor::Components

#endif // SwitchIO_h
