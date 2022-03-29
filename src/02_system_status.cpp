#include "packet.h"

// Define bytes from a intellitouch touch message.
#define HOUR 0
#define MINUTES 1
#define CIRCUITS 2
#define MODE 9
#define SOLAR 10
#define POOL_TEMP 14
#define SPA_TEMP 15
#define AIR_TEMP 18
#define SOLAR_TEMP 19
#define HEAT_SETTINGS 22

#define CIRCUITS_COUNT 8

static const char *CIRCUITS_NAMES[CIRCUITS_COUNT] = { "SPA", "CLEANER", "??",
    "??", "??", "POOL", "??", "??" };

const char* SystemModeName(byte mode) {
  switch (mode) {
    case SYSTEM_MODE_AUTO:
      return "Auto";
    case SYSTEM_MODE_SERVICE:
      return "Service";
    case SYSTEM_MODE_TIMEOUT:
      return "Timeout";
  }
  return "Unknown";
}

const char* HeatSourceName(byte settings) {
  const char *NAMES[] = { "Off", "Heater", "Solar Pref", "Solar" };
  if (settings < sizeof(NAMES)) {
    return NAMES[settings];
  }
  return "Unknown";
}

const char* tempUnit(int celcius) {
  return celcius ? "C" : "F";
}

// 1 bit specifies if temperatures are in F (0) or C (1).
#define CELCIUS_MASK 0x4
// 2 bits are on when solar is on
#define SOLAR_MASK 0x30
// mask to extract the control center's current mode
#define MODE_MASK 0x81

const char* SystemStatus::Name() {
  return "System Status Broadcast";
}

void SystemStatus::Read(const uint8_t *data_) {
  hour_ = data_[HOUR];
  minutes_ = data_[MINUTES];
  for (uint8_t c = 0; c < CIRCUITS_COUNT; c++) {
    circuits_[c] = data_[CIRCUITS] & (1 << c);
  }
//  uint8_t mode = data_[MODE] & MODE_MASK;
  mode_ = data_[MODE] & MODE_MASK;
  celcius_ = (data_[MODE] & CELCIUS_MASK) == CELCIUS_MASK;
  solar_ = (data_[SOLAR] & SOLAR_MASK) == SOLAR_MASK;
  // bits 1 & 2 contain the pool heat settings
  pool_heat_source_ = data_[HEAT_SETTINGS] & 0x03;
  // bits 3 & 4 contain the spa heat settings
  spa_heat_source_ = ((data_[HEAT_SETTINGS] >> 2) & 0x03);
  air_temp_ = data_[AIR_TEMP];
  pool_temp_ = data_[POOL_TEMP];
  spa_temp_ = data_[SPA_TEMP];
  solar_temp_ = data_[SOLAR_TEMP];
}

void SystemStatus::PrintTo(Print &p) {
  PrintLabel(p, "time");
  PrintTime(p, hour_, minutes_);
  p.println();

  PrintLabel(p, "mode");
  p.println(SystemModeName(mode_));

  PrintLabel(p, "solar");
  p.println(solar_ ? "On" : "Off");

  PrintLabel(p, "air T");
  p.print(air_temp_);
  p.println(tempUnit(celcius_));

  PrintLabel(p, "pool T");
  p.print(pool_temp_);
  p.println(tempUnit(celcius_));

  PrintLabel(p, "spa T");
  p.print(spa_temp_);
  p.println(tempUnit(celcius_));

  PrintLabel(p, "solar T");
  p.print(solar_temp_);
  p.println(tempUnit(celcius_));

  PrintLabel(p, "circuits");
  for (int c = 0; c < 8; c++) {
    p.print(CIRCUITS_NAMES[c]);
    p.print(circuits_[c] ? "=ON" : "=OFF");
    p.print(" ");
  }
  p.println();

  PrintLabel(p, "pool heat");
  p.println(HeatSourceName(pool_heat_source_));

  PrintLabel(p, "spa heat");
  p.println(HeatSourceName(spa_heat_source_));
}
