#include "src/schedule-util.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

TEST(MinutesUntilTest, TargetInPast) {
  EXPECT_EQ(minutesUntil(119, 120), 24 * 60 - 1);
  EXPECT_EQ(minutesUntil(24 * 60 - 1, 0), 24 * 60 - 1);
}
TEST(MinutesUntilTest, TargetInFuture) {
  EXPECT_EQ(minutesUntil(120, 119), 1);
  EXPECT_EQ(minutesUntil(0, 24 * 60 - 1), 1);
}
TEST(MinutesUntilTest, TargetEqual) {
  EXPECT_EQ(minutesUntil(119, 119), 0);
}

TEST(NearestMinutesTest, TargetInPast) {
  EXPECT_EQ(nearestMinutes(119, 120), -1);
  EXPECT_EQ(nearestMinutes(24 * 60 - 1, 0), -1);
}
TEST(NearestMinutesTest, TargetInFuture) {
  EXPECT_EQ(nearestMinutes(120, 119), 1);
  EXPECT_EQ(nearestMinutes(0, 24 * 60 - 1), 1);
}
TEST(NearestMinutesTest, TargetEqual) {
  EXPECT_EQ(nearestMinutes(119, 119), 0);
}

TEST(ProcessScheduleTest, TriggersEventInPast) {
  int triggers = 0;
  uint16_t schedule[2] = {24 * 60 - 1, kScheduleNoEvent};
  EXPECT_EQ(
    processSchedule(
      schedule, /*size=*/2, /*now=*/0, /*delta=*/5,
      /*callback=*/[&]() { triggers++; }
    ), 5);
  EXPECT_EQ(triggers, 1);
}
TEST(ProcessScheduleTest, TriggersEventInFuture) {
  int triggers = 0;
  uint16_t schedule[2] = {1, kScheduleNoEvent};
  EXPECT_EQ(
    processSchedule(
      schedule, /*size=*/2, /*now=*/0, /*delta=*/5,
      /*callback=*/[&]() { triggers++; }
    ), 7);
  EXPECT_EQ(triggers, 1);
}
TEST(ProcessScheduleTest, TriggersMultipleEvents) {
  int triggers = 0;
  uint16_t schedule[2] = {1, 24 * 60 - 1};
  EXPECT_EQ(
    processSchedule(
      schedule, /*size=*/2, /*now=*/0, /*delta=*/5,
      /*callback=*/[&]() { triggers++; }
    ), 7);
  EXPECT_EQ(triggers, 2);
}
TEST(ProcessScheduleTest, ReturnsCurrentDayEventAfterTrigger) {
  int triggers = 0;
  uint16_t schedule[2] = {1, 23 * 60};
  EXPECT_EQ(
    processSchedule(
      schedule, /*size=*/2, /*now=*/0, /*delta=*/5,
      /*callback=*/[&]() { triggers++; }
    ), 23 * 60);
  EXPECT_EQ(triggers, 1);
}
TEST(ProcessScheduleTest, ReturnsNextDayEventAfterTrigger) {
  int triggers = 0;
  uint16_t schedule[2] = {12 * 60, 2 * 60};
  EXPECT_EQ(
    processSchedule(
      schedule, /*size=*/2, /*now=*/12 * 60, /*delta=*/5,
      /*callback=*/[&]() { triggers++; }
    ), 14 * 60);
  EXPECT_EQ(triggers, 1);
}
TEST(ProcessScheduleTest, ReturnsNearestCurrentDayEvent) {
  uint16_t schedule[4] = {0, 12 * 60, 2 * 60, 10 * 60};
  EXPECT_EQ(
    processSchedule(
      schedule, /*size=*/4, /*now=*/60, /*delta=*/5,
      /*callback=*/[]() {}
    ), 60);
}
TEST(ProcessScheduleTest, ReturnsNearestNextDayEvent) {
  uint16_t schedule[4] = {19 * 60, 12 * 60, 2 * 60, 10 * 60};
  EXPECT_EQ(
    processSchedule(
      schedule, /*size=*/4, /*now=*/20 * 60, /*delta=*/5,
      /*callback=*/[]() {}
    ), 6 * 60);
}
TEST(ProcessScheduleTest, ReturnsZeroForNoEvents) {
  int triggers = 0;
  uint16_t schedule[2] = {0};
  schedule[0] = kScheduleNoEvent;
  EXPECT_EQ(
    processSchedule(
      schedule, /*size=*/2, /*now=*/0, /*delta=*/5,
      /*callback=*/[&]() { triggers++; }
    ), 0);
  EXPECT_EQ(triggers, 0);
}

} // namespace