/*
 *      Author: jm
 */

#ifndef PACKET_TEST_H_
#define PACKET_TEST_H_

#ifdef UNITTEST

#include "gtest.h"

#include "packet.h"

class PacketTest : public ::testing::Test {
 private:
  void invalidChar(char c) {
    ASSERT_FALSE(true)<< "invalid hex character: " << c;
  }

protected:
  void SetUp() override {
    p_.Reset();
  }

  // Read a Frame formatted as an hex string, e.g. "ff 10 02 50 00 00 62 10 03"
  bool ReadPacket(const char *f) {
    p_.Reset();
    return p_.ReadFrom(f);
  }

  const String asHex(uint8_t c) {
    return c < 0x10 ? "0" + String((int) c, HEX) : String((int) c, HEX);
  }

  String CheckSum(const char *f) {
    uint16_t chk = 0;
    for (int i = 0; i < strlen(f); i++) {
      if (f[i] != ' ' && f[i] != '\t') {
        char hi = f[i++];
        char lo = f[i];
        uint8_t b = htoi(hi) * 16 + htoi(lo);
        chk += b;
      }
    }
    byte hi = chk / 256;
    byte lo = chk & 0xff;
    return asHex(hi) + " " + asHex(lo);
  }

  String CheckSum(const String &f) {
    return CheckSum(f.c_str());
  }

  Packet p_;
};

#endif // UNITTEST

#endif /* PACKET_TEST_H_ */
