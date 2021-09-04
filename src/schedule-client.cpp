#include "schedule-client.h"

#include <string.h>
#include <stdio.h>

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

namespace {
constexpr PROGMEM char kUrl[] = "http://cadouthat.duckdns.org/shade-timer.txt";
constexpr int kMinutesPerHour = 60;
} // namespace

bool fetchSchedule(uint16_t* output_minutes, int size) {
  WiFiClient wifi;
  HTTPClient http;
  if (!http.begin(wifi, FPSTR(kUrl))) {
    Serial.println(F("fetchSchedule connection failed"));
    return false;
  }
  if (http.GET() != HTTP_CODE_OK) {
    http.end();
    Serial.println(F("fetchSchedule got bad HTTP status"));
    return false;
  }
  String payload = http.getString();
  http.end();

  const char* scan = payload.c_str();
  // Stores one line in HH:MM format, plus null-termination.
  char line[6] = {0};
  int count = 0;
  while (*scan && count < size) {
    // Exact line length is expected.
    strncpy(line, scan, sizeof(line) - 1);
    uint8_t hour, minute;
    if (sscanf(line, "%2hhu:%2hhu", &hour, &minute) < 2) {
      Serial.println(F("fetchSchedule got bad payload"));
      return false;
    }
    output_minutes[count++] = hour * kMinutesPerHour + minute;
    scan += strlen(line);
    if (*scan == '\n') scan++;
  }
  if (count < size) {
    // Special value to indicate early end of schedule.
    output_minutes[count] = kScheduleNoEvent;
  }
  return true;
}
