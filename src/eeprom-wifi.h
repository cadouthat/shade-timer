#ifndef __EEPROM_WIFI_H
#define __EEPROM_WIFI_H

#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>

constexpr int kWiFiSSIDMax = 32;
constexpr int kWiFiPSKMax = 64;
constexpr int kWiFiConfigSize = (kWiFiSSIDMax + kWiFiPSKMax);

// Attempts to connect to WiFi from credentials stored in EEPROM.
bool connectToWiFi(
    EEPROMClass& = EEPROM, ESP8266WiFiClass& = WiFi, HardwareSerial& = Serial);

// Prompts for new WiFi credentials via serial and stores in EEPROM.
bool configureWiFi(EEPROMClass& = EEPROM, HardwareSerial& = Serial);

#endif
