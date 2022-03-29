/*
 *      Author: jm
 */
#ifdef UNITTEST

#include "packet_test.h"

class ChlorinatorTest : public PacketTest {
 protected:
  void P(const char *data) {
    String packet = "10 02 ";  // chlorinator preamble
    packet += data;
    String checksum = CheckSum(packet);
    packet += " " + checksum + " 10 03";
    EXPECT_TRUE(this->ReadPacket(packet.c_str()));
    EXPECT_EQ(true, p_.IsValid());
  }
};

TEST_F(ChlorinatorTest, Request) {
  ChlorinatorRequest &actual = p_.chlorinator_request;
  {
    P("50 00 01");

    EXPECT_EQ(true, p_.IsValid());
    EXPECT_EQ(0, p_.version_);
    EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.source_);
    EXPECT_EQ(DEVICE_CHLORINATOR, p_.target_);
    EXPECT_EQ(COMMAND_CHLORINATOR, p_.cid_);
    EXPECT_FALSE(actual.mode_);
    EXPECT_EQ(1, actual.percent_);
  }
  {
    SetUp();
    P("50 11 64");

    EXPECT_EQ(true, p_.IsValid());
    EXPECT_EQ(0, p_.version_);
    EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.source_);
    EXPECT_EQ(DEVICE_CHLORINATOR, p_.target_);
    EXPECT_EQ(COMMAND_CHLORINATOR, p_.cid_);
    EXPECT_TRUE(actual.mode_);
    EXPECT_EQ(100, actual.percent_);
  }
}

TEST_F(ChlorinatorTest, Response) {
  ChlorinatorResponse &actual = p_.chlorinator_response;
  P("00 01 36 00");

  EXPECT_EQ(0, p_.version_);
  EXPECT_EQ(DEVICE_CHLORINATOR, p_.source_);
  EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.target_);
  EXPECT_EQ(COMMAND_CHLORINATOR, p_.cid_);
  EXPECT_EQ(2700, actual.salinity_);
}

#endif // UNITTEST

