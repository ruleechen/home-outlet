#include "TimesCounter.h"

namespace Victor::Events {

  TimesCounter::TimesCounter(unsigned long resetMillis) {
    _resetMillis = resetMillis;
  }

  void TimesCounter::count() {
    // https://www.arduino.cc/reference/en/language/functions/time/millis/
    // Returns the number of milliseconds passed since the Arduino board began running the current program.
    // This number will overflow (go back to zero), after approximately 50 days.
    const auto now = millis();
    if (now - _last >= _resetMillis) {
      reset();
    }
    _count++;
    _last = now;
    if (onCount) {
      onCount(_count);
    }
  }

  void TimesCounter::reset() {
    _count = 0;
  }

} // namespace Victor::Events
