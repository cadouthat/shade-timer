#ifndef __DS3231RTC_H
#define __DS3231RTC_H

#include <stdint.h>

class DS3231RTC {
 public:
  void begin();
  // Triggers an interrupt after the specified duration in minutes.
  // Warning: The duration must be less than 28 days. To avoid repeat triggers,
  // this countdown must be replaced or cleared within 28 days of expiration.
  bool countdown(uint16_t duration_minutes);
};

#endif
