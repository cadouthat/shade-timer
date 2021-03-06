#ifndef __TEST_MOCK_ARDUINO_H
#define __TEST_MOCK_ARDUINO_H

#include <stdint.h>
#include <stdlib.h>

#include "gmock/gmock.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define bit(x) (1 << (x))
#define F(str) str
#define PROGMEM
#define FPSTR(str) String(str)
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

  size_t length() const {
    return strlen(c_str_);
  }
  const char* c_str() const {
    return c_str_;
  }
  bool equals(const char* str) const {
    return !strcmp(c_str_, str);
  }
  int toInt() const {
    return atoi(c_str_);
  }

 private:
  const char* c_str_;
};
typedef FakeString String;

class FakeSerial {
 public:
  void println(const char* str) {}
};
extern FakeSerial Serial;

#endif
