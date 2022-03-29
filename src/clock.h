/*
 *      Author: jm
 */

#ifndef SRC_CLOCK_H_
#define SRC_CLOCK_H_

#include "Arduino.h"

class Clock {
 public:
  int hour = 0;
  int minute = 0;
  int second = 0;
  long millisecond = 0;
  long last_millis = 0;
  bool initialized = false;

  void initialize(int h, int m) {
    initialize(h, m, millis());
  }

  void initialize(int h, int m, int millis) {
    hour = h;
    minute = m;
    second = 0;
    millisecond = 0;
    last_millis = millis;
    initialized = true;
  }

  void time() {
    long now = millis();
    advance(now - last_millis);
    last_millis = now;
  }

  void advance(const long passed) {
    millisecond += passed;
    second += millisecond / 1000;
    millisecond = millisecond % 1000;
    minute += second / 60;
    second = second % 60;
    hour += minute / 60;
    minute = minute % 60;
    hour = hour % 24;
  }
};

#endif /* SRC_CLOCK_H_ */
