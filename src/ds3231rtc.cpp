#include "ds3231rtc.h"

#include <stdint.h>

#include <Arduino.h>
#include <Wire.h>

namespace {
constexpr uint16_t kMinutesPerHour = 60;
constexpr uint16_t kHoursPerDay = 24;
constexpr uint8_t kMinDaysPerMonth = 28;

constexpr uint8_t DS3231_ADDRESS = 0x68;
constexpr uint8_t DS3231_TIME = 0x00;
constexpr uint8_t DS3231_ALARM1 = 0x07;
constexpr uint8_t DS3231_CONTROL = 0x0E;
constexpr uint8_t DS3231_STATUS = 0x0F;

constexpr uint8_t DS3231_CONTROL_DEFALT = 0b11100;
constexpr uint8_t DS3231_ALARM1_ENABLE = bit(0);

uint8_t binToBCD(uint8_t bin) {
  return (bin / 10 << 4) | (bin % 10);
}

bool write(uint8_t write_register, uint8_t value) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(write_register);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool resetTime() {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_TIME);
  Wire.write(0); // Second
  Wire.write(0); // Minute
  Wire.write(0); // Hour
  Wire.write(1); // Weekday (ignored)
  Wire.write(1); // Day of month
  Wire.write(1); // Month/century
  Wire.write(0); // Year
  return Wire.endTransmission() == 0;
}

bool disableAlarm() {
  return write(DS3231_CONTROL, DS3231_CONTROL_DEFALT);
}

bool enableAlarm() {
  return write(DS3231_CONTROL, DS3231_CONTROL_DEFALT | DS3231_ALARM1_ENABLE);
}

bool setAlarmTime(uint16_t duration) {
  uint8_t minutes = duration % kMinutesPerHour;
  duration /= kMinutesPerHour;
  uint8_t hours = duration % kHoursPerDay;
  duration /= kHoursPerDay;
  if (duration >= kMinDaysPerMonth) {
    return false;
  }
  uint8_t days = duration;
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_ALARM1);
  Wire.write(0); // Second
  Wire.write(binToBCD(minutes)); // Minute
  Wire.write(binToBCD(hours)); // Hour
  Wire.write(binToBCD(days + 1)); // Day of month
  return Wire.endTransmission() == 0;
}

} // namespace

void DS3231RTC::begin() {
  Wire.begin();
}

bool DS3231RTC::countdown(uint16_t duration_minutes) {
  if (!disableAlarm()) return false;
  if (!resetTime()) return false;
  if (!setAlarmTime(duration_minutes)) return false;
  return enableAlarm();
}

