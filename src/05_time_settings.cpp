/*
 *      Author: jm
 */

#include "packet.h"

static const char* DayOfWeek(byte dow) {
  switch (dow) {
    case DAY_OF_WEEK_SUNDAY:
      return "Sun";
    case DAY_OF_WEEK_MONDAY:
      return "Mon";
    case DAY_OF_WEEK_TUESDAY:
      return "Tue";
    case DAY_OF_WEEK_WEDNESDAY:
      return "Wed";
    case DAY_OF_WEEK_THURSDAY:
      return "Thu";
    case DAY_OF_WEEK_FRIDAY:
      return "Fri";
    case DAY_OF_WEEK_SATURDAY:
      return "Sat";
    default:
      return "???";
  }
}
const char* TimeSettings::Name() {
  return "Time Settings";
}

void TimeSettings::Read(const uint8_t *data_) {
  hour_ = data_[0];
  minute_ = data_[1];
  week_day_ = data_[2];
  day_ = data_[3];
  month_ = data_[4];
  year_ = data_[5];
  // byte 6 carries the clock accuracy offset. hi bit is sign, 7 bits value
  // 0x01 means 5 seconds; 0x81 means -5 seconds.
  int sign = (data_[6] & 0x80) ? -5 : +5;
  clock_accuracy_offset_ = sign * (data_[6] & 0x7F);
  auto_adjust_dst_ = data_[7];
}

void TimeSettings::PrintTo(Print &p) {
  PrintLabel(p, "time");
  PrintTime(p, hour_, minute_);
  p.println();

  PrintLabel(p, "date");
  p.print(DayOfWeek(week_day_));
  p.print(" ");
  PrintDate(p, day_, month_, year_);
  p.println();

  PrintLabel(p, "dst");
  p.print(auto_adjust_dst_ ? "Auto" : "Manual");
  p.println();

  PrintLabel(p, "adjust");
  p.print(clock_accuracy_offset_);
  p.println();
}

