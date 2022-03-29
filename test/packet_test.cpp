#ifdef UNITTEST
#include "gtest.h"

#include "packet.h"
#include "string_print.h"

#include "packet_test.h"

TEST_F(PacketTest, PrintHex) {
  {
    StringPrint hex;  // @suppress("Abstract class cannot be instantiated")
    PrintHex(hex, 0x00);
    EXPECT_STREQ("00", hex.c_str());
  }
  {
    StringPrint hex;  // @suppress("Abstract class cannot be instantiated")
    PrintHex(hex, 1);
    EXPECT_STREQ("01", hex.c_str());
  }
  {
    StringPrint hex;  // @suppress("Abstract class cannot be instantiated")
    PrintHex(hex, 0x0F);
    EXPECT_STREQ("0f", hex.c_str());
  }
  {
    StringPrint hex;  // @suppress("Abstract class cannot be instantiated")
    PrintHex(hex, 0x10);
    EXPECT_STREQ("10", hex.c_str());
  }
  {
    StringPrint hex;  // @suppress("Abstract class cannot be instantiated")
    PrintHex(hex, 0x3b);
    EXPECT_STREQ("3b", hex.c_str());
  }
  {
    StringPrint hex;  // @suppress("Abstract class cannot be instantiated")
    PrintHex(hex, 0xf0);
    EXPECT_STREQ("f0", hex.c_str());
  }
  {
    StringPrint hex;  // @suppress("Abstract class cannot be instantiated")
    PrintHex(hex, 0xFF);
    EXPECT_STREQ("ff", hex.c_str());
  }
}

TEST_F(PacketTest, ReadFrom) {
  String p = "10  02  50  11";   // chlorinator packet
  p += " " + CheckSum(p) + " 10  03";

  p_.ReadFrom(p.c_str());

  EXPECT_EQ(true, p_.IsValid());
  EXPECT_EQ(DEVICE_CONTROL_CENTER, p_.source_);
  EXPECT_EQ(DEVICE_CHLORINATOR, p_.target_);
  EXPECT_EQ(COMMAND_CHLORINATOR, p_.cid_);
  EXPECT_TRUE(p_.chlorinator_request.mode_);
  EXPECT_EQ(0, p_.chlorinator_request.percent_);
}

TEST_F(PacketTest, CheckSum) {
  const char *checksum =
      CheckSum(
          "a5 25 0f 10 02 1d 08 18 00 00 00 00 00 00 00 00 03 00 20 04 44 44 20 00 3b 3c 00 00 07 00 00 a9 b4 00 0d")
          .c_str();
  EXPECT_STREQ("03 df", checksum);

  const char *checksum2 = CheckSum("a5 25 10 20 cb 01 02").c_str();
  EXPECT_STREQ("01 c8", checksum2);
}

TEST_F(PacketTest, CheckSumSpaces) {
  const char *checksum = CheckSum(" 10    02  50 \t 11 ").c_str();
  EXPECT_STREQ("00 73", checksum);
}

TEST_F(PacketTest, WriteTo_00) {
  const char* expected = "10 02 50 11 64 d7 10 03 ";
  p_.ReadFrom(expected);

  StringPrint actual;
  HexPrint hex(&actual);
  p_.WriteTo(hex);
  EXPECT_STREQ(expected, actual.c_str());
}

TEST_F(PacketTest, WriteTo_25) {
  const char* expected = "ff 00 ff a5 25 0f 10 02 1d 0c 0e 20 00 00 00 00 00 00 00 33 00 20 04 50 50 00 00 4b 65 00 00 03 00 00 77 7f 00 0d 03 ef ";
  p_.ReadFrom(expected);

  StringPrint actual;
  HexPrint hex(&actual);
  p_.WriteTo(hex);
  EXPECT_STREQ(expected, actual.c_str());
}

TEST_F(PacketTest, Make) {
  const char* data = "01 02";
  const char* expected = "ff 00 ff a5 25 10 20 cb 01 02 01 c8 ";
  p_.Make(0xcb, data);

  StringPrint actual;
  HexPrint hex(&actual);
  p_.WriteTo(hex);
  EXPECT_STREQ(expected, actual.c_str());
}

#endif // UNITTEST
