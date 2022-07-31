/*
 *      Author: jm
 */

#include "packet.h"

const char* ChlorinatorRequest::Name() {
  return "Chlorinator Request";
}

void ChlorinatorRequest::Read(const uint8_t *data) {
  mode_ = (data[1] == 0x11) ? true : false;
  percent_ = data[2];
}

void ChlorinatorRequest::PrintTo(Print &p) {
  PrintLabel(p, "running");
  p.print(mode_ ? "Yes" : "No");
  p.println();

  PrintLabel(p, "percent");
  p.print(percent_);
  p.println();
}

const char* ChlorinatorResponse::Name() {
  return "Chlorinator Response";
}

void ChlorinatorResponse::Read(const uint8_t *data) {
  salinity_ = (uint16_t) data[2] * (uint16_t) 50;
  inspect_cell_ = data[3] & INSPECT_CELL_MASK;
}

void ChlorinatorResponse::PrintTo(Print &p) {
  PrintLabel(p, "ppm");
  p.print(salinity_);
  p.println();
  PrintLabel(p, "inspect");
  p.print(inspect_cell_ ? "Yes" : "No");
  p.println();
}

