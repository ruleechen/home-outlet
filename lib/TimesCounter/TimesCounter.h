#ifndef TimesCounter_h
#define TimesCounter_h

#include <functional>
#include <Arduino.h>

namespace Victor::Events {
  class TimesCounter {
    typedef std::function<void(uint8_t count)> TCountHandler;

   public:
    TimesCounter(unsigned long resetMillis);
    TCountHandler onCount;
    void count();
    void reset();

   private:
    // args
    unsigned long _resetMillis;
    // state
    uint8_t _count = 0;
    unsigned long _last = 0;
  };
} // namespace Victor::Events

#endif // TimesCounter_h
