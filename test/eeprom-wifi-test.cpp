#include "src/eeprom-wifi.h"

#include <string.h>

#include "Arduino.h"
#include "EEPROM.h"
#include "ESP8266WiFi.h"
#include "gmock/gmock.h"
#include "gmock-global/gmock-global.h"
#include "gtest/gtest.h"

MOCK_GLOBAL_FUNC1(delay, void(uint64_t));
MOCK_GLOBAL_FUNC0(millis, uint64_t());

namespace {

using ::testing::_;
using ::testing::Invoke;
using ::testing::Mock;
using ::testing::Return;
using ::testing::StrEq;

constexpr char* kSSID = "ssid";
constexpr char* kPSK = "psk";

class EEPROMWiFiTest : public testing::Test {
 protected:
  uint64_t clock;
  FakeEEPROM EEPROM;
  MockESP8266WiFi WiFi;
  MockSerial Serial;

  EEPROMWiFiTest() : clock(0) {
    EEPROM.begin(kWiFiConfigSize);
  }

  bool registerMocksAndConnect() {
    ON_GLOBAL_CALL(millis, millis()).WillByDefault(Invoke([&]() {
      return clock;
    }));
    ON_GLOBAL_CALL(delay, delay(_))
        .WillByDefault(Invoke([&](uint64_t duration) {
          clock += duration;
        }));

    return connectToWiFi(EEPROM, WiFi, Serial);
  }
};

TEST_F(EEPROMWiFiTest, ConnectFailsWithoutConfig) {
  EXPECT_FALSE(registerMocksAndConnect());
  EXPECT_EQ(clock, 0);
}

TEST_F(EEPROMWiFiTest, ConnectTimesOut) {
  memcpy(&EEPROM[0], kSSID, strlen(kSSID));
  memcpy(&EEPROM[kWiFiSSIDMax], kPSK, strlen(kPSK));
  EXPECT_CALL(WiFi, begin(StrEq(kSSID), StrEq(kPSK)));
  EXPECT_CALL(WiFi, status())
      .WillRepeatedly(Return(WL_PENDING));

  EXPECT_FALSE(registerMocksAndConnect());
  EXPECT_EQ(clock, 30500);
}

TEST_F(EEPROMWiFiTest, ConnectSucceeds) {
  memcpy(&EEPROM[0], kSSID, strlen(kSSID));
  memcpy(&EEPROM[kWiFiSSIDMax], kPSK, strlen(kPSK));
  EXPECT_CALL(WiFi, begin(StrEq(kSSID), StrEq(kPSK)));
  EXPECT_CALL(WiFi, status())
      .WillOnce(Return(WL_CONNECTED));

  EXPECT_TRUE(registerMocksAndConnect());
  EXPECT_EQ(clock, 0);
}

TEST_F(EEPROMWiFiTest, ConnectSucceedsWithLongValues) {
    // 32 characters (max SSID length).
  const char* ssid = "01234567890123456789012345678901";
    // 64 characters (max PSK length).
  const char* psk = "0123456789012345678901234567890123456789012345678901234567"
                    "890123";
  memcpy(&EEPROM[0], ssid, strlen(ssid));
  memcpy(&EEPROM[kWiFiSSIDMax], psk, strlen(psk));
  EXPECT_CALL(WiFi, begin(StrEq(ssid), StrEq(psk)));
  EXPECT_CALL(WiFi, status())
      .WillOnce(Return(WL_PENDING))
      .WillOnce(Return(WL_CONNECTED));

  EXPECT_TRUE(registerMocksAndConnect());
  EXPECT_EQ(clock, 500);
}

TEST_F(EEPROMWiFiTest, ConfigureFailsOnEmptySSID) {
  EXPECT_CALL(Serial, readString()).WillOnce(Return("\n"));

  EXPECT_FALSE(configureWiFi(EEPROM, Serial));
}

TEST_F(EEPROMWiFiTest, ConfigureFailsOnSSIDOverflow) {
  // 33 characters (max SSID length + 1), excluding newline.
  const char* ssid = "012345678901234567890123456789012\n";
  EXPECT_CALL(Serial, readString()).WillOnce(Return(ssid));

  EXPECT_FALSE(configureWiFi(EEPROM, Serial));
}

TEST_F(EEPROMWiFiTest, ConfigureFailsOnEmptyPSK) {
  EXPECT_CALL(Serial, readString())
      .WillOnce(Return(kSSID))
      .WillOnce(Return(""));

  EXPECT_FALSE(configureWiFi(EEPROM, Serial));
}

TEST_F(EEPROMWiFiTest, ConfigureFailsOnPSKOverflow) {
  // 65 characters (max PSK length + 1), excluding newline.
  const char* psk = "0123456789012345678901234567890123456789012345678901234567"
                    "8901234\n";
  EXPECT_CALL(Serial, readString())
      .WillOnce(Return(kSSID))
      .WillOnce(Return(psk));

  EXPECT_FALSE(configureWiFi(EEPROM, Serial));
}

TEST_F(EEPROMWiFiTest, ConfigureSetsSerialTimeouts) {
  EXPECT_CALL(Serial, setTimeout(10000));
  EXPECT_CALL(Serial, setTimeout(30000));
  EXPECT_CALL(Serial, readString())
      .WillOnce(Return(kSSID))
      .WillOnce(Return(""));

  EXPECT_FALSE(configureWiFi(EEPROM, Serial));
}

TEST_F(EEPROMWiFiTest, ConfigureFailsOnCommitFailure) {
  EXPECT_CALL(Serial, readString())
      .WillOnce(Return(kSSID))
      .WillOnce(Return(kPSK));
  EXPECT_CALL(EEPROM, commit()).WillOnce(Return(false));

  EXPECT_FALSE(configureWiFi(EEPROM, Serial));
}

TEST_F(EEPROMWiFiTest, ConfigureSucceeds) {
  std::string ssid = kSSID;
  ssid += "\n";
  std::string psk = kPSK;
  psk += "\n";
  EXPECT_CALL(Serial, setTimeout(10000));
  EXPECT_CALL(Serial, setTimeout(30000));
  EXPECT_CALL(Serial, readString())
      .WillOnce(Return(ssid.c_str()))
      .WillOnce(Return(psk.c_str()));
  EXPECT_CALL(EEPROM, commit()).WillOnce(Return(true));

  EXPECT_TRUE(configureWiFi(EEPROM, Serial));
  EXPECT_EQ(strcmp((char*)&EEPROM[0], kSSID), 0);
  EXPECT_EQ(strcmp((char*)&EEPROM[kWiFiSSIDMax], kPSK), 0);
}

TEST_F(EEPROMWiFiTest, ConfigureSucceedsWithMaxLength) {
    // 32 characters (max SSID length).
  std::string ssid = "01234567890123456789012345678901";
    // 64 characters (max PSK length).
  std::string psk = "0123456789012345678901234567890123456789012345678901234567"
                    "890123";
  std::string combined = ssid + psk;
  ssid += "\n";
  psk += "\n";
  EXPECT_CALL(Serial, readString())
      .WillOnce(Return(ssid.c_str()))
      .WillOnce(Return(psk.c_str()));
  EXPECT_CALL(EEPROM, commit()).WillOnce(Return(true));

  EXPECT_TRUE(configureWiFi(EEPROM, Serial));
  EXPECT_EQ(memcmp(&EEPROM[0], combined.c_str(), combined.length()), 0);
}

} // namespace
