/*
 *      Author: jm
 */
#ifdef UNITTEST

#include "packet_test.h"

class ScheduleConfigTest : public PacketTest {
 protected:
  ScheduleConfig &actual = p_.schedule_config;
  void P(const char *data) {
    String preamble("ff 00 ff ");
    String header("a5 25 0f 10 11 07 ");
    String checksum = CheckSum(header + data);
    String packet = preamble + header + data + " " + checksum;
    this->ReadPacket(packet.c_str());
    EXPECT_TRUE(this->ReadPacket(packet.c_str()));
    EXPECT_EQ(true, p_.IsValid());
  }
};

TEST_F(ScheduleConfigTest, Header) {
  P("01 06 0a 0f 14 0f 7e");

  EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.source_);
  EXPECT_EQ(DEVICE_BROADCAST, p_.target_);
  EXPECT_EQ(COMMAND_SCHEDULE_CONFIG, p_.cid_);
}

TEST_F(ScheduleConfigTest, Broadcast) {
  P("01 06 0a 0f 14 0f 7e");

  EXPECT_EQ(1, actual.circuit_);
  EXPECT_EQ(10, actual.start_hour_);
  EXPECT_EQ(15, actual.start_minute_);
  EXPECT_EQ(20, actual.stop_hour_);
  EXPECT_EQ(15, actual.stop_minute_);
  EXPECT_EQ(0x7e, actual.active_days_);
}

#endif // UNITTEST

