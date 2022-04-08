#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "SoftwareSerial.h"

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#include "packet.h"

#define SSerialReceive 4          // GPIO4 = D2 pin <-> RS485 RO (Receive Out)
#define SSerialTransmit 0         // GPIO0 = D3 pin <-> RS485 DI (Data In)
#define RS485DirectionControl 14  // GPI14 = D5 pin <-> RS485 DE (Data Enable) & RE (Receive Enable)

#define RS485Transmit HIGH
#define RS485Receive LOW

SoftwareSerial rs485(SSerialReceive, SSerialTransmit);  // @suppress("Abstract class cannot be instantiated")

WiFiServer server(80);  // @suppress("Abstract class cannot be instantiated")
char eeprom_data[128];
char *wifi_ssid = eeprom_data;
char *wifi_password = 0;
char *device_id = 0;
char *device_key = 0;

WiFiConnectionHandler *ArduinoIoTPreferredConnection = NULL;

CloudTemperatureSensor airTemp;
CloudTemperatureSensor poolTemp;
CloudTemperatureSensor solarTemp;
int salinity;

Packet packet;  // last received pentair packet.

void initArduinoCloud() {
  ArduinoCloud.setBoardId(device_id);
  ArduinoCloud.setSecretDeviceKey(device_key);

  ArduinoCloud.addProperty(airTemp, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(poolTemp, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(solarTemp, READ, 60 * SECONDS, NULL);
  ArduinoCloud.addProperty(salinity, READ, 60 * SECONDS, NULL);

  ArduinoIoTPreferredConnection = new WiFiConnectionHandler(wifi_ssid, wifi_password);
  ArduinoCloud.begin(*ArduinoIoTPreferredConnection);

  setDebugMessageLevel(10);
  ArduinoCloud.printDebugInfo();
}

void updateArduinoCloud(){
  airTemp = packet.system_status.air_temp_;
  poolTemp = packet.system_status.pool_temp_;
  solarTemp = packet.system_status.solar_temp_;
  salinity = packet.chlorinator_response.salinity_;
  ArduinoCloud.update();
}


/**
 * Read secrets previously stored using utils/write_eeprom.ino
 */
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

void initServer() {
  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to '");
  Serial.print(wifi_ssid);
  Serial.println("'");
  Serial.print("with '");
  Serial.print(wifi_password);
  Serial.println("'");
  WiFi.hostname("pentair");
  WiFi.begin(wifi_ssid, wifi_password);

  int elapsed = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
    elapsed++;
    if (elapsed > 30) {
      Serial.println("Restarting in 3 seconds");
      delay(3000);
      ESP.restart();
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RS485DirectionControl, OUTPUT);
  digitalWrite(RS485DirectionControl, RS485Receive);

  rs485.begin(9600);

  readSecrets();
  initArduinoCloud();
  initServer();

  Serial.println("Monitor Ready.");
}

void ExportMetrics(WiFiClient &client) {
  client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain; version=0.0.4\r\n\r\n");
  client.print("air_temp "); client.print(packet.system_status.air_temp_); client.print("\n");
  client.print("pool_temp "); client.print(packet.system_status.pool_temp_); client.print("\n");
  client.print("solar_temp "); client.print(packet.system_status.solar_temp_); client.print("\n");
  client.print("spa_temp "); client.print(packet.system_status.spa_temp_); client.print("\n");

  client.print("pump_rpms "); client.print(packet.pump_status_response.rpms_); client.print("\n");
  client.print("pump_watts "); client.print(packet.pump_status_response.watts_); client.print("\n");

  client.print("chlorinator_percent "); client.print(packet.chlorinator_request.percent_); client.print("\n");
  client.print("chlorinator_salinity "); client.print(packet.chlorinator_response.salinity_); client.print("\n");

  client.print("\n");
}

void ExportCSV(WiFiClient &client) {
  client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain; version=0.0.4\r\n\r\n");
  client.print(packet.system_status.air_temp_); client.print(",");
  client.print(packet.system_status.pool_temp_); client.print(",");
  client.print(packet.system_status.solar_temp_); client.print(",");
  client.print(packet.system_status.spa_temp_); client.print(",");

  client.print(packet.pump_status_response.rpms_); client.print(",");
  client.print(packet.pump_status_response.watts_); client.print(",");

  client.print(packet.chlorinator_request.percent_); client.print(",");
  client.print(packet.chlorinator_response.salinity_); client.print("\n");

  client.print("\n");
}

void NotFound(WiFiClient &client) {
  client.print("HTTP/1.1 404 Not Found\n");
}

void Ok(WiFiClient &client) {
  client.print("HTTP/1.1 200 OK\n");
}

boolean SENT = true;
void ServeClient(WiFiClient &client) {
  // Wait until the client sends some data
  Serial.println("new client");
  unsigned long timeout = millis() + 3000;
  while (!client.available() && millis() < timeout) {
    delay(1);
  }
  if (millis() > timeout) {
    Serial.println("timeout");
    client.flush();
    client.stop();
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.print("req=");
  Serial.println(req);
  client.flush();

  if (req.startsWith("GET /metrics")) {
    ExportMetrics(client);
  } else if (req.startsWith("GET /csv")) {
    ExportCSV(client);
  } else if (req.startsWith("GET /send")) {
    SENT = false;
    Ok(client);
  } else {
    NotFound(client);
  }
  delay(1);
  Serial.println("Client disconnected");

}

void loop() {
  boolean send_ready = false;
  while (rs485.available()) {
    byte c = rs485.read();
    if (packet.Push(c)) {
      if (packet.IsValid()) {
        // It is not clear when a good time is to send packets and avoid collision.
        // Main controller status broadcast is always followed by some quiet time, so we'll use that for now to send.
        send_ready = (packet.cid_ == COMMAND_SYSTEM_STATUS);
        packet.PrintTo(Serial);
        updateArduinoCloud();
      } else {
        // skip invalids for now
        Serial.println("   ***   INVALID PACKET ***");
        packet.PrintTo(Serial);
      }
      packet.Reset();
    }
  }
  if (!SENT && send_ready) {
    Packet out;
    // Test write. ask for circuit_name[02]. response should be a 0x0b message.
    out.Make(0xcb, "01 02");
    //   0x00 0xFF 0xA5 0x01   0x10 0x48   0x86  0x02 0x01 0x01   0x01 0x88
    digitalWrite(RS485DirectionControl, RS485Transmit);
    if (rs485.availableForWrite()) {
      out.WriteTo(rs485);
      Serial.print(out.data_len_);
      Serial.println(" data bytes written");
      rs485.flush();
      Serial.println("flushed");
      Serial.println("---------------------------------");
      SENT = true;
    }
    digitalWrite(RS485DirectionControl, RS485Receive);
  }

  WiFiClient client = server.available();  // @suppress("Abstract class cannot be instantiated")
  if (client) {
    ServeClient(client);
  }
}
