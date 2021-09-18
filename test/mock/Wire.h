#ifndef __TEST_MOCK_WIRE_H
#define __TEST_MOCK_WIRE_H

#include <stdint.h>
#include <deque>

class FakeWire {
 public:
  enum ErrorType : uint8_t {
    kNoError = 0,
    kBadAddress = 1,
    kNoData = 2,
  };

  uint8_t expected_address;
  std::deque<uint8_t> read_queue;
  std::deque<uint8_t> write_queue;

  FakeWire() { reset(); }
  void reset() {
    expected_address = 0;
    transmit_error_ = kNoError;
    read_queue.clear();
    write_queue.clear();
  }

  void begin() {}
  void beginTransmission(uint8_t address) {
    transmit_error_ = kNoError;
    if (address != expected_address) {
      transmit_error_ = kBadAddress;
    }
  }
  void write(uint8_t value) {
    write_queue.push_back(value);
  }
  ErrorType endTransmission() { return transmit_error_; }

  uint8_t requestFrom(uint8_t address, uint8_t bytes) {
    if (address != expected_address) {
      return kBadAddress;
    }
    if (bytes != read_queue.size()) {
      return kNoData;
    }
    return kNoError;
  }
  uint8_t read() {
    if (read_queue.empty()) {
      return 0;
    }
    uint8_t value = read_queue.front();
    read_queue.pop_front();
    return value;
  }

 private:
  ErrorType transmit_error_;
};

extern FakeWire Wire;

#endif
