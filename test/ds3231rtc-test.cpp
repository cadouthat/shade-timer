#include "src/ds3231rtc.h"

#include "Wire.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

FakeWire Wire;

namespace {

using ::testing::ElementsAre;

class DS3231RTCTest : public testing::Test {
 public:
  DS3231RTCTest() {
    Wire.reset();
    Wire.expected_address = 0x68; // DS3231 I2C address.
    rtc.begin();
  }

 protected:
  DS3231RTC rtc;
};

TEST_F(DS3231RTCTest, ReturnsWireFailures) {
  EXPECT_FALSE(rtc.isTimeValid());
  EXPECT_FALSE(rtc.setMinuteOfDay(1));
  EXPECT_EQ(rtc.getMinuteOfDay(), 0);
  EXPECT_TRUE(rtc.isAlarmActive());
  EXPECT_FALSE(rtc.setMinuteOfDayAlarm(1));
  EXPECT_EQ(rtc.getMinuteOfDayAlarm(), 0);
}

TEST_F(DS3231RTCTest, ReturnsTimeValid) {
  Wire.read_queue.push_back(0);

  EXPECT_TRUE(rtc.isTimeValid());
  EXPECT_THAT(Wire.write_queue, ElementsAre(/*status reg*/0x0F));
}

TEST_F(DS3231RTCTest, ReturnsTimeInvalid) {
  Wire.read_queue.push_back(/*OSF flag*/0b10000000);

  EXPECT_FALSE(rtc.isTimeValid());
  EXPECT_THAT(Wire.write_queue, ElementsAre(/*status reg*/0x0F));
}

TEST_F(DS3231RTCTest, ReturnsAlarmInctive) {
  Wire.read_queue.push_back(0);

  EXPECT_FALSE(rtc.isAlarmActive());
  EXPECT_THAT(Wire.write_queue, ElementsAre(/*status reg*/0x0F));
}

TEST_F(DS3231RTCTest, ReturnsAlarmActive) {
  Wire.read_queue.push_back(/*Alarm1 flag*/0b1);

  EXPECT_TRUE(rtc.isAlarmActive());
  EXPECT_THAT(Wire.write_queue, ElementsAre(/*status reg*/0x0F));
}

TEST_F(DS3231RTCTest, GetsMinuteOfDay) {
  Wire.read_queue.push_back(/*seconds*/33);
  Wire.read_queue.push_back(/*minutes*/0b00010001);
  Wire.read_queue.push_back(/*hours*/0b00010001);

  EXPECT_EQ(rtc.getMinuteOfDay(), /*11:11*/11*60 + 11);
  EXPECT_THAT(Wire.write_queue, ElementsAre(/*time reg*/0x00));
}

TEST_F(DS3231RTCTest, SetsMinuteOfDay) {
  Wire.read_queue.push_back(/*Alarm1|OSF flags*/0b10000001);

  EXPECT_TRUE(rtc.setMinuteOfDay(/*11:11*/11*60 + 11));
  EXPECT_THAT(Wire.write_queue, ElementsAre(
    /*time reg*/0x00,
    /*seconds*/0,
    /*minutes*/0b00010001,
    /*hours*/0b00010001,
    /*weekday*/1,
    /*day of month*/1,
    /*month*/1,
    /*year*/0,
    /*status reg*/0x0F,
    /*status reg*/0x0F,
    /*Alarm1 flag*/0b1));
}

TEST_F(DS3231RTCTest, SetsMinuteOfDayWithWrap) {
  Wire.read_queue.push_back(/*Alarm1|OSF flags*/0b10000001);

  EXPECT_TRUE(rtc.setMinuteOfDay(24*60 + /*11:11*/11*60 + 11));
  EXPECT_THAT(Wire.write_queue, ElementsAre(
    /*time reg*/0x00,
    /*seconds*/0,
    /*minutes*/0b00010001,
    /*hours*/0b00010001,
    /*weekday*/1,
    /*day of month*/1,
    /*month*/1,
    /*year*/0,
    /*status reg*/0x0F,
    /*status reg*/0x0F,
    /*Alarm1 flag*/0b1));
}

TEST_F(DS3231RTCTest, GetsMinuteOfDayAlarm) {
  Wire.read_queue.push_back(/*seconds*/33);
  Wire.read_queue.push_back(/*minutes*/0b00010001);
  Wire.read_queue.push_back(/*hours*/0b00010001);

  EXPECT_EQ(rtc.getMinuteOfDayAlarm(), /*11:11*/11*60 + 11);
  EXPECT_THAT(Wire.write_queue, ElementsAre(/*Alarm1 reg*/0x07));
}

TEST_F(DS3231RTCTest, SetsMinuteOfDayAlarm) {
  Wire.read_queue.push_back(/*Alarm1|OSF flags*/0b10000001);

  EXPECT_TRUE(rtc.setMinuteOfDayAlarm(/*11:11*/11*60 + 11));
  EXPECT_THAT(Wire.write_queue, ElementsAre(
    /*alarm1 reg*/0x07,
    /*seconds*/0,
    /*minutes*/0b00010001,
    /*hours*/0b00010001,
    /*day of month*/0b10000000,
    /*status reg*/0x0F,
    /*status reg*/0x0F,
    /*OSF flag*/0b10000000));
}

TEST_F(DS3231RTCTest, SetsMinuteOfDayAlarmWithWrap) {
  Wire.read_queue.push_back(/*Alarm1|OSF flags*/0b10000001);

  EXPECT_TRUE(rtc.setMinuteOfDayAlarm(24*60 + /*11:11*/11*60 + 11));
  EXPECT_THAT(Wire.write_queue, ElementsAre(
    /*alarm1 reg*/0x07,
    /*seconds*/0,
    /*minutes*/0b00010001,
    /*hours*/0b00010001,
    /*day of month*/0b10000000,
    /*status reg*/0x0F,
    /*status reg*/0x0F,
    /*OSF flag*/0b10000000));
}

} // namespace
