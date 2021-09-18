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
  name = "eeprom-wifi-test",
  srcs = ["eeprom-wifi-test.cpp"],
  deps = [
    "//src:eeprom-wifi",
    "//test/mock:EEPROM",
    "//test/mock:ESP8266WiFi",
    "@gmock-global//:gmock-global",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)
