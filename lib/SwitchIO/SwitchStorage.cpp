#include "SwitchStorage.h"

namespace Victor::Components {

  SwitchStorage::SwitchStorage() {
    _filePath = "/switch.json";
  }

  void SwitchStorage::_serializeTo(const SwitchSetting& model, DynamicJsonDocument& doc) {
    const JsonArray pinArr = doc.createNestedArray(F("pin"));
    pinArr[0] = model.inputPin;
    pinArr[1] = model.outputPin;
    pinArr[2] = model.inputTrueValue;
    pinArr[3] = model.outputTrueValue;
    const JsonArray stateArr = doc.createNestedArray(F("state"));
    stateArr[0] = model.outputOn ? 1 : 0;
    stateArr[1] = model.saveOutput ? 1 : 0;
  }

  void SwitchStorage::_deserializeFrom(SwitchSetting& model, const DynamicJsonDocument& doc) {
    const auto pinArr = doc[F("pin")];
    model.inputPin = pinArr[0];
    model.outputPin = pinArr[1];
    model.inputTrueValue = pinArr[2];
    model.outputTrueValue = pinArr[3];
    const auto stateArr = doc[F("state")];
    model.outputOn = stateArr[0] == 1;
    model.saveOutput = stateArr[1] == 1;
  }

  // global
  SwitchStorage switchStorage;

} // namespace Victor::Components
