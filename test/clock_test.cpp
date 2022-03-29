#ifdef UNITTEST
/*
 *      Author: jm
 */
#include "gtest.h"

#include "packet.h"

class ClockTest : public ::testing::Test {
  // empty
};

TEST_F(ClockTest, Init) {
  Clock c;
  ASSERT_FALSE(c.initialized);
  c.initialize(0, 0, 0);
  ASSERT_TRUE(c.initialized);
  ASSERT_EQ(0, c.hour);
  ASSERT_EQ(0, c.minute);
  ASSERT_EQ(0, c.second);
  ASSERT_EQ(0, c.millisecond);
  ASSERT_EQ(0, c.last_millis);
}

TEST_F(ClockTest, Advance) {
  Clock c;
  c.initialize(0, 0, 0);

  c.advance(1000);
  ASSERT_EQ(0, c.hour);
  ASSERT_EQ(0, c.minute);
  ASSERT_EQ(1, c.second);
  ASSERT_EQ(0, c.millisecond);

  c.advance(50);
  ASSERT_EQ(0, c.hour);
  ASSERT_EQ(0, c.minute);
  ASSERT_EQ(1, c.second);
  ASSERT_EQ(50, c.millisecond);

  c.advance(50);
  ASSERT_EQ(0, c.hour);
  ASSERT_EQ(0, c.minute);
  ASSERT_EQ(1, c.second);
  ASSERT_EQ(100, c.millisecond);

  // + 1 minute
  c.advance(60 * 1000);
  ASSERT_EQ(0, c.hour);
  ASSERT_EQ(1, c.minute);
  ASSERT_EQ(1, c.second);
  ASSERT_EQ(100, c.millisecond);

  // + 1 hour
  c.advance(60 * 60 * 1000);
  ASSERT_EQ(1, c.hour);
  ASSERT_EQ(1, c.minute);
  ASSERT_EQ(1, c.second);
  ASSERT_EQ(100, c.millisecond);

  // + 59 seconds
  c.advance(59 * 1000);
  ASSERT_EQ(1, c.hour);
  ASSERT_EQ(2, c.minute);
  ASSERT_EQ(0, c.second);
  ASSERT_EQ(100, c.millisecond);

  // + 1 day
  c.advance(24 * 60 * 60 * 1000);
  ASSERT_EQ(1, c.hour);
  ASSERT_EQ(2, c.minute);
  ASSERT_EQ(0, c.second);
  ASSERT_EQ(100, c.millisecond);
}




#endif // UNITTEST
