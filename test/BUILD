cc_test(
  name = "ds3231rtc-test",
  srcs = ["ds3231rtc-test.cpp"],
  deps = [
    "//src:ds3231rtc",
    "//test/mock:Wire",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)
cc_test(
  name = "schedule-client-test",
  srcs = ["schedule-client-test.cpp"],
  deps = [
    "//src:schedule-client",
    "//src:schedule-util",
    "//test/mock:Arduino",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)
cc_test(
  name = "schedule-util-test",
  srcs = ["schedule-util-test.cpp"],
  deps = [
    "//src:schedule-util",
    "//test/mock:Arduino",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)
