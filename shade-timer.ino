#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "src/eeprom-wifi.h"
#include "src/ds3231rtc.h"
#include "src/schedule-client.h"

#define CHAIN_PULL_PIN 13
#define SERIAL_BAUD 115200

// Enable/disable debugging print statements.
#if true
#define DBGLN(v) Serial.println(v)
#define DBG(v) Serial.print(v)
#else
#define DBGLN(v)
#define DBG(v)
#endif

constexpr int kSecondsPerMinute = 60;
constexpr int kSecondsPerDay = kSecondsPerMinute * 60 * 24;
constexpr int kMinutesPerDay = 60 * 24;
constexpr int kMicros = 1e6;

constexpr int kScheduleMaxEvents = 2;
constexpr int kScheduleTriggerDeltaMinutes = 5;
// Only go into deep sleep for short durations, to retain accuracy.
constexpr int kMaxDeepSleepMinutes = 15;
// Check for schedule changes at least every 4 hours.
constexpr uint16_t kMaxWaitMinutes = 240;

WiFiUDP ntpUDP;
NTPClient ntp(ntpUDP);
DS3231RTC rtc;
uint16_t schedule_in_minutes[kScheduleMaxEvents] = {0};

void toggleShade() {
  DBGLN(F("toggleShade() called"));
  digitalWrite(CHAIN_PULL_PIN, LOW);
  delay(500);
  digitalWrite(CHAIN_PULL_PIN, HIGH);
}

void deepSleepMinutes(uint64_t minutes) {
  DBG(F("Going into deepSleep() for "));
  DBG(minutes);
  DBGLN(F(" minutes"));
  ESP.deepSleep(minutes * kSecondsPerMinute * kMicros);
}

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

bool initTime() {
  if (!ntp.update()) {
    return false;
  }
  int minute_of_day =
    (ntp.getEpochTime() % kSecondsPerDay) / kSecondsPerMinute;
  DBG(F("initTime() at minute "));
  DBGLN(minute_of_day);
  return rtc.setMinuteOfDay(minute_of_day);
}

void setup() {
  digitalWrite(CHAIN_PULL_PIN, HIGH);
  pinMode(CHAIN_PULL_PIN, OUTPUT_OPEN_DRAIN);
  ntp.begin();
  rtc.begin();

  Serial.begin(SERIAL_BAUD);
  delay(500);
  DBGLN();
  DBGLN(F("Awake"));

  if (rtc.isTimeValid() && !rtc.isAlarmActive()) {
    if (int minutes =
          minutesUntil(rtc.getMinuteOfDayAlarm(), rtc.getMinuteOfDay());
        minutes > 0) {
      DBG(F("Alarm minutes remaining: "));
      DBGLN(minutes);
      deepSleepMinutes(min(minutes, kMaxDeepSleepMinutes));
    }
  }

  EEPROM.begin(kWiFiConfigSize + sizeof(schedule_in_minutes));
  // Block until WiFi is connected.
  while (!connectToWiFi()) {
    // Prompt for new configuration via serial.
    if (!configureWiFi()) {
      DBGLN(F("WiFi configuration failed!"));
      delay(1000);
    }
  }

  // Try to fetch the schedule from our server.
  if (fetchSchedule(schedule_in_minutes, kScheduleMaxEvents)) {
    // Write latest schedule to EEPROM, behind WiFi config.
    EEPROM.put(kWiFiConfigSize, schedule_in_minutes);
    // Commit only writes to flash if the data has actually changed.
    EEPROM.commit();
  } else {
    // Read last known schedule from EEPROM.
    EEPROM.get(kWiFiConfigSize, schedule_in_minutes);
  }
  DBGLN(F("Loaded schedule:"));
  for (int i = 0;
      i < kScheduleMaxEvents && schedule_in_minutes[i] != kScheduleNoEvent;
      i++) {
    DBG(schedule_in_minutes[i] / 60);
    DBG(F(":"));
    DBGLN(schedule_in_minutes[i] % 60);
  }
}

void loop() {
  if (!rtc.isTimeValid() && !initTime()) {
    DBGLN(F("Failed to initTime()!"));
    delay(1000);
    return;
  }
  int minute_of_day = rtc.getMinuteOfDay();
  DBG(F("loop() at minute "));
  DBGLN(minute_of_day);
  int wait_minutes = kMaxWaitMinutes;
  int min_wait_minutes = 1;
  for (int i = 0; i < kScheduleMaxEvents; i++) {
    // If the schedule is not full, remaining slots are kScheduleNoEvent.
    if (schedule_in_minutes[i] == kScheduleNoEvent) {
      break;
    }
    // Trigger any event within the time delta.
    if (int delta_minutes =
          nearestMinutes(schedule_in_minutes[i], minute_of_day);
        abs(delta_minutes) <= kScheduleTriggerDeltaMinutes) {
      toggleShade();
      // Need to wait at least enough time to avoid duplicate triggers.
      min_wait_minutes =
        max(min_wait_minutes, delta_minutes + kScheduleTriggerDeltaMinutes + 1);
      continue;
    }
    // Find the soonest upcoming event.
    if (int minutes_until = minutesUntil(schedule_in_minutes[i], minute_of_day);
          minutes_until < wait_minutes) {
      wait_minutes = minutes_until;
    }
  }
  if (wait_minutes < min_wait_minutes) {
    wait_minutes = min_wait_minutes;
  }

  DBG(F("Setting alarm minutes from now "));
  DBGLN(wait_minutes);
  if (!rtc.setMinuteOfDayAlarm(minute_of_day + wait_minutes)) {
    DBGLN(F("Failed to set alarm!"));
    // Try again after the minimum wait period.
    delay(min_wait_minutes * kSecondsPerMinute * 1000);
    return;
  }
  // Sleep as long as possible, or until the countdown ends.
  deepSleepMinutes(min(wait_minutes, kMaxDeepSleepMinutes));
}
