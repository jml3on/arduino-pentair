/*
 *      Author: jm
 */

#ifdef UNITTEST

#include "packet_test.h"

class PumpStatusTest : public PacketTest {
 protected:
  PumpStatusResponse &actual = p_.pump_status_response;

  void P(const char *data) {
    String preamble("ff 00 ff ");
    String header("a5 00 10 60 07 0f ");
    String checksum = CheckSum(header + data);
    String packet = preamble + header + data + " " + checksum;
    EXPECT_TRUE(this->ReadPacket(packet.c_str()));
    EXPECT_EQ(true, p_.IsValid());
  }
};

TEST_F(PumpStatusTest, Header) {
  P("04 00 00 00 00 00 00 00 00 00 00 00 00 08 0b");

  EXPECT_EQ(DEVICE_PUMP1, p_.source_);
  EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.target_);
  EXPECT_EQ(COMMAND_PUMP_STATUS, p_.cid_);
}

TEST_F(PumpStatusTest, Response_Off) {
  P("04 00 00 00 00 00 00 00 00 00 00 00 00 08 0b");

  ASSERT_FALSE(actual.running_);
  ASSERT_EQ(0, actual.watts_);
  ASSERT_EQ(0, actual.rpms_);
  ASSERT_EQ(8, actual.hour_);
  ASSERT_EQ(11, actual.minutes_);
  ASSERT_FALSE(actual.priming_);
}

TEST_F(PumpStatusTest, Response_Running) {
  P("0a 00 00 02 83 08 98 00 00 00 00 00 01 13 06");

  ASSERT_TRUE(actual.running_);
  ASSERT_EQ(643, actual.watts_);
  ASSERT_EQ(2200, actual.rpms_);
  ASSERT_EQ(19, actual.hour_);
  ASSERT_EQ(06, actual.minutes_);
  ASSERT_FALSE(actual.priming_);
}

TEST_F(PumpStatusTest, Response_Priming) {
  P("0a 00 00 02 83 08 98 00 00 00 00 00 0b 13 06");

  ASSERT_TRUE(actual.running_);
  ASSERT_EQ(643, actual.watts_);
  ASSERT_EQ(2200, actual.rpms_);
  ASSERT_EQ(19, actual.hour_);
  ASSERT_EQ(06, actual.minutes_);
  ASSERT_TRUE(actual.priming_);
}

#endif // UNITTEST

