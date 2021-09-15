#ifndef __DS3231RTC_H
#define __DS3231RTC_H

#include <stdint.h>

class DS3231RTC {
 public:
  void begin();

  bool isTimeValid();
  int getMinuteOfDay();
  bool setMinuteOfDay(int minute_of_day);

  bool isAlarmActive();
  int getMinuteOfDayAlarm();
  bool setMinuteOfDayAlarm(int minute_of_day);
};

#endif
