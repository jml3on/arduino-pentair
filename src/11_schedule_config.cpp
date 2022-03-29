/*
 *      Author: jm
 */

#include "packet.h"

const char* ScheduleConfig::Name() {
  return "Schedule Config";
}

void ScheduleConfig::Read(const uint8_t *data_) {
  circuit_ = data_[0];
  start_hour_ = data_[2];
  start_minute_ = data_[3];
  stop_hour_ = data_[4];
  stop_minute_ = data_[5];
  active_days_ = data_[6];
}

void ScheduleConfig::PrintTo(Print &p) {
  PrintLabel(p, "circuit");
  p.print(circuit_);
  p.println();

  PrintLabel(p, "start");
  PrintTime(p, start_hour_, start_minute_);
  p.println();

  PrintLabel(p, "stop");
  PrintTime(p, stop_hour_, stop_minute_);
  p.println();

  PrintLabel(p, "active");
  const char DAYS[] = "MTWTFSS";
  for (int i = 0; i < 7; i++) {
    boolean on = active_days_ & (1 << i);
    p.print(on ? DAYS[i] : '-');
  }
  p.println();
}

