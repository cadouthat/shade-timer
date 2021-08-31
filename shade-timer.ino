#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "src/eeprom-wifi.h"

#define CHAIN_PULL_PIN 4
#define SERIAL_BAUD 115200

constexpr int kScheduleMaxEvents = 2;
constexpr uint16_t kScheduleNoEvent = 0xFFFF;
constexpr int kScheduleTriggerDelta_m = 5;

constexpr int kSecondsPerMinute = 60;
constexpr int kSecondsPerHour = kSecondsPerMinute * 60;
constexpr int kSecondsPerDay = kSecondsPerHour * 24;
constexpr int kMinutesPerDay = 24 * 60;
constexpr int kMicros = 1e6;

WiFiUDP ntpUDP;
NTPClient ntp(ntpUDP);

void toggleShade() {
  digitalWrite(CHAIN_PULL_PIN, LOW);
  delay(500);
  digitalWrite(CHAIN_PULL_PIN, HIGH);
}

int minutesUntil(int target_m, int current_m) {
  target_m -= current_m;
  // Wrap to the next day.
  if (target_m < 0) {
    target_m += kMinutesPerDay;
  }
  return target_m;
}

void setup() {
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(CHAIN_PULL_PIN, HIGH);
  pinMode(CHAIN_PULL_PIN, OUTPUT_OPEN_DRAIN);

  Serial.begin(SERIAL_BAUD);
  delay(500);

  uint16_t schedule_m[kScheduleMaxEvents] = {0};
  EEPROM.begin(kWiFiConfigSize + sizeof(schedule_m));
  initEEPROMWiFi();
  // TODO: Load schedule from server
  // TODO: Load back up schedule from EEPROM
  schedule_m[0] = 2 * 60; // 02:00 UTC -> 19:00 PDT
  schedule_m[1] = 16 * 60; // 16:00 UTC -> 09:00 PDT

  ntp.begin();
  ntp.update();
  int minute_in_day =
    (ntp.getEpochTime() % kSecondsPerDay) / kSecondsPerMinute;
  uint16_t next_event_m = kScheduleNoEvent;
  for (int i = 0; i < kScheduleMaxEvents; i++) {
    // If the schedule is not full, remaining slots are kScheduleNoEvent.
    if (schedule_m[i] == kScheduleNoEvent) {
      break;
    }
    // The first event found later in the day is the next event.
    if (schedule_m[i] > minute_in_day && next_event_m == kScheduleNoEvent) {
      next_event_m = schedule_m[i];
    }
    // Trigger the event if we are within the time delta.
    if (minutesUntil(minute_in_day, schedule_m[i]) <= kScheduleTriggerDelta_m) {
      toggleShade();
    }
  }
  // There are no events later in the day, the first event on the schedule is
  // next.
  if (next_event_m == kScheduleNoEvent) {
    next_event_m = schedule_m[0];
  }

  // Sleep until the next event, but wake up at least every hour to check for
  // schedule updates.
  int wait_s = kSecondsPerHour;
  if (next_event_m != kScheduleNoEvent) {
    wait_s = minutesUntil(next_event_m, minute_in_day) * kSecondsPerMinute;
    if (wait_s > kSecondsPerHour) {
      wait_s = kSecondsPerHour;
    }
  }
  uint64_t wait_us = wait_s * kMicros;
  // Make sure to avoid sleeping forever.
  if (wait_us < 1) {
    wait_us = 1;
  }
  if (wait_us > ESP.deepSleepMax()) {
    wait_us = ESP.deepSleepMax();
  }
  ESP.deepSleep(wait_us);
}

void loop() {}
