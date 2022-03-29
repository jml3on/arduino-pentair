/*
 *      Author: jm
 */

#include "packet.h"

const char* PumpControlRequest::Name() {
  return "Pump Control Request";
}

void PumpControlRequest::Read(const uint8_t *data) {
  running_ = (data[0] == 0x10) ? true : false;
}

void PumpControlRequest::PrintTo(Print &printer) {
  PrintLabel(printer, "running");
  printer.print(running_ ? "Yes" : "No");
  printer.println();
}

const char* PumpControlResponse::Name() {
  return "Pump Control Response";
}

void PumpControlResponse::Read(const uint8_t *data) {
  running_ = (data[0] == 0x10) ? true : false;
}

void PumpControlResponse::PrintTo(Print &printer) {
  PrintLabel(printer, "running");
  printer.print(running_ ? "Yes" : "No");
  printer.println();
}
