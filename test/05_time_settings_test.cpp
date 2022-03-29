/*
 *      Author: jm
 */
#ifdef UNITTEST

#include "packet_test.h"

class TimeConfigTest : public PacketTest {
 protected:
  TimeSettings &actual = p_.time_settings;
  void P(const char *data) {
    String preamble("ff 00 ff ");
    String header("a5 25 0f 10 05 08 ");
    String checksum = CheckSum(header + data);
    String packet = preamble + header + data + " " + checksum;
    EXPECT_TRUE(this->ReadPacket(packet.c_str()));
    EXPECT_EQ(true, p_.IsValid());
  }
};

TEST_F(TimeConfigTest, Header) {
  P("0b 0a 02 14 08 12 00 00");

  EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.source_);
  EXPECT_EQ(DEVICE_BROADCAST, p_.target_);
  EXPECT_EQ(COMMAND_TIME_SETTINGS, p_.cid_);
}

TEST_F(TimeConfigTest, AllValues) {
  P("0b 0a 02 14 08 12 00 00");

  EXPECT_EQ(11, actual.hour_);
  EXPECT_EQ(10, actual.minute_);
  EXPECT_EQ(DAY_OF_WEEK_MONDAY, actual.week_day_);
  EXPECT_EQ(20, actual.day_);
  EXPECT_EQ(8, actual.month_);
  EXPECT_EQ(18, actual.year_);
  EXPECT_EQ(0, actual.clock_accuracy_offset_);
  EXPECT_FALSE(actual.auto_adjust_dst_);
}

TEST_F(TimeConfigTest, DST_Auto) {
  P("0b 0a 02 14 08 12 00 01");
  EXPECT_TRUE(actual.auto_adjust_dst_);
}

TEST_F(TimeConfigTest, Clock_Adjust) {
  {
    SetUp();
    P("0b 0a 02 14 08 12 01 01");
    EXPECT_EQ(5, actual.clock_accuracy_offset_);
  }
  {
    SetUp();
    P("0b 0a 02 14 08 12 81 01");
    EXPECT_EQ(-5, actual.clock_accuracy_offset_);
  }
}

#endif // UNITTEST

