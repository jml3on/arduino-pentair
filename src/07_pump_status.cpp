/*
 *      Author: jm
 */

#include "packet.h"

const char* PumpStatusRequest::Name() {
  return "Pump Status Request";
}

void PumpStatusRequest::Read(const uint8_t *data) {
  // nothing
}

void PumpStatusRequest::PrintTo(Print &printer) {
  // nothing
}

const char* PumpStatusResponse::Name() {
  return "Pump Status Response";
}

void PumpStatusResponse::Read(const uint8_t *data_) {
  running_ = (data_[0] == 0x0a) ? true : false;
  watts_ = toUint16(data_[3], data_[4]);
  rpms_ = toUint16(data_[5], data_[6]);
  priming_ = (data_[12] == 0x0b) ? true : false;
  hour_ = data_[13];
  minutes_ = data_[14];
}

void PumpStatusResponse::PrintTo(Print &p) {
  PrintLabel(p, "time");
  PrintTime(p, hour_, minutes_);
  p.println();

  PrintLabel(p, "running");
  p.print((running_ ? "Yes" : "No"));
  p.println();

  PrintLabel(p, "priming");
  p.print((priming_ ? "Yes" : "No"));
  p.println();

  PrintLabel(p, "watts");
  p.print(watts_);
  p.println();

  PrintLabel(p, "rpms");
  p.print(rpms_);
  p.println();
}
