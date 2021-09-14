#ifndef __DS3231RTC_H
#define __DS3231RTC_H

#include <stdint.h>

class DS3231RTC {
 public:
  void begin();
  // Starts a countdown for the specified duration, in minutes.
  // Warning: The duration must be less than 28 days.
  bool countdown(uint16_t duration_minutes);
  // Returns the remaining minutes in the countdown, or zero if the countdown
  // has ended.
  uint16_t remainingMinutes();
};

#endif
