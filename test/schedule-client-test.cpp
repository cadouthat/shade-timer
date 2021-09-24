#include "src/schedule-client.h"

#include "Arduino.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/schedule-util.h"

FakeSerial Serial;

namespace {

using ::testing::_;
using ::testing::ElementsAre;
using ::testing::Invoke;
using ::testing::Return;

TEST(ScheduleClientTest, FetchesValidSchedule) {
  HTTPClient http;
  EXPECT_CALL(http, begin(_, _))
    .WillOnce(Invoke([](WiFiClient& wifi, const String& url) {
      EXPECT_TRUE(url.equals("http://cadouthat.duckdns.org/shade-timer.txt"));
      return true;
  }));
  EXPECT_CALL(http, GET()).WillOnce(Return(HTTP_CODE_OK));
  EXPECT_CALL(http, getString()).WillOnce(Return(String("00:00\n23:59\n")));

  uint16_t schedule[2];
  EXPECT_TRUE(
    fetchSchedule(http, schedule, sizeof(schedule) / sizeof(uint16_t)));
  EXPECT_THAT(schedule, ElementsAre(0, 1439));
}

TEST(ScheduleClientTest, FetchesTruncatedSchedule) {
  HTTPClient http;
  EXPECT_CALL(http, begin(_, _))
    .WillOnce(Invoke([](WiFiClient& wifi, const String& url) {
      EXPECT_TRUE(url.equals("http://cadouthat.duckdns.org/shade-timer.txt"));
      return true;
  }));
  EXPECT_CALL(http, GET()).WillOnce(Return(HTTP_CODE_OK));
  EXPECT_CALL(http, getString()).WillOnce(Return(String("23:59")));

  uint16_t schedule[2];
  EXPECT_TRUE(
    fetchSchedule(http, schedule, sizeof(schedule) / sizeof(uint16_t)));
  EXPECT_THAT(schedule, ElementsAre(1439, kScheduleNoEvent));
}

TEST(ScheduleClientTest, FailsOnConnectionError) {
  uint16_t schedule[2];
  EXPECT_FALSE(
    fetchSchedule(schedule, sizeof(schedule) / sizeof(uint16_t)));
}

TEST(ScheduleClientTest, FailsOnHTTPError) {
  HTTPClient http;
  EXPECT_CALL(http, begin(_, _)).WillOnce(Return(true));
  EXPECT_CALL(http, GET()).WillOnce(Return(0));

  uint16_t schedule[2];
  EXPECT_FALSE(
    fetchSchedule(http, schedule, sizeof(schedule) / sizeof(uint16_t)));
}

TEST(ScheduleClientTest, FailsOnParseError) {
  HTTPClient http;
  EXPECT_CALL(http, begin(_, _))
    .WillOnce(Invoke([](WiFiClient& wifi, const String& url) {
      EXPECT_TRUE(url.equals("http://cadouthat.duckdns.org/shade-timer.txt"));
      return true;
  }));
  EXPECT_CALL(http, GET()).WillOnce(Return(HTTP_CODE_OK));
  EXPECT_CALL(http, getString()).WillOnce(Return(String("00:00a\n23:59\n")));

  uint16_t schedule[2];
  EXPECT_FALSE(
    fetchSchedule(http, schedule, sizeof(schedule) / sizeof(uint16_t)));
}

TEST(ScheduleClientTest, FailsOnOverflow) {
  HTTPClient http;
  EXPECT_CALL(http, begin(_, _))
    .WillOnce(Invoke([](WiFiClient& wifi, const String& url) {
      EXPECT_TRUE(url.equals("http://cadouthat.duckdns.org/shade-timer.txt"));
      return true;
  }));
  EXPECT_CALL(http, GET()).WillOnce(Return(HTTP_CODE_OK));
  EXPECT_CALL(http, getString()).WillOnce(Return(String("00:00\n23:59\n")));

  uint16_t schedule[2];
  EXPECT_FALSE(
    fetchSchedule(http, schedule, 1));
}

} // namespace
