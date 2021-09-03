#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "src/eeprom-wifi.h"
#include "src/ds3231rtc.h"

#define CHAIN_PULL_PIN 13
#define SERIAL_BAUD 115200

constexpr int kSecondsPerMinute = 60;
constexpr int kSecondsPerDay = kSecondsPerMinute * 60 * 24;
constexpr int kMinutesPerDay = 60 * 24;

constexpr int kScheduleMaxEvents = 2;
constexpr uint16_t kScheduleNoEvent = 0xFFFF;
constexpr int kScheduleTriggerDeltaMinutes = 5;
// Wake up and check for schedule changes at least once per hour.
constexpr uint16_t kMaxWaitMinutes = 60;

WiFiUDP ntpUDP;
NTPClient ntp(ntpUDP);
DS3231RTC rtc;
uint16_t schedule_in_minutes[kScheduleMaxEvents] = {0};

void toggleShade() {
  digitalWrite(CHAIN_PULL_PIN, LOW);
  delay(500);
  digitalWrite(CHAIN_PULL_PIN, HIGH);
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

void setup() {
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(CHAIN_PULL_PIN, HIGH);
  pinMode(CHAIN_PULL_PIN, OUTPUT_OPEN_DRAIN);

  Serial.begin(SERIAL_BAUD);
  delay(500);
  Serial.println();
  Serial.println("Awake");

  EEPROM.begin(kWiFiConfigSize + sizeof(schedule_in_minutes));
  initEEPROMWiFi();
  // TODO: Load schedule from server
  // TODO: Load back up schedule from EEPROM
  schedule_in_minutes[0] = 2 * 60; // 02:00 UTC -> 19:00 PDT
  schedule_in_minutes[1] = 16 * 60; // 16:00 UTC -> 09:00 PDT

  ntp.begin();
  rtc.begin();
}

void loop() {
  ntp.update();
  int minute_in_day =
    (ntp.getEpochTime() % kSecondsPerDay) / kSecondsPerMinute;
  uint16_t event_minute = kScheduleNoEvent;
  int min_wait_minutes = 1;
  for (int i = 0; i < kScheduleMaxEvents; i++) {
    // If the schedule is not full, remaining slots are kScheduleNoEvent.
    if (schedule_in_minutes[i] == kScheduleNoEvent) {
      break;
    }
    // The first event at a later time of day is up next.
    if (schedule_in_minutes[i] > minute_in_day
        && event_minute == kScheduleNoEvent) {
      event_minute = schedule_in_minutes[i];
    }
    // Trigger any event within the time delta.
    if (int delta_minutes =
          nearestMinutes(schedule_in_minutes[i], minute_in_day);
        delta_minutes <= kScheduleTriggerDeltaMinutes) {
      toggleShade();
      // Need to wait at least enough time to avoid duplicate triggers.
      min_wait_minutes = delta_minutes + kScheduleTriggerDeltaMinutes + 1;
    }
  }
  // When there are no events at a later time of day, the first event on the
  // schedule is up next.
  if (event_minute == kScheduleNoEvent) {
    event_minute = schedule_in_minutes[0];
  }

  // Sleep until the next event, within limits.
  int wait_minutes = kMaxWaitMinutes;
  if (event_minute != kScheduleNoEvent) {
    wait_minutes = minutesUntil(event_minute, minute_in_day);
  }
  if (wait_minutes > kMaxWaitMinutes) {
    wait_minutes = kMaxWaitMinutes;
  }
  if (wait_minutes < min_wait_minutes) {
    wait_minutes = min_wait_minutes;
  }

  if (!rtc.countdown(wait_minutes)) {
    Serial.println("Failed to start timer!");
    delay(min_wait_minutes * kSecondsPerMinute * 1000);
    return;
  }
  Serial.println("Calling deepSleep");
  // Sleep until external reset from RTC.
  ESP.deepSleep(0);
}
