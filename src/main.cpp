#include <Arduino.h>
#include <arduino_homekit_server.h>

#include <GlobalHelpers.h>
#include <Console.h>
#include <DigitalOutput.h>
#include <BuiltinLed.h>
#include <VictorOTA.h>
#include <VictorWifi.h>
#include <VictorWeb.h>

#include <TimesCounter.h>
#include <SwitchIO/SwitchIO.h>

using namespace Victor;
using namespace Victor::Components;

VictorWeb webPortal(80);
TimesCounter times(1000);
SwitchIO* switchIO;
String hostName;
String serialNumber;
DigitalOutput* light;

extern "C" homekit_characteristic_t onState;
extern "C" homekit_characteristic_t inUseState;
extern "C" homekit_characteristic_t accessoryName;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_server_config_t serverConfig;

void setOnState(const bool value) {
  ESP.wdtFeed();
  builtinLed.flash();
  times.count();
  onState.value.bool_value = value;
  homekit_characteristic_notify(&onState, onState.value);
  switchIO->setOutputState(value);
  light->setValue(value);
  console.log().section(F("state"), GlobalHelpers::toOnOffName(value));
}

void setInUseState(const bool value) {
  builtinLed.flash();
  inUseState.value.bool_value = value;
  homekit_characteristic_notify(&inUseState, inUseState.value);
  console.log().section(F("in use"), GlobalHelpers::toYesNoName(value));
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
    states.push_back({ .text = F("State"),   .value = GlobalHelpers::toOnOffName(onState.value.bool_value) });
    states.push_back({ .text = F("In Use"),  .value = GlobalHelpers::toYesNoName(inUseState.value.bool_value) });
    states.push_back({ .text = F("Paired"),  .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"), .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("Unpair"), .value = F("Unpair") });
    buttons.push_back({ .text = F("Toggle"), .value = F("Toggle") });
  };
  webPortal.onServicePost = [](const String& value) {
    if (value == F("Unpair")) {
      homekit_server_reset();
    } else if (value == F("Toggle")) {
      setOnState(!onState.value.bool_value);
    }
  };
  webPortal.setup();

  // setup switch io
  const auto storage = new SwitchStorage("/outlet.json");
  switchIO = new SwitchIO(storage);
  switchIO->onInputChange = setOnState;

  // setup homekit server
  hostName = victorWifi.getHostName();
  serialNumber = String(VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER) + "/" + victorWifi.getHostId();
  accessoryName.value.string_value = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value =const_cast<char*>(serialNumber.c_str());
  onState.setter = [](const homekit_value_t value) { setOnState(value.bool_value); };
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
