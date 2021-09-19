#include "schedule-util.h"

#include <functional>
#include <stdint.h>

#include "schedule-client.h"

namespace {
constexpr int kMinutesPerDay = 60 * 24;
} // namespace

int minutesUntil(int target_minute, int current_minute) {
  target_minute -= current_minute;
  // Wrap to the next day.
  if (target_minute < 0) {
    target_minute += kMinutesPerDay;
  }
  return target_minute;
}

int nearestMinutes(int target_minute, int current_minute) {
  target_minute -= current_minute;
  // Wrap to nearest day.
  if (target_minute <= -kMinutesPerDay / 2) {
    target_minute += kMinutesPerDay;
  }
  if (target_minute > kMinutesPerDay / 2) {
    target_minute -= kMinutesPerDay;
  }
  return target_minute;
}

int processSchedule(
    uint16_t* schedule_in_minutes, size_t schedule_size, int minute_of_day,
    int trigger_delta_minutes, std::function<void()> trigger_callback) {
  int wait_minutes = 0;
  int min_wait_minutes = 1;
  for (int i = 0; i < schedule_size; i++) {
    // If the schedule is not full, remaining slots are kScheduleNoEvent.
    if (schedule_in_minutes[i] == kScheduleNoEvent) {
      break;
    }
    // Trigger any event within the time delta.
    if (int delta_minutes =
          nearestMinutes(schedule_in_minutes[i], minute_of_day);
        abs(delta_minutes) <= trigger_delta_minutes) {
      trigger_callback();
      // Need to wait at least enough time to avoid duplicate triggers.
      min_wait_minutes =
        max(min_wait_minutes, delta_minutes + trigger_delta_minutes + 1);
      continue;
    }
    // Find the soonest upcoming event (not yet triggered).
    if (int minutes_until = minutesUntil(schedule_in_minutes[i], minute_of_day);
          wait_minutes == 0 || minutes_until < wait_minutes) {
      wait_minutes = minutes_until;
    }
  }
  if (wait_minutes < min_wait_minutes) {
    return min_wait_minutes;
  }
  return wait_minutes;
}
