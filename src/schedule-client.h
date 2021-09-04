#ifndef __SCHEDULE_CLIENT_H
#define __SCHEDULE_CLIENT_H

#include <stdint.h>

constexpr uint16_t kScheduleNoEvent = 0xFFFF;

bool fetchSchedule(uint16_t* output_minutes, int size);

#endif
