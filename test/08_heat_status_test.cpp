/*
 *      Author: jm
 */
#ifdef UNITTEST

#include "packet_test.h"

class HeatStatusTest : public PacketTest {
 protected:
  HeatStatus &actual = p_.heat_status;
  void P(const char *data) {
    String preamble("ff 00 ff ");
    String header("a5 25 0f 10 08 0d ");
    String checksum = CheckSum(header + data);
    String packet = preamble + header + data + " " + checksum;
    EXPECT_TRUE(this->ReadPacket(packet.c_str()));
    EXPECT_EQ(true, p_.IsValid());
  }
};

TEST_F(HeatStatusTest, Header) {
  P("45 46 40 58 62 00 00 00 4c 64 00 00 00");

  EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.source_);
  EXPECT_EQ(DEVICE_BROADCAST, p_.target_);
  EXPECT_EQ(COMMAND_HEAT_STATUS, p_.cid_);
}

TEST_F(HeatStatusTest, Off) {
  P("45 46 40 58 62 00 00 00 4c 64 00 00 00");

  ASSERT_EQ(69, actual.pool_temp_);
  ASSERT_EQ(70, actual.spa_temp_);
  ASSERT_EQ(64, actual.air_temp_);
  ASSERT_EQ(76, actual.solar_temp_);
  ASSERT_EQ(88, actual.pool_temp_target_);
  ASSERT_EQ(HEAT_SOURCE_OFF, actual.pool_heat_source_);
  ASSERT_EQ(98, actual.spa_temp_target_);
  ASSERT_EQ(HEAT_SOURCE_OFF, actual.spa_heat_source_);
}

TEST_F(HeatStatusTest, Heater) {
  P("45 46 40 58 62 05 00 00 4c 64 00 00 00");

  ASSERT_EQ(69, actual.pool_temp_);
  ASSERT_EQ(70, actual.spa_temp_);
  ASSERT_EQ(64, actual.air_temp_);
  ASSERT_EQ(76, actual.solar_temp_);
  ASSERT_EQ(88, actual.pool_temp_target_);
  ASSERT_EQ(88, actual.pool_temp_target_);
  ASSERT_EQ(HEAT_SOURCE_HEATER, actual.pool_heat_source_);
  ASSERT_EQ(98, actual.spa_temp_target_);
  ASSERT_EQ(HEAT_SOURCE_HEATER, actual.spa_heat_source_);
}

TEST_F(HeatStatusTest, SolarPref) {
  P("45 46 40 58 62 0a 00 00 4c 64 00 00 00");

  ASSERT_EQ(69, actual.pool_temp_);
  ASSERT_EQ(70, actual.spa_temp_);
  ASSERT_EQ(64, actual.air_temp_);
  ASSERT_EQ(76, actual.solar_temp_);
  ASSERT_EQ(88, actual.pool_temp_target_);
  ASSERT_EQ(88, actual.pool_temp_target_);
  ASSERT_EQ(HEAT_SOURCE_SOLAR_PREF, actual.pool_heat_source_);
  ASSERT_EQ(98, actual.spa_temp_target_);
  ASSERT_EQ(HEAT_SOURCE_SOLAR_PREF, actual.spa_heat_source_);
}

TEST_F(HeatStatusTest, Solar) {
  P("45 46 40 58 62 0f 00 00 4c 64 00 00 00");

  ASSERT_EQ(69, actual.pool_temp_);
  ASSERT_EQ(70, actual.spa_temp_);
  ASSERT_EQ(64, actual.air_temp_);
  ASSERT_EQ(76, actual.solar_temp_);
  ASSERT_EQ(88, actual.pool_temp_target_);
  ASSERT_EQ(88, actual.pool_temp_target_);
  ASSERT_EQ(HEAT_SOURCE_SOLAR, actual.pool_heat_source_);
  ASSERT_EQ(98, actual.spa_temp_target_);
  ASSERT_EQ(HEAT_SOURCE_SOLAR, actual.spa_heat_source_);
}

#endif // UNITTEST

