
#include <EEPROM.h>

/**
 * Write secrets (WIFI Credentials & Arduino cloud config) to EEPROM.
 */
const char* WIFI_SSID = "YOUR WIFI SSID";
const char* WIFI_PASSWORD = "YOUR WIFI PASSWORD";
const char* DEVICE_ID = "YOUR ARDUINO CLOUD DEVICE ID";
const char* DEVICE_KEY = "YOUR ARDUINO CLOUD DEVICE KEY";

#define SECRET_COUNT 4

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(5000); 
  
  Serial.println("writting EEPROM");
  EEPROM.begin(128);
  const char *secrets[SECRET_COUNT] = {
    WIFI_SSID, WIFI_PASSWORD, DEVICE_ID, DEVICE_KEY,
  };
  int address = 0;
  for(int s = 0; s < SECRET_COUNT; s++) {  
    const char *secret = secrets[s];  
    Serial.println(secret);
    for(int i = 0; i <= strlen(secret); i++, address++) {
      EEPROM.write(address, secret[i]);
    }
  }
  
  Serial.print("wrote ");
  Serial.print(address);
  Serial.println(" bytes.");
  
  if (EEPROM.commit()) {
    Serial.println("EEPROM successfully committed");
  } else {
    Serial.println("ERROR! EEPROM commit failed");
  }
}

void loop() {
  Serial.println("w");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
