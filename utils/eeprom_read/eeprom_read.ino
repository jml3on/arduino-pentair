
#include <EEPROM.h>
/**
   Read secrets previously stored using utils/write_eeprom.ino
*/

char eeprom_data[128];
char *wifi_ssid = eeprom_data;
char *wifi_password = 0;
char *device_id = 0;
char *device_key = 0;

void readSecrets() {
  EEPROM.begin(sizeof(eeprom_data));
  int component = 0;
  boolean done = false;
  for (unsigned int address = 0; !done && address < sizeof(eeprom_data); address++) {
    byte c = EEPROM.read(address);
    eeprom_data[address] = c;
    if (c == 0) {
      switch (component++) {
        case 0:
          wifi_password = eeprom_data + address + 1;
          break;
        case 1:
          device_id = eeprom_data + address + 1;
          break;
        case 2:
          device_key = eeprom_data + address + 1;
          break;
        case 3:
          done = true;
          break;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  readSecrets();

  Serial.println(wifi_ssid);
  Serial.println(wifi_password);
  Serial.println(device_id);
  Serial.println(device_key);
}

void loop() {
  Serial.println("r");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
