#ifndef __TEST_MOCK_ESP8266HTTPCLIENT_H
#define __TEST_MOCK_ESP8266HTTPCLIENT_H

#include "Arduino.h"
#include "WiFiClient.h"
#include "gmock/gmock.h"

#define HTTP_CODE_OK 200

class MockESP8266HTTPClient {
 public:
  MOCK_METHOD(bool, begin, (WiFiClient&, const String&));
  MOCK_METHOD(int, GET, ());
  MOCK_METHOD(String, getString, ());
  MOCK_METHOD(void, end, ());
};

typedef MockESP8266HTTPClient HTTPClient;

#endif
