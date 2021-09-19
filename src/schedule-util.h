#ifndef __SRC_SCHEDULE_UTIL_H
#define __SRC_SCHEDULE_UTIL_H

#include <functional>
#include <stdint.h>

constexpr uint16_t kScheduleNoEvent = 0xFFFF;

// Returns minute duration until the target time of day in the future, wrapping
// to the next day if needed.
int minutesUntil(int target_minute, int current_minute);
// Returns nearest relative minute delta to the target time of day, wrapping to
// the previous or next day if needed.
int nearestMinutes(int target_minute, int current_minute);
// Calls the provided function for any events within the trigger delta, and
// returns the minute duration to wait for the next event.
int processSchedule(
    uint16_t* schedule_in_minutes, size_t schedule_size, int minute_of_day,
    int trigger_delta_minutes, std::function<void()> trigger_callback);

#endif
