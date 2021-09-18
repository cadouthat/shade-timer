#ifndef __TEST_MOCK_ESP8266WIFI_H
#define __TEST_MOCK_ESP8266WIFI_H

#include "gmock/gmock.h"

#define WL_PENDING 111
#define WL_CONNECTED 123

class MockESP8266WiFi {
 public:
  MOCK_METHOD2(begin, void(const char* ssid, const char* psk));
  MOCK_METHOD0(status, int());
  MOCK_METHOD0(localIP, const char*());
  MOCK_METHOD0(subnetMask, const char*());
  MOCK_METHOD0(gatewayIP, const char*());
};

typedef MockESP8266WiFi ESP8266WiFiClass;
extern ESP8266WiFiClass WiFi;

#endif
