#include "ds3231rtc.h"

#include <stdint.h>

#include <Arduino.h>
#include <Wire.h>

namespace {

constexpr int kMinutesPerHour = 60;
constexpr int kMinutesPerDay = kMinutesPerHour * 24;

constexpr uint8_t DS3231_ADDRESS = 0x68;
constexpr uint8_t DS3231_TIME = 0x00;
constexpr uint8_t DS3231_ALARM1 = 0x07;
constexpr uint8_t DS3231_STATUS = 0x0F;

constexpr uint8_t DS3231_ALARM_MASK = bit(7);
constexpr uint8_t DS3231_OSF_FLAG = bit(7);
constexpr uint8_t DS3231_ALARM1_FLAG = bit(0);

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

bool getStatusFlag(uint8_t flag) {
  return read(DS3231_STATUS) & flag;
}

bool clearStatusFlag(uint8_t flag) {
  uint8_t status = read(DS3231_STATUS);
  return write(DS3231_STATUS, status & ~flag);
}

int minuteOfDay(int hour, int minute) {
  return hour * kMinutesPerHour + minute;
}

} // namespace

void DS3231RTC::begin() {
  Wire.begin();
}

bool DS3231RTC::isTimeValid() {
  return !getStatusFlag(DS3231_OSF_FLAG);
}
int DS3231RTC::getMinuteOfDay() {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_TIME);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 3);
  Wire.read(); // Ignore seconds.
  uint8_t minute = BCDToBin(Wire.read());
  uint8_t hour = BCDToBin(Wire.read());
  return minuteOfDay(hour, minute);
}
bool DS3231RTC::setMinuteOfDay(int minute_of_day) {
  minute_of_day = minute_of_day % kMinutesPerDay;
  uint8_t minute = minute_of_day % kMinutesPerHour;
  uint8_t hour = minute_of_day / kMinutesPerHour;
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_TIME);
  Wire.write(0); // Second
  Wire.write(binToBCD(minute)); // Minute
  Wire.write(binToBCD(hour)); // Hour
  Wire.write(1); // Day of week
  Wire.write(1); // Day of month
  Wire.write(1); // Month/century
  Wire.write(0); // Year
  if (Wire.endTransmission() != 0) {
    return false;
  }
  return clearStatusFlag(DS3231_OSF_FLAG);
}

bool DS3231RTC::isAlarmActive() {
  return getStatusFlag(DS3231_ALARM1_FLAG);
}
int DS3231RTC::getMinuteOfDayAlarm() {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_ALARM1);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 3);
  Wire.read(); // Ignore seconds.
  uint8_t minute = BCDToBin(Wire.read());
  uint8_t hour = BCDToBin(Wire.read());
  return minuteOfDay(hour, minute);
}
bool DS3231RTC::setMinuteOfDayAlarm(int minute_of_day) {
  minute_of_day = minute_of_day % kMinutesPerDay;
  uint8_t minute = minute_of_day % kMinutesPerHour;
  uint8_t hour = minute_of_day / kMinutesPerHour;
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_ALARM1);
  Wire.write(0); // Second
  Wire.write(binToBCD(minute)); // Minute
  Wire.write(binToBCD(hour)); // Hour
  Wire.write(DS3231_ALARM_MASK); // Day of month (ignored)
  if (Wire.endTransmission() != 0) {
    return false;
  }
  return clearStatusFlag(DS3231_ALARM1);
}
