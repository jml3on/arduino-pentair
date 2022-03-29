/*
 *      Author: jm
 */
#ifdef UNITTEST

#include "packet_test.h"

class PumpControlTest : public PacketTest {
 protected:
  void P(const char *preamble, const char *data) {
    // we need to handle request/response which have different headers.
    String checksum = CheckSum(data);
    String packet(preamble);
    packet += String(" ") + data + " " + checksum;
    EXPECT_TRUE(this->ReadPacket(packet.c_str()));
    EXPECT_EQ(true, p_.IsValid());
  }
};

TEST_F(PumpControlTest, Request_Off) {
  P("ff 00 ff", "a5 00 60 10 06 01 04");

  EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.source_);
  EXPECT_EQ(DEVICE_PUMP1, p_.target_);
  EXPECT_EQ(COMMAND_PUMP_CONTROL, p_.cid_);
  EXPECT_FALSE(p_.pump_control_request.running_);
}

TEST_F(PumpControlTest, Response_Off) {
  P("ff ff 00 ff", "a5 00 10 60 06 01 04");

  EXPECT_EQ(DEVICE_PUMP1, p_.source_);
  EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.target_);
  EXPECT_EQ(COMMAND_PUMP_CONTROL, p_.cid_);
  EXPECT_FALSE(p_.pump_control_response.running_);
}

TEST_F(PumpControlTest, Request_On) {
  P("ff 00 ff", "a5 00 60 10 06 01 10");

  EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.source_);
  EXPECT_EQ(DEVICE_PUMP1, p_.target_);
  EXPECT_EQ(COMMAND_PUMP_CONTROL, p_.cid_);
  EXPECT_TRUE(p_.pump_control_request.running_);
}

TEST_F(PumpControlTest, Response_On) {
  P("ff ff 00 ff", "a5 00 10 60 06 01 10 01 2c");

  EXPECT_EQ(DEVICE_PUMP1, p_.source_);
  EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.target_);
  EXPECT_EQ(COMMAND_PUMP_CONTROL, p_.cid_);
  EXPECT_TRUE(p_.pump_control_response.running_);
}

#endif // UNITTEST

