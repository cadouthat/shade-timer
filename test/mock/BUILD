package(default_visibility = ["//visibility:public"])

cc_library(
  name = "Arduino",
  testonly = True,
  hdrs = ["//test/mock:Arduino.h"],
  strip_include_prefix = "",
  deps = [
    "@gtest//:gtest",
  ],
)
cc_library(
  name = "EEPROM",
  testonly = True,
  hdrs = ["//test/mock:EEPROM.h"],
  strip_include_prefix = "",
  deps = [
    "@gtest//:gtest",
  ],
)
cc_library(
  name = "ESP8266HTTPClient",
  testonly = True,
  hdrs = ["//test/mock:ESP8266HTTPClient.h"],
  strip_include_prefix = "",
  deps = [
    ":Arduino",
    ":WiFiClient",
    "@gtest//:gtest",
  ],
)
cc_library(
  name = "ESP8266WiFi",
  testonly = True,
  hdrs = ["//test/mock:ESP8266WiFi.h"],
  strip_include_prefix = "",
  deps = [
    "@gtest//:gtest",
  ],
)
cc_library(
  name = "Wire",
  testonly = True,
  hdrs = ["//test/mock:Wire.h"],
  strip_include_prefix = "",
)
cc_library(
  name = "WiFiClient",
  testonly = True,
  hdrs = ["//test/mock:WiFiClient.h"],
  strip_include_prefix = "",
)
