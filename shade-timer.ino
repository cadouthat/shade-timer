#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <eeprom-wifi.h>

#include "src/ds3231rtc.h"
#include "src/schedule-client.h"
#include "src/schedule-util.h"

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

constexpr uint32_t kSecondsPerMinute = 60;
constexpr uint32_t kSecondsPerDay = kSecondsPerMinute * 60 * 24;
constexpr uint32_t kMicros = 1e6;

constexpr int kScheduleTriggerDeltaMinutes = 5;
// Only go into deep sleep for short durations, to retain accuracy.
constexpr int kMaxDeepSleepMinutes = 15;
// Check for schedule changes at least every 4 hours.
constexpr int kMaxWaitMinutes = 240;

WiFiUDP ntpUDP;
NTPClient ntp(ntpUDP);
DS3231RTC rtc;
constexpr size_t kScheduleMaxEvents = 2;
uint16_t schedule_in_minutes[kScheduleMaxEvents] = {0};

void toggleShade() {
  DBGLN(F("toggleShade() called"));
  digitalWrite(CHAIN_PULL_PIN, LOW);
  delay(500);
  digitalWrite(CHAIN_PULL_PIN, HIGH);
}

void deepSleepMinutes(uint64_t minutes) {
  DBG(F("Going into deepSleep() for minutes "));
  DBGLN(minutes);
  ESP.deepSleep(minutes * kSecondsPerMinute * kMicros);
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
  DBGLN(F("\nAwake"));

  // Go back into deep sleep if we are still waiting for an alarm on the RTC.
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

  int wait_minutes = processSchedule(
    schedule_in_minutes, kScheduleMaxEvents, minute_of_day,
    kScheduleTriggerDeltaMinutes, toggleShade);
  if (wait_minutes == 0 || wait_minutes > kMaxWaitMinutes) {
    wait_minutes = kMaxWaitMinutes;
  }

  DBG(F("Setting alarm minutes from now "));
  DBGLN(wait_minutes);
  if (!rtc.setMinuteOfDayAlarm(minute_of_day + wait_minutes)) {
    DBGLN(F("Failed to set alarm!"));
    delay((uint32_t)wait_minutes * kSecondsPerMinute * 1000);
    return;
  }
  deepSleepMinutes(min(wait_minutes, kMaxDeepSleepMinutes));
}
