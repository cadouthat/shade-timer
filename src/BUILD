package(default_visibility = ["//visibility:public"])

cc_library(
  name = "ds3231rtc",
  testonly = True,
  hdrs = ["ds3231rtc.h"],
  srcs = ["ds3231rtc.cpp"],
  deps = [
    "//test/mock:Arduino",
    "//test/mock:Wire",
  ],
)
cc_library(
  name = "eeprom-wifi",
  testonly = True,
  srcs = ["eeprom-wifi.cpp"],
  hdrs = ["eeprom-wifi.h"],
  deps = [
    "//test/mock:Arduino",
    "//test/mock:EEPROM",
    "//test/mock:ESP8266WiFi",
  ],
)
cc_library(
  name = "schedule-client",
  testonly = True,
  srcs = ["schedule-client.cpp"],
  hdrs = ["schedule-client.h"],
  deps = [
    "//test/mock:Arduino",
    "//test/mock:ESP8266HTTPClient",
    "//test/mock:WiFiClient",
  ],
)
