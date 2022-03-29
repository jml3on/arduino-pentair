/*
 *      Author: jm
 */

#include "packet.h"

const char* HeatStatus::Name() {
  return "Heat Status";
}

void HeatStatus::Read(const uint8_t *data) {
  pool_temp_ = data[0];
  spa_temp_ = data[1];
  air_temp_ = data[2];
  solar_temp_ = data[8];
  pool_temp_target_ = data[3];
  pool_heat_source_ = data[5] & 0x03;
  spa_temp_target_ = data[4];
  spa_heat_source_ = (data[5] >> 2) & 0x03;
}

void HeatStatus::PrintTo(Print &p) {
  PrintLabel(p, "pool T");
  p.print(pool_temp_);
  p.println();

  PrintLabel(p, "spa T");
  p.print(spa_temp_);
  p.println();

  PrintLabel(p, "air T");
  p.print(air_temp_);
  p.println();

  PrintLabel(p, "solar T");
  p.print(solar_temp_);
  p.println();

  PrintLabel(p, "pool heat");
  p.print(pool_temp_target_);
  p.print("/");
  p.print(HeatSourceName(pool_heat_source_));
  p.println();

  PrintLabel(p, "spa heat");
  p.print(spa_temp_target_);
  p.print("/");
  p.print(HeatSourceName(spa_heat_source_));
  p.println();
}

