#include "SwitchIO.h"

namespace Victor::Components {

  SwitchIO::SwitchIO() {
    const auto model = switchStorage.load();
    _input = new DigitalInput(model.inputPin, model.inputTrueValue);
    _output = new DigitalOutput(model.outputPin, model.outputTrueValue);
    _inputState = getInputState();
    setOutputState(model.outputOn);
  }

  void SwitchIO::loop() {
    const auto now = millis();
    if (now - _lastLoop > 100) {
      _lastLoop = now;
      const auto inputOn = getInputState();
      if (inputOn != _inputState) {
        _inputState = inputOn;
        if (onInputChange && inputOn) {
          const auto outputState = _output->lastValue();
          onInputChange(!outputState); // toggle value
        }
      }
    }
  }

  bool SwitchIO::getInputState() {
    return _input->getValue();
  }

  void SwitchIO::setOutputState(bool on) {
    _output->setValue(on);
    // save output state
    auto model = switchStorage.load();
    if (model.saveOutput) {
      model.outputOn = on;
      switchStorage.save(model);
    }
  }

} // namespace Victor::Components
