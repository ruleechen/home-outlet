#include <Arduino.h>
#include <arduino_homekit_server.h>

#include <Console.h>
#include <DigitalOutput.h>
#include <BuiltinLed.h>
#include <VictorOTA.h>
#include <VictorWifi.h>
#include <VictorWeb.h>

#include "TimesCounter.h"
#include "SwitchIO.h"

using namespace Victor;
using namespace Victor::Events;
using namespace Victor::Components;

VictorWeb webPortal(80);
TimesCounter times(1000);
SwitchIO* switchIO;
String hostName;
String serialNumber;
DigitalOutput* light;

extern "C" homekit_characteristic_t switchState;
extern "C" homekit_characteristic_t accessoryName;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_server_config_t serverConfig;

String toYesNoName(bool state) {
  return state ? F("Yes") : F("No");
}

String toSwitchStateName(bool state) {
  return state ? F("On") : F("Off");
}

void setSwitchState(const bool value) {
  ESP.wdtFeed();
  builtinLed.flash();
  times.count();
  switchState.value.bool_value = value;
  homekit_characteristic_notify(&switchState, switchState.value);
  switchIO->setOutputState(value);
  light->setValue(value);
  console.log()
    .bracket(F("switch"))
    .section(F("state"), toSwitchStateName(value));
}

void setup(void) {
  console.begin(115200);
  if (!LittleFS.begin()) {
    console.error()
      .bracket(F("fs"))
      .section(F("mount failed"));
  }

  builtinLed.setup();
  builtinLed.turnOn();

  light = new DigitalOutput(14, LOW);
  light->setValue(false);

  // setup web
  webPortal.onRequestStart = []() { builtinLed.toggle(); };
  webPortal.onRequestEnd = []() { builtinLed.toggle(); };
  webPortal.onRadioEmit = [](uint8_t index) { };
  webPortal.onServiceGet = [](std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons) {
    // states
    states.push_back({ .text = F("Service"), .value = VICTOR_ACCESSORY_SERVICE_NAME });
    states.push_back({ .text = F("State"),   .value = toSwitchStateName(switchState.value.bool_value) });
    states.push_back({ .text = F("Paired"),  .value = toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"), .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("Unpair"), .value = F("Unpair") });
    buttons.push_back({ .text = F("Toggle"), .value = F("Toggle") });
  };
  webPortal.onServicePost = [](const String& value) {
    if (value == F("Unpair")) {
      homekit_server_reset();
    } else if (value == F("Toggle")) {
      setSwitchState(!switchState.value.bool_value);
    }
  };
  webPortal.setup();

  // setup switch io
  switchIO = new SwitchIO();
  switchIO->onInputChange = setSwitchState;

  // setup homekit server
  hostName = victorWifi.getHostName();
  serialNumber = String(VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER) + "/" + victorWifi.getHostId();
  accessoryName.value.string_value = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value =const_cast<char*>(serialNumber.c_str());
  switchState.setter = [](const homekit_value_t value) { setSwitchState(value.bool_value); };
  arduino_homekit_setup(&serverConfig);

  // counter
  times.onCount = [](uint8_t count) {
    if (count == 10) {
      homekit_server_reset();
      ESP.restart();
    }
  };

  // setup wifi
  victorOTA.setup();
  victorWifi.setup();

  // done
  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  arduino_homekit_loop();
  webPortal.loop();
  switchIO->loop();
}
