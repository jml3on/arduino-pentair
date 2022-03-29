#ifdef UNITTEST

#include "packet_test.h"

class SystemStatusTest : public PacketTest {
 protected:
  SystemStatus &actual = p_.system_status;
  void P(const char *data) {
    String preamble("ff 00 ff ");
    String header("a5 25 0f 10 02 1d ");
    String checksum = CheckSum(header + data);
    String packet = preamble + header + data + " " + checksum;
    EXPECT_TRUE(this->ReadPacket(packet.c_str()));
    EXPECT_EQ(true, p_.IsValid());
  }
};

TEST_F(SystemStatusTest, Header) {
  P("0a 0e 00 00 00 00 00 00 00 00 03 00 20 04 4b 62 20 00 4c 5c 00 00 07 00 00 57 80 00 0d");

  EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.source_);
  EXPECT_EQ(DEVICE_BROADCAST, p_.target_);
  EXPECT_EQ(COMMAND_SYSTEM_STATUS, p_.cid_);
}

TEST_F(SystemStatusTest, Values) {
  P("0a 0e 00 00 00 00 00 00 00 00 03 00 20 04 4b 62 20 00 4c 5c 00 00 07 00 00 57 80 00 0d");

  EXPECT_EQ(76, actual.air_temp_);
  EXPECT_EQ(false, actual.celcius_);
  EXPECT_FALSE(actual.circuits_[5]);
  EXPECT_EQ(10, actual.hour_);
  EXPECT_EQ(14, actual.minutes_);
  EXPECT_EQ(SYSTEM_MODE_AUTO, actual.mode_);
  EXPECT_EQ(HEAT_SOURCE_SOLAR, actual.pool_heat_source_);
  EXPECT_EQ(75, actual.pool_temp_);
  EXPECT_FALSE(actual.solar_);
  EXPECT_EQ(92, actual.solar_temp_);
  EXPECT_EQ(HEAT_SOURCE_HEATER, actual.spa_heat_source_);
  EXPECT_EQ(98, actual.spa_temp_);
}

TEST_F(SystemStatusTest, Mode) {
  {
    SetUp();
    P("0a 0e 00 00 00 00 00 00 00 00 03 00 20 04 4b 62 20 00 4c 5c 00 00 07 00 00 57 80 00 0d");
    EXPECT_FALSE(actual.celcius_);
    EXPECT_EQ(SYSTEM_MODE_AUTO, actual.mode_);
  }
  {
    SetUp();
    P("0a 0e 00 00 00 00 00 00 00 01 03 00 20 04 4b 62 20 00 4c 5c 00 00 07 00 00 57 80 00 0d");

    EXPECT_FALSE(actual.celcius_);
    EXPECT_EQ(SYSTEM_MODE_SERVICE, actual.mode_);
  }
  {
    SetUp();
    P("0a 0e 00 00 00 00 00 00 00 85 03 00 20 04 4b 62 20 00 4c 5c 00 00 07 00 00 57 80 00 0d");

    EXPECT_TRUE(actual.celcius_);
    EXPECT_EQ(SYSTEM_MODE_TIMEOUT, actual.mode_);
  }
}

TEST_F(SystemStatusTest, HeatSettings) {
  {
    SetUp();
    P("0a 0e 00 00 00 00 00 00 00 00 03 00 20 04 4b 62 20 00 4c 5c 00 00 00 00 00 57 80 00 0d");

    EXPECT_EQ(HEAT_SOURCE_OFF, actual.pool_heat_source_);
    EXPECT_EQ(HEAT_SOURCE_OFF, actual.spa_heat_source_);
  }
  {
    SetUp();
    P("0a 0e 00 00 00 00 00 00 00 00 03 00 20 04 4b 62 20 00 4c 5c 00 00 05 00 00 57 80 00 0d");

    EXPECT_EQ(HEAT_SOURCE_HEATER, actual.pool_heat_source_);
    EXPECT_EQ(HEAT_SOURCE_HEATER, actual.spa_heat_source_);
  }
  {
    SetUp();
    P("0a 0e 00 00 00 00 00 00 00 00 03 00 20 04 4b 62 20 00 4c 5c 00 00 0a 00 00 57 80 00 0d");

    EXPECT_EQ(HEAT_SOURCE_SOLAR_PREF, actual.pool_heat_source_);
    EXPECT_EQ(HEAT_SOURCE_SOLAR_PREF, actual.spa_heat_source_);
  }
  {
    SetUp();
    P("0a 0e 00 00 00 00 00 00 00 00 03 00 20 04 4b 62 20 00 4c 5c 00 00 0f 00 00 57 80 00 0d");

    EXPECT_EQ(HEAT_SOURCE_SOLAR, actual.pool_heat_source_);
    EXPECT_EQ(HEAT_SOURCE_SOLAR, actual.spa_heat_source_);
  }

}

TEST_F(SystemStatusTest, Circuits) {
  {
    SetUp();
    P("0a 0e 00 00 00 00 00 00 00 00 03 00 20 04 4b 62 20 00 4c 5c 00 00 07 00 00 57 80 00 0d");

    for (int i = 0; i < 8; i++) {
      EXPECT_FALSE(actual.circuits_[i]);
    }
  }
  {
    SetUp();
    P("0a 0e 21 00 00 00 00 00 00 00 03 00 20 04 4b 62 20 00 4c 5c 00 00 07 00 00 57 80 00 0d");

    EXPECT_TRUE(actual.circuits_[0]);
    EXPECT_TRUE(actual.circuits_[5]);
  }
}

TEST_F(SystemStatusTest, Solar_Off) {
  P("0a 0e 21 00 00 00 00 00 00 00 03 00 20 04 4b 62 20 00 4c 5c 00 00 07 00 00 57 80 00 0d");
  EXPECT_FALSE(actual.solar_);
}

TEST_F(SystemStatusTest, Solar_On) {
  P("0a 0e 21 00 00 00 00 00 00 00 33 00 20 04 4b 62 20 00 4c 5c 00 00 07 00 00 57 80 00 0d");
  EXPECT_TRUE(actual.solar_);
}

#endif // UNITTEST

