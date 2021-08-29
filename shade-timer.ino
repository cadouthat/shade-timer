#include <EEPROM.h>

#define CHAIN_PULL_PIN 14

constexpr uint64_t kOneHour_us = 60ull * 60 * 1000 * 1000;

void toggleShade() {
  digitalWrite(CHAIN_PULL_PIN, LOW);
  delay(500);
  digitalWrite(CHAIN_PULL_PIN, HIGH);
}

void blink(int n) {
  while (n-- > 0) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
}

void setup() {
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(CHAIN_PULL_PIN, HIGH);
  pinMode(CHAIN_PULL_PIN, OUTPUT_OPEN_DRAIN);

  toggleShade();

  EEPROM.begin(1);
  uint64_t duration_us;
  if (EEPROM.read(0) > 0) {
    duration_us = kOneHour_us * 9;
    EEPROM.write(0, 0);
    blink(1);
  } else {
    duration_us = kOneHour_us * 15;
    EEPROM.write(0, 1);
    blink(2);
  }
  while (!EEPROM.commit()) {
    blink(10);
  }

  // Adjust duration to offset execution time.
  ESP.deepSleep(duration_us - micros());
}

void loop() {}
