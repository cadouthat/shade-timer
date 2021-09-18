#ifndef __TESTING_ARDUINO_H
#define __TESTING_ARDUINO_H

#include <stdint.h>
#include <stdlib.h>

#include "gmock/gmock.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define bit(x) (1 << (x))
#define F(str) str
#define LED_BUILTIN 99
#define LOW 88
#define HIGH 77

void analogWrite(int pin, int val);
void digitalWrite(int pin, int val);

void delayMicroseconds(uint64_t us);
void delay(uint64_t ms);
uint64_t millis();

class FakeString {
 public:
  FakeString() : c_str_("") {}
  FakeString(const char* c_str) : c_str_(c_str) {}

  size_t length() {
    return strlen(c_str_);
  }
  const char* c_str() {
    return c_str_;
  }
  bool equals(const char* str) {
    return !strcmp(c_str_, str);
  }
  int toInt() {
    return atoi(c_str_);
  }

 private:
  const char* c_str_;
};
typedef FakeString String;

class MockSerial {
 public:
  MOCK_METHOD1(print, void(const char* str));
  MOCK_METHOD1(print, void(String str));
  MOCK_METHOD1(println, void(const char* str));
  MOCK_METHOD1(setTimeout, void(unsigned int ms));
  MOCK_METHOD0(readString, String());
};
typedef MockSerial HardwareSerial;
extern HardwareSerial Serial;

#endif
