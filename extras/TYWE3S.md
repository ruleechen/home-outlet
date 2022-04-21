## TYWE3S

1. Copy platformio board `tywe3s.json` to `~/.platformio/platforms/espressif8266/boards`

2. Set `platformio.ini` with `board = tywe3s`

3. Config `data/app.json` with `{"name":"VIC","led":[12,0,1],"wifi":["ssid","pass",1]}`

4. Config `data/switch.json` with `{"pin":[4,5,0,1],"state":[0,0]}`

5. Implement led indicator with following code in `main.cpp`
```cpp
#include <DigitalOutput.h>
DigitalOutput* light;

void setup(void) {
  light = new DigitalOutput(14, LOW);
  light->setValue(false);
}

void setSwitchState(const bool value) {
  light->setValue(value);
}
```
