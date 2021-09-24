# shade-timer
Scheduled window shade controlling device which synchonizes open/close schedule
and time (NTP) over WiFi. Conserves battery power by entering deep sleep as much
as possible, with an external RTC for accurate wake timing.

# Hardware
* ESP8266 MCU
* DS3231 RTC
* Roller shade capable of opening/closing with a single toggle

# Environment
* Arduino IDE for target device build/upload
* Bazel for running unit tests

# Related Links
* [ESP8266 Arduino core](https://github.com/esp8266/Arduino)
* [NTPClient library](https://github.com/arduino-libraries/NTPClient)
