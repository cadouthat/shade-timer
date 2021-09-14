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
constexpr uint8_t DS3231_STATUS = 0x0F;

constexpr uint8_t DS3231_ALARM1_FLAG = bit(0);
constexpr uint8_t DS3231_STATUS_DEFAULT = bit(3);

uint8_t binToBCD(uint8_t bin) {
  return (bin / 10 << 4) | (bin % 10);
}

uint8_t BCDToBin(uint8_t bcd) {
  return (bcd >> 4) * 10 + (bcd & 0b1111);
}

uint8_t read(uint8_t read_register) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(read_register);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 1);
  return Wire.read();
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

bool getAlarmFlag() {
  return (read(DS3231_STATUS) & DS3231_ALARM1_FLAG) > 0;
}

bool resetAlarm() {
  return write(DS3231_STATUS, DS3231_STATUS_DEFAULT);
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

uint16_t convertToMinutes(
    uint8_t day_of_month, uint8_t hour, uint8_t minute) {
  uint16_t minutes = day_of_month;
  minutes *= kHoursPerDay;
  minutes += hour;
  minutes *= kMinutesPerHour;
  minutes += minute;
  return minutes;
}

uint16_t getAlarmTime() {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_ALARM1);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 4);
  Wire.read(); // Ignore seconds
  uint8_t minute = BCDToBin(Wire.read());
  uint8_t hour = BCDToBin(Wire.read());
  uint8_t day_of_month = BCDToBin(Wire.read());
  return convertToMinutes(day_of_month, hour, minute);
}

uint16_t getTime() {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_TIME);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 5);
  Wire.read(); // Ignore seconds.
  uint8_t minute = BCDToBin(Wire.read());
  uint8_t hour = BCDToBin(Wire.read());
  Wire.read(); // Ignore day of week.
  uint8_t day_of_month = BCDToBin(Wire.read());
  return convertToMinutes(day_of_month, hour, minute);
}

} // namespace

void DS3231RTC::begin() {
  Wire.begin();
}

bool DS3231RTC::countdown(uint16_t duration_minutes) {
  if (!resetTime()) return false;
  if (!setAlarmTime(duration_minutes)) return false;
  return resetAlarm();
}

uint16_t DS3231RTC::remainingMinutes() {
  if (getAlarmFlag()) return 0;
  uint16_t time = getTime();
  uint16_t alarm = getAlarmTime();
  if (time >= alarm) return 0;
  return alarm - time;
}
