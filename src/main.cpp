#include <Arduino.h>
#include <arduino_homekit_server.h>

#include <AppMain/AppMain.h>
#include <TimesCounter.h>
#include <SwitchIO/SwitchIO.h>

using namespace Victor;
using namespace Victor::Components;

extern "C" homekit_characteristic_t onState;
extern "C" homekit_characteristic_t inUseState;
extern "C" homekit_characteristic_t accessoryName;
extern "C" homekit_characteristic_t accessorySerialNumber;
extern "C" homekit_server_config_t serverConfig;

AppMain* appMain;
TimesCounter times(1000);
SwitchIO* switchIO;
String hostName;
String serialNumber;

void setOnState(const bool value) {
  ESP.wdtFeed();
  builtinLed.flash();
  onState.value.bool_value = value;
  homekit_characteristic_notify(&onState, onState.value);
  switchIO->setOutputState(value);
  console.log().section(F("state"), GlobalHelpers::toOnOffName(value));
}

void setInUseState(const bool value) {
  builtinLed.flash();
  inUseState.value.bool_value = value;
  homekit_characteristic_notify(&inUseState, inUseState.value);
  console.log().section(F("in use"), GlobalHelpers::toYesNoName(value));
}

void setup(void) {
  appMain = new AppMain();
  appMain->setup({
    .web = true,
    .radio = false,
  });

  // counter
  times.onCount = [](uint8_t count) {
    if (count == 20) {
      homekit_server_reset();
      ESP.restart();
    }
  };

  // setup web
  appMain->webPortal->onServiceGet = [](std::vector<TextValueModel>& states, std::vector<TextValueModel>& buttons) {
    // states
    states.push_back({ .text = F("Service"), .value = VICTOR_ACCESSORY_SERVICE_NAME });
    states.push_back({ .text = F("State"),   .value = GlobalHelpers::toOnOffName(onState.value.bool_value) });
    states.push_back({ .text = F("In Use"),  .value = GlobalHelpers::toYesNoName(inUseState.value.bool_value) });
    states.push_back({ .text = F("Paired"),  .value = GlobalHelpers::toYesNoName(homekit_is_paired()) });
    states.push_back({ .text = F("Clients"), .value = String(arduino_homekit_connected_clients_count()) });
    // buttons
    buttons.push_back({ .text = F("UnPair"), .value = F("UnPair") });
    buttons.push_back({ .text = F("Toggle"), .value = F("Toggle") });
  };
  appMain->webPortal->onServicePost = [](const String& value) {
    if (value == F("UnPair")) {
      homekit_server_reset();
      ESP.restart();
    } else if (value == F("Toggle")) {
      setOnState(!onState.value.bool_value);
    }
  };

  // setup homekit server
  hostName = victorWifi.getHostName();
  serialNumber = String(VICTOR_ACCESSORY_INFORMATION_SERIAL_NUMBER) + "/" + victorWifi.getHostId();
  accessoryName.value.string_value = const_cast<char*>(hostName.c_str());
  accessorySerialNumber.value.string_value = const_cast<char*>(serialNumber.c_str());
  onState.setter = [](const homekit_value_t value) { setOnState(value.bool_value); };
  arduino_homekit_setup(&serverConfig);

  // setup switch io
  switchIO = new SwitchIO("/switch.json");
  setOnState(switchIO->getOutputState());
  switchIO->input->onAction = [](const ButtonAction action) {
    console.log()
      .bracket(F("button"))
      .section(F("action"), String(action));
    if (action == ButtonActionPressed) {
      const auto outputValue = switchIO->getOutputState();
      setOnState(!outputValue); // toggle
    } else if (action == ButtonActionReleased) {
      times.count(); // count only for real button released
    } else if (action == ButtonActionDoublePressed) {
      builtinLed.flash(500);
      const auto enable = victorWifi.isLightSleepMode();
      victorWifi.enableAP(enable); // toggle enabling ap
    } else if (action == ButtonActionRestart) {
      ESP.restart();
    } else if (action == ButtonActionRestore) {
      homekit_server_reset();
      ESP.restart();
    }
  };

  // done
  console.log()
    .bracket(F("setup"))
    .section(F("complete"));
}

void loop(void) {
  appMain->loop();
  switchIO->loop();
  arduino_homekit_loop();
}
