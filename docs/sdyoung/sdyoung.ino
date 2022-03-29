// Auduino Code v2.6.1 29-JUN-2016
// Source structure graciously provided by draythomp
// (http://www.desert-home.com/p/swimming-pool.html)
// and sufficiently bastardized for Pentair protocol

#include <HttpClient.h>
#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>s
#include <EthernetUdp.h>
#include <Time.h>
#include <CountingStream.h>
#include <avr/wdt.h>

//RS-485 write stuff
//   PACKET FORMAT        <------------------NOISE---------------->  <------PREAMBLE------>  Sub   Dest  Src   CFI   Len   Dat1  Dat2  ChkH  ChkL     //Checksum is sum of bytes A5 thru Dat2.
byte cleanerOn[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x86, 0x02, 0x01, 0x01, 0x01, 0x66};
byte cleanerOff[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x86, 0x02, 0x01, 0x00, 0x01, 0x65};
byte poolLightOn[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x86, 0x02, 0x02, 0x01, 0x01, 0x67};
byte poolLightOff[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x86, 0x02, 0x02, 0x00, 0x01, 0x66};
byte waterFallOn[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x86, 0x02, 0x03, 0x01, 0x01, 0x68};
byte waterFallOff[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x86, 0x02, 0x03, 0x00, 0x01, 0x67};
byte poolOn[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x86, 0x02, 0x06, 0x01, 0x01, 0x6B};
byte poolOff[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x86, 0x02, 0x06, 0x00, 0x01, 0x6A};
uint8_t saltPctQuery[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0xD9, 0x01, 0x00, 0x01, 0xB6};                 //triggers salt percent setting
uint8_t chlorStep1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x99, 0x04, 0x01, 0x02, 0x00, 0x00, 0x01, 0x7C}; // 2% chlorination
uint8_t chlorStep2[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x99, 0x04, 0x01, 0x05, 0x00, 0x00, 0x01, 0x7F}; // 5% chlorination
uint8_t chlorStep3[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x99, 0x04, 0x01, 0x0A, 0x00, 0x00, 0x01, 0x84}; //10% chlorination
uint8_t chlorStep4[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x99, 0x04, 0x01, 0x0F, 0x00, 0x00, 0x01, 0x89}; //15% chlorination
uint8_t chlorStep5[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x99, 0x04, 0x01, 0x14, 0x00, 0x00, 0x01, 0x8E}; //20% chlorination
uint8_t chlorStep6[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x99, 0x04, 0x01, 0x19, 0x00, 0x00, 0x01, 0x93}; //25% chlorination
uint8_t chlorStep7[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x99, 0x04, 0x01, 0x21, 0x00, 0x00, 0x01, 0x9B}; //33% chlorination
uint8_t chlorStep8[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xA5, 0x07, 0x10, 0x20, 0x99, 0x04, 0x01, 0x32, 0x00, 0x00, 0x01, 0xAC}; //50% chlorination

#define REQ_BUF_SZ 60

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetServer server(2560);     // create a server at port 2560
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer

EthernetClient php;
char phpServer[] = {"192.168.1.000"}; //RasPi Python Script

//NTP
IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov
EthernetUDP Udp;
unsigned int localPort = 8888; // local port to listen for UDP packets
int timeZone = -7;             // set this shit later

//Vera HA info
char veraServer[] = {"192.168.1.000"}; //Vera HA IP
EthernetClient vera;

// RS-485 read stuff
uint8_t buffer[256];
uint8_t *bPointer;
uint8_t bufferOfBytes[256];
uint8_t *bPointerOfBytes;

#define DTR 2
//#define onBoardLed 13
#define header1 1
#define header3 2
#define header4 3
#define bufferData 4
#define calcCheckSum 5
#define saltHead2 6
#define saltTerm 7
#define bufferSaltData 8

String s1 = " This is version 2.6.1 compiled 29-JUN-2016\n Developed by Jason Young";
int goToCase = header1;
int byteNum = 0;
int remainingBytes = 0;
int bytesOfDataToGet = 0;
int chkSumBits = 0;
boolean veraUpdatePending = false;
byte veraVarDevId = 0;
byte veraVarTargetVal = 0;
byte veraVarIDs[] = {0x1A, 0x22, 0x23, 0x24}; //list of Vera VSwitch ID's (in HEX)
byte veraVarVals[4];                          //count of Vera Values to update
byte veraVarLight = 26;
byte veraVarWaterfall = 34;
byte veraVarCirculation = 35;
byte veraVarCleaner = 36;
volatile byte lightState = 0;
byte waterfallState = 0;
byte circulationState = 0;
byte cleanerState = 0;
byte veraMstringChlor = 0x1;
byte veraMstringChlorOn = 0x2;
byte veraMstringWaterTemp = 0x3;
byte veraMstringPumpRPM = 0x4;
byte veraMstringPumpWatts = 0x5;
volatile byte poolTemp = 0;
byte oldPoolTemp = 0;
volatile byte airTemp = 0;
volatile byte poolMode = 0x0;
byte oldPoolMode = 0x0;
volatile byte pumpMode = 0x0;
volatile int pumpWatts = 0;
int oldPumpWatts = 0;
volatile int pumpRPM = 0;
int oldPumpRPM = 0;
volatile byte saltPct = 0;
byte oldSaltPct = 0;
volatile byte saltSetpoint = 0;
boolean saltSetpointTrigger = false;
byte oldSaltSetpoint = 0;
byte saltStateLow = 0;
byte saltStateHigh = 0;
volatile byte saltStateResult = 0xF0;
int saltBytes1 = 0;
int saltBytes2 = 0;
int sumOfBytesInChkSum = 0;
int chkSumValue = 0;
boolean salt = false;
boolean xivelyTrigger = false;
boolean veraSuccess = false;
//byte veraUpdateCount = 0;
unsigned long currentMillis = 0;
long days = 0;
long hours = 0;
long mins = 0;
long secs = 0;
unsigned long rawUptime = 0;
String strUptime;
boolean debug = true;
byte panelHour = 0;
byte panelMinute = 0;
byte ntpHours = 0;
byte ntpMinutes = 0;
unsigned long phpStart = 0;
unsigned long phpStop = 0;
unsigned long saltOutputToggle = 0;
boolean finalXivelyPost = false;
unsigned long xivelyMillis = 0;
int salinityNow = 0;

int freeRam()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
};

void setup()
{
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);

    Serial.println(s1);

    Serial.begin(115200);
    Serial.println(F("Initializing.."));
    Serial1.begin(9600);
    pinMode(DTR, INPUT);
    digitalWrite(DTR, HIGH);
    pinMode(DTR, OUTPUT);
    digitalWrite(DTR, LOW); // to receive from rs485

    pinMode(10, OUTPUT);
    digitalWrite(10, LOW);
    pinMode(53, OUTPUT); //Ethernet
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);
    Serial.print(F("SPI Bus initialized"));
    Serial.println();
    Serial.println();
    Serial.print(F("Waiting for DHCP..."));
    Serial.println();

    byte i = 0;
    int DHCP = 0;
    DHCP = Ethernet.begin(mac);

    while (DHCP == 0 && i < 30)
    { //Try to get dhcp settings 30 times before giving up
        delay(1000);
        DHCP = Ethernet.begin(mac);
        i++;
    }
    if (!DHCP)
    {
        Serial.println(F("DHCP FAILED"));
        for (;;)
            ; //Infinite loop because DHCP Failed
    }
    Serial.println(F("DHCP Success"));
    Serial.println();
    Serial.println();
    Serial.print(F("Ethernet initialized"));
    Serial.println();
    Serial.print(F("IP address assigned by DHCP is "));
    Serial.println(Ethernet.localIP());
    Serial.println();

    Udp.begin(localPort);
    Serial.println(F("Waiting for NTP sync"));
    setSyncProvider(getNtpTime);
    Serial.println();

    server.begin();
    Serial.print(F("HTTP Server started on port 2560 "));
    Serial.println();
    Serial.print(F("I'M ALIVE AT http://"));
    Serial.print(Ethernet.localIP());
    Serial.println(F(":2560"));
    Serial.println();
    Serial.println();
    headerNotes();

    memset(buffer, 0, sizeof(buffer));
    bPointer = buffer;
    memset(bufferOfBytes, 0, sizeof(bufferOfBytes));
    bPointerOfBytes = bufferOfBytes;

    Serial.println(F("Let's do this"));
    Serial.println();
    Serial.print(F("\n Free RAM = "));
    Serial.println(freeRam());
    Serial.println();

    digitalWrite(13, LOW);
}
time_t prevDisplay = 0;      // when the digital clock was displayed NTP STUFF, HERE FOR SOME REASON
void (*resetFunc)(void) = 0; //declare reset function at address 0
void veraSendVswitch(byte veraVswitchID, byte veraVswitchVal)
{
    if (vera.connect(veraServer, 3480))
    {
        delay(250);
        vera.print(F("GET /data_request?id=lu_action&output_format=json&serviceId=urn:upnp-org:serviceId:VSwitch1&DeviceNum="));
        vera.print(veraVswitchID);
        vera.print(F("&action=SetTarget&newTargetValue="));
        vera.print(veraVswitchVal);
        vera.println(F(" HTTP/1.1"));
        vera.println(F("Host: www.sdyoung.com"));
        vera.println();
        delay(100);
        vera.println();
        //    veraSuccess = true;
        Serial.println(F("\n Updated datagram sent to Vera multistring app"));
    }
    else
    {
        //    veraSuccess = false;
        //    Serial.println(F("\n ### FAILED POSTING VIRTUAL SWITCH UPDATE TO VERA.SDYOUNG.COM ###\r\n"));
        resetFunc();
    }
    vera.stop();
    while (vera.status() != 0)
    {
        delay(5);
    }
}

void veraSendMultiString(byte veraMstringID, int veraMstringVal)
{
    if (vera.connect(veraServer, 3480))
    {
        delay(250);
        vera.print(F("GET /data_request?id=variableset&DeviceNum=33&serviceId=urn:upnp-org:serviceId:VContainer1&Variable=Variable"));
        vera.print(veraMstringID);
        vera.print(F("&Value="));
        vera.print(veraMstringVal);
        vera.println(F(" HTTP/1.1"));
        vera.println(F("Host: www.sdyoung.com"));
        vera.println();
        delay(100);
        vera.println();
        veraSuccess = true;
        //Serial.println(F("\n Data sent to Vera"));
    }
    else
    {
        //    veraSuccess = false;
        Serial.println(F("\n ### FAILED POSTING MULTISTRING UPDATE TO VERA.SDYOUNG.COM ###\r\n"));
        resetFunc();
    }
    vera.stop();
    while (vera.status() != 0)
    {
        delay(5);
    }
}

void loop()
{
    digitalWrite(DTR, LOW);                     // Enable Receiving Data
    EthernetClient client = server.available(); // try to get client
    if (client)
    { // got client?
        boolean currentLineIsBlank = true;
        while (client.connected())
        {
            if (client.available())
            {                           // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                if (req_index < (REQ_BUF_SZ - 1))
                {
                    digitalWrite(13, HIGH);
                    HTTP_req[req_index] = c; // save HTTP request character
                    req_index++;
                    digitalWrite(13, LOW);
                }
                if (c == '\n' && currentLineIsBlank)
                {
                    client.println(F("HTTP/1.1 200 OK")); // send a standard http response header
                    if (StrContains(HTTP_req, "/pool/light/on"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        lightState = 1;                //light is on
                        veraVarDevId = veraVarLight;   //set devID var to 26 for Vera
                        veraVarTargetVal = lightState; //set devID26 value to 1 which is on
                        veraUpdatePending = true;
                        xivelyTrigger = true;
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 2; count++)
                        {
                            for (byte i = 0; i < sizeof(poolLightOn); i++)
                            {
                                Serial1.write(poolLightOn[i]);
                            }
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/light/off"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        lightState = 0;
                        veraVarDevId = veraVarLight;   //set devID var to 26 for Vera
                        veraVarTargetVal = lightState; //set devID26 value to 0 which is on
                        veraUpdatePending = true;
                        xivelyTrigger = true;
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 2; count++)
                        {
                            for (byte i = 0; i < sizeof(poolLightOff); i++)
                            {
                                Serial1.write(poolLightOff[i]);
                            }
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/clean/on"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        cleanerState = 1;
                        veraVarDevId = veraVarCleaner;   //set devID var to 36 for Vera
                        veraVarTargetVal = cleanerState; //set devID36 value to 0 which is on
                        veraUpdatePending = true;
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 2; count++)
                        {
                            for (byte i = 0; i < sizeof(cleanerOn); i++)
                            {
                                Serial1.write(cleanerOn[i]);
                            }
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/clean/off"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        cleanerState = 0;
                        veraVarDevId = veraVarCleaner;   //set devID var to 36 for Vera
                        veraVarTargetVal = cleanerState; //set devID36 value to 0 which is on
                        veraUpdatePending = true;
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 2; count++)
                        {
                            for (byte i = 0; i < sizeof(cleanerOff); i++)
                            {
                                Serial1.write(cleanerOff[i]);
                            }
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/waterfall/on"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        waterfallState = 1;
                        veraVarDevId = veraVarWaterfall;   //set devID var to 34 for Vera
                        veraVarTargetVal = waterfallState; //set devID34 value to 1 which is on
                        veraUpdatePending = true;
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 2; count++)
                        {
                            for (byte i = 0; i < sizeof(waterFallOn); i++)
                            {
                                Serial1.write(waterFallOn[i]);
                            }
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/waterfall/off"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        waterfallState = 0;
                        veraVarDevId = veraVarWaterfall;   //set devID var to 34 for Vera
                        veraVarTargetVal = waterfallState; //set devID34 value to 0 which is on
                        veraUpdatePending = true;
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 2; count++)
                        {
                            for (byte i = 0; i < sizeof(waterFallOff); i++)
                            {
                                Serial1.write(waterFallOff[i]);
                            }
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/circulate/on"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        circulationState = 1;
                        veraVarDevId = veraVarCirculation;   //set devID var to 35 for Vera
                        veraVarTargetVal = circulationState; //set devID35 value to 0 which is on
                        veraUpdatePending = true;
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 2; count++)
                        {
                            for (byte i = 0; i < sizeof(poolOn); i++)
                            {
                                Serial1.write(poolOn[i]);
                            }
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/circulate/off"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        circulationState = 0;
                        veraVarDevId = veraVarCirculation;   //set devID var to 35 for Vera
                        veraVarTargetVal = circulationState; //set devID35 value to 0 which is on
                        veraUpdatePending = true;
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 2; count++)
                        {
                            for (byte i = 0; i < sizeof(poolOff); i++)
                            {
                                Serial1.write(poolOff[i]);
                            }
                        }
                    }
                    else if (StrContains(HTTP_req, "/xively/update"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        xivelyTrigger = true;
                    }
                    else if (StrContains(HTTP_req, "/pool/chlorinator/getsetpoint"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        client.print(F("[{\"chlorSetpoint\":\""));
                        client.print(saltSetpoint);
                        client.print(F("\"}]"));
                    }
                    else if (StrContains(HTTP_req, "/pool/chlorinator/getsalinity"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        client.print(F("[{\"currentSalinityPPM\":\""));
                        client.print(salinityNow);
                        client.print(F("\"}]"));
                    }
                    else if (StrContains(HTTP_req, "/pool/chlorinator/error"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        client.print(F("[{\"chlorError\":\""));
                        client.print(saltStateResult);
                        client.print(F("\"}]"));
                    }
                    else if (StrContains(HTTP_req, "/pool/water/temp"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        client.print(F("[{\"waterTemp\":\""));
                        client.print(poolTemp);
                        client.print(F("\"}]"));
                    }
                    else if (StrContains(HTTP_req, "/pool/pump/rpm"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        client.print(F("[{\"pumpRPM\":\""));
                        client.print(pumpRPM);
                        client.print(F("\"}]"));
                    }
                    else if (StrContains(HTTP_req, "/pool/pump/watts"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        client.print(F("[{\"pumpWatts\":\""));
                        client.print(pumpWatts);
                        client.print(F("\"}]"));
                    }
                    else if (StrContains(HTTP_req, "/pool/light/state"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        client.print(F("[{\"lightState\":\""));
                        client.print(lightState);
                        client.print(F("\"}]"));
                    }
                    else if (StrContains(HTTP_req, "/avr/uptime"))
                    {
                        rawUptime = (millis() / 1000);
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        client.print(F("[{\"rawUptimeInSecs\":\""));
                        client.print(rawUptime);
                        client.print(F("\"}]"));
                    }
                    else if (StrContains(HTTP_req, "/pool/chlorinator/set/1"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 8; count++)
                        {
                            for (byte i = 0; i < sizeof(chlorStep1); i++)
                            {
                                Serial1.write(chlorStep1[i]);
                            }
                            delay(5);
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/chlorinator/set/2"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 8; count++)
                        {
                            for (byte i = 0; i < sizeof(chlorStep2); i++)
                            {
                                Serial1.write(chlorStep2[i]);
                            }
                            delay(5);
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/chlorinator/set/3"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 8; count++)
                        {
                            for (byte i = 0; i < sizeof(chlorStep3); i++)
                            {
                                Serial1.write(chlorStep3[i]);
                            }
                            delay(5);
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/chlorinator/set/4"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 8; count++)
                        {
                            for (byte i = 0; i < sizeof(chlorStep4); i++)
                            {
                                Serial1.write(chlorStep4[i]);
                            }
                            delay(5);
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/chlorinator/set/5"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 8; count++)
                        {
                            for (byte i = 0; i < sizeof(chlorStep5); i++)
                            {
                                Serial1.write(chlorStep5[i]);
                            }
                            delay(5);
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/chlorinator/set/6"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 8; count++)
                        {
                            for (byte i = 0; i < sizeof(chlorStep6); i++)
                            {
                                Serial1.write(chlorStep6[i]);
                            }
                            delay(5);
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/chlorinator/set/7"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 8; count++)
                        {
                            for (byte i = 0; i < sizeof(chlorStep7); i++)
                            {
                                Serial1.write(chlorStep7[i]);
                            }
                            delay(5);
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/chlorinator/set/8"))
                    {
                        client.println(F("Content-Type: text/plain"));
                        client.println(F("Connection: close"));
                        client.println();
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 8; count++)
                        {
                            for (byte i = 0; i < sizeof(chlorStep8); i++)
                            {
                                Serial1.write(chlorStep8[i]);
                            }
                            delay(5);
                        }
                    }
                    else if (StrContains(HTTP_req, "/pool/status"))
                    {
                        rawUptime = (millis() / 1000);
                        client.println(F("HTTP/1.1 200 OK"));
                        client.println(F("Content-Type: text/html"));
                        client.println(F("Connnection: close"));
                        client.println();
                        client.println(F("<!DOCTYPE html>"));
                        client.println(F("<html>"));
                        client.println(F("<head>"));
                        client.println(F("<style>p {font-family: \"Lucida Console\", \"Lucida Sans Typewriter\", monaco, \"Bitstream Vera Sans Mono\", monospace; font-size: 16px;  font-style: normal;  font-variant: normal;  font-weight: 400;  line-height: 20px; margin:0px;} </style>"));
                        client.println(F("<title>Pool Params</title>"));
                        client.println(F("<meta http-equiv=\"refresh\" content=\"5\">"));
                        client.println(F("</head>"));
                        client.println(F("<body>"));

                        client.print(F("<p>"));
                        client.println(s1);
                        client.print(F("</p>"));
                        client.print(F("<p>Pump state........ "));
                        if (circulationState == 0 && waterfallState == 0 && cleanerState == 0)
                            client.print(F("Off</p>"));
                        if (circulationState > 0 || waterfallState > 0 || cleanerState > 0)
                            client.print(F("On</p>"));
                        client.print(F("<p>Pump RPM.......... "));
                        client.print(pumpRPM);
                        client.print(F("<p>Pump watts........ "));
                        client.print(pumpWatts);
                        client.print(F("<p>Pool temp......... "));
                        client.print(poolTemp);
                        client.print((char)176);
                        if (circulationState == 0 && waterfallState == 0 && cleanerState == 0)
                        {
                            client.print(F("F (plumbing temp)</p>"));
                        }
                        else
                        {
                            client.print(F("F</p>"));
                        }

                        client.print(F("<p>Air temp.......... "));
                        client.print(airTemp);
                        client.print((char)176);
                        client.print(F("F</p>"));

                        client.print(F("<p>Waterfall state... "));
                        if (waterfallState == 0)
                            client.print(F("Off</p>"));
                        if (waterfallState == 1)
                            client.print(F("On</p>"));

                        client.print(F("<p>Cleaner state..... "));
                        if (cleanerState == 0)
                            client.print(F("Off</p>"));
                        if (cleanerState == 1)
                            client.print(F("On</p>"));

                        client.print(F("<p>Light state....... "));
                        if (lightState == 0)
                            client.print(F("Off</p>"));
                        if (lightState == 1)
                            client.print(F("On</p>"));

                        if (salinityNow <= 3800)
                            client.print(F("<p>Water salinity.... "));
                        if (salinityNow >= 3801 && salinityNow <= 4200)
                            client.print(F("<p style=\"background-color:yellow;\"> Water salinity.... "));
                        if (salinityNow >= 4201)
                            client.print(F("<p style=\"background-color:red;\">Water salinity.... "));
                        client.print(salinityNow);
                        client.print(F(" PPM</p>"));

                        client.print(F("<p>Salt cell setpoint "));
                        client.print(saltSetpoint);
                        client.print(F("%</p>"));
                        client.print(F("<p>Salt cell output.. "));
                        client.print(saltPct);
                        if (saltPct > saltSetpoint)
                        {
                            client.print(F("% - SUPERCHLOR ACTIVE"));
                        }
                        else if (saltPct < saltSetpoint && (circulationState > 0 || waterfallState > 0 || cleanerState > 0))
                        {
                            client.print(F("% - ON, IDLE DWELL"));
                        }
                        else if (saltPct == saltSetpoint)
                        {
                            client.print(F("% - NORMAL OPERATION"));
                        }
                        else
                        {
                            client.print(F("% - PUMP IS IDLE"));
                        }
                        client.print(F("</p>"));
                        client.print(F("<p>Salt cell state... "));
                        if (saltStateResult == 0x0)
                        {
                            client.print(F("RUNNING - NO ERRORS</p>"));
                        }
                        else if (saltStateResult == 0x1)
                        {
                            client.print(F("COMM ERROR</p>"));
                        }
                        else if (saltStateResult == 0x2)
                        {
                            client.print(F("RUNNING - LOW FLOW</p>"));
                        }
                        else if (saltStateResult == 0x3)
                        {
                            client.print(F("DISABLED - PUMP OFF</p>"));
                        }
                        else if (saltStateResult == 0xF0)
                        {
                            client.print(F("PENDING QUERY RESPONSE</p>"));
                        }
                        else
                        {
                            client.print(saltStateResult);
                            client.print(F("</p>"));
                        }

                        client.print(F("<p>Panel time........ "));
                        client.print(panelHour);
                        client.print(F(":"));
                        if (panelMinute < 10)
                        {
                            client.print(F("0"));
                        }
                        client.print(panelMinute);
                        client.print(F("</p>"));

                        client.print(F("<p>NTP real time..... "));
                        client.print(ntpHours);
                        client.print(F(":"));
                        if (ntpMinutes < 10)
                        {
                            client.print(F("0"));
                        }
                        client.print(ntpMinutes);
                        client.print(F("</p>"));

                        client.print(F("<p>AVR uptime........ "));
                        uptimeHttp();
                        client.print(strUptime);
                        client.print(F("</p>"));
                        strUptime = "";

                        client.print(F("\n<br/>"));

                        client.print(F("<p><a href=\"https://personal.xively.com/feeds/805849745\" target=\"_blank\">https://personal.xively.com/feeds/805849745</a></p>"));
                        client.print(F("<p><a href=\"../../avr/api/usage\" target=\"_blank\">HTTP API command usage</a></p>"));

                        client.print(F("</body>"));
                        client.print(F("</html>"));
                    }
                    else if (StrContains(HTTP_req, "/avr/api/usage"))
                    {
                        client.println(F("HTTP/1.1 200 OK"));
                        client.println(F("Content-Type: text/html"));
                        client.println(F("Connnection: close"));
                        client.println();
                        client.println(F("<!DOCTYPE html>"));
                        client.println(F("\n<html>"));
                        client.println(F("\n<head>"));
                        client.println(F("\n<style>p {font-family: \"Lucida Console\", \"Lucida Sans Typewriter\", monaco, \"Bitstream Vera Sans Mono\", monospace; font-size: 16px;  font-style: normal;  font-variant: normal;  font-weight: 400;  line-height: 20px; margin:0px;} </style>"));
                        client.println(F("\n<title>Pool API Usage</title>"));
                        client.println(F("\n<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">"));
                        client.println(F("\n</head>"));
                        client.println(F("\n<body>"));

                        client.print(F("<p>"));
                        client.println(s1);
                        client.print(F("</p>"));

                        client.print(F("\n<p> Most commands should be self-explanatory.  Ambiguous commands have notes.</p>"));
                        client.print(F("\n<br/>"));
                        client.print(F("\n<p> /pool/light/on</p>"));
                        client.print(F("\n<p> /pool/light/off</p>"));
                        client.print(F("\n<p> /pool/clean/on <--------------------sets pump to in-floor cleaner RPM</p>"));
                        client.print(F("\n<p> /pool/clean/off</p>"));
                        client.print(F("\n<p> /pool/waterfall/on</p>"));
                        client.print(F("\n<p> /pool/waterfall/off</p>"));
                        client.print(F("\n<p> /pool/circulate/on <----------------sets pump to low speed circulation</p>"));
                        client.print(F("\n<p> /pool/circulate/off</p>"));
                        client.print(F("\n<p> /pool/chlorinator/set/X <-----------where X is 1-8 to change chlor setpoint</p>"));
                        client.print(F("\n<p> /pool/chlorinator/getsetpoint <-----returns JSON formatted setpoint</p>"));
                        client.print(F("\n<p> /pool/chlorinator/getsalinity <-----returns JSON formatted salinity</p>"));
                        client.print(F("\n<p> /pool/chlorinator/error <-----------returns JSON formatted error level</p>"));
                        client.print(F("\n<p> /pool/water/temp <------------------returns JSON formatted water temp</p>"));
                        client.print(F("\n<p> /pool/pump/rpm <--------------------returns JSON formatted pump RPM</p>"));
                        client.print(F("\n<p> /pool/pump/watts <------------------returns JSON formatted pump watts</p>"));
                        client.print(F("\n<p> /pool/light/state <-----------------returns JSON formatted light status</p>"));
                        client.print(F("\n<p> /pool/status <----------------------returns HTML formatted pool summary</p>"));
                        client.print(F("\n<p> /avr/uptime <-----------------------returns JSON formatted arduino uptime</p>"));
                        client.print(F("\n<p> /avr/api/usage <--------------------returns HTML formatted HTTP API calls</p>"));
                        client.print(F("\n<p> /xively/update <--------------------manually force Xively update</p>"));
                        client.print(F("\n<br/>"));
                        client.print(F("\n<p><a href=\"../../pool/status\">Pool Status</a></p>"));

                        client.print(F("\n</body>"));
                        client.print(F("\n</html>"));
                    }

                    // display received HTTP request on serial port
                    Serial.print(F("\r\nHTTP --> "));
                    Serial.println(HTTP_req);
                    delay(20);
                    // reset buffer index and all buffer elements to 0
                    req_index = 0;
                    StrClear(HTTP_req, REQ_BUF_SZ);
                    if (debug == true)
                        Serial.println(F("step 1"));
                    break;
                }
                if (c == '\n')
                {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                }
                else if (c != '\r')
                {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            }          // end if (client.available())
        }              // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    }                  // end if (client)

    if (saltSetpointTrigger == true)
    {
        digitalWrite(DTR, HIGH);
        for (byte count = 0; count < 1; count++)
        { //whenever update Xively occurs, query the chlorinator setpoint
            for (byte i = 0; i < sizeof(saltPctQuery); i++)
            {
                Serial1.write(saltPctQuery[i]);
                Serial.print((saltPctQuery[i]), HEX);
            }
        }
        saltSetpointTrigger = false;
    }

    char c;
    while (Serial1.available())
    {
        c = (uint8_t)Serial1.read();
        switch (goToCase)
        {
        case header1:
            if (c == 0xFFFFFFFF)
            { // ignoring leading FF so do nothing, repeat again
                *bPointer++ = (char)c;
                byteNum = 1;
                if (debug == true)
                    Serial.println(F("step 2"));
                break;
            }
            else if (c == 0x0)
            { // is this a 0 in byte 2?  could be Pentair packet
                goToCase = header3;
                *bPointer++ = (char)c;
                byteNum++;
                if (debug == true)
                    Serial.println(F("step 3"));
                break;
            }
            else
            { //if (c == 0x10)                              // is this an IntelliChlor header?  could be an IntelliChlor packet
                goToCase = saltHead2;
                *bPointer++ = (char)c;
                byteNum = 1;
                if (debug == true)
                    Serial.println(F("step 4"));
                break;
            }
            if (debug == true)
                Serial.println(F("step 5"));
            break;

        case header3:
            *bPointer++ = (char)c;
            if (c == 0xFFFFFFFF)
            { // it's not really the start of a frame, must be deeper into a Pentair packet
                goToCase = header4;
                byteNum++;
                if (debug == true)
                    Serial.println(F("step 6"));
                break;
            }
            else
            {
                clear485Bus();
                goToCase = header1;
                if (debug == true)
                    Serial.println(F("step 7"));
                break;
            }
            if (debug == true)
                Serial.println(F("step 8"));
            break;

        case header4:
            if (c == 0xFFFFFFA5)
            { // it's not really the start of a frame, almost have a Pentair preamble match
                goToCase = bufferData;
                sumOfBytesInChkSum += (byte)c, HEX;
                *bPointerOfBytes++ = (char)c;
                *bPointer++ = (char)c;
                byteNum++;
                if (debug == true)
                    Serial.println(F("step 9"));
                break;
            }
            else
            {
                clear485Bus();
                goToCase = header1;
                if (debug == true)
                    Serial.println(F("step 10"));
                break;
            }
            if (debug == true)
                Serial.println(F("step 11"));
            break;

        case bufferData:
            *bPointer++ = (char)c;        // loop until byte 9 is seen
            *bPointerOfBytes++ = (char)c; // add up in the checksum bytes
            byteNum++;
            sumOfBytesInChkSum += (byte)c, HEX;
            if (1 != 2)
            { // janky code here... whatever.  you clean it up mr. awesome
                if (byteNum == 9)
                { // get data payload length of bytes
                    bytesOfDataToGet = (c);
                    Serial.println();
                    Serial.println();
                    Serial.print(F("\n Free RAM = "));
                    Serial.print(freeRam());
                    Serial.println(F(" <-- watch for memory leaks"));
                    Serial.println();
                    Serial.println();
                    digitalClockDisplay();
                    uptime();
                    Serial.println(F("NEW RS-485 FRAMES RECEIVED"));
                    Serial.print(F("Payload bytes to get... "));
                    Serial.println(bytesOfDataToGet);
                    if (bytesOfDataToGet < 0 || bytesOfDataToGet > 47)
                    { //uh oh.....buffer underflow or buffer overflow... Time to GTFO
                        clear485Bus();
                        if (debug == true)
                            Serial.println(F("step 12"));
                        break;
                    }
                    if (remainingBytes == bytesOfDataToGet)
                    {
                        goToCase = calcCheckSum;
                        if (debug == true)
                            Serial.println(F("step 13"));
                        break;
                    }
                    remainingBytes++;
                    if (debug == true)
                        Serial.println(F("step 14"));
                    break;
                }
                if (byteNum >= 10)
                {
                    if (remainingBytes == bytesOfDataToGet)
                    {
                        goToCase = calcCheckSum;
                        if (debug == true)
                            Serial.println(F("step 15"));
                        break;
                    }
                    remainingBytes++;
                    if (debug == true)
                        Serial.println(F("step 16"));
                    break;
                }
                if (debug == true)
                    Serial.println(F("step 17"));
                break;
            }
            if (debug == true)
                Serial.println(F("step 18"));
            break;

        case calcCheckSum:
            if (chkSumBits < 2)
            {
                *bPointer++ = (char)c;
                if (chkSumBits == 0)
                {
                    Serial.print(F("Checksum high byte..... "));
                    Serial.println(c, HEX);
                    chkSumValue = (c * 256);
                }
                else if (chkSumBits == 1)
                {
                    Serial.print(F("Checksum low byte...... "));
                    Serial.println((byte)c, HEX);
                    goToCase = header1;
                    byte len = (byte)(bPointer - buffer);
                    chkSumValue += (byte)c;
                    printFrameData(buffer, len);
                    clear485Bus();
                    if (debug == true)
                        Serial.println(F("step 19"));
                    break;
                }
                chkSumBits++;
                if (debug == true)
                    Serial.println(F("step 20"));
                break;
            }
            if (debug == true)
                Serial.println(F("step 21"));
            break;

        case saltHead2:
            if (c == 0x02)
            { // is this Intellichlor STX header frame 2 ?
                goToCase = bufferSaltData;
                *bPointer++ = (char)c;
                byteNum++;
                if (debug == true)
                    Serial.println(F("step 22"));
                break;
            }
            else
            {
                clear485Bus();
                goToCase = header1;
                if (debug == true)
                    Serial.println(F("step 23"));
                break;
            }
            if (debug == true)
                Serial.println(F("step 24"));
            break;

        case bufferSaltData:
            if (c != 0x10)
            {
                *bPointer++ = (char)c; // loop until byte value 0x10 is seen
                byteNum++;
                if (debug == true)
                    Serial.println(F("step 25"));
                break;
            }
            else
            { // a ha! found a 0x10, we're close
                goToCase = saltTerm;
                *bPointer++ = (char)c;
                byteNum++;
                if (debug == true)
                    Serial.println(F("step 26"));
                break;
            }
            if (debug == true)
                Serial.println(F("step 27"));
            break;

        case saltTerm:
            *bPointer++ = (char)c;
            byteNum++;
            goToCase = header1;
            if (c != 0x03)
            {
                clear485Bus();
                if (debug == true)
                    Serial.println(F("step 28"));
                break;
            }
            else
            { // found an ETX 0x3.  See what we've got
                byte len = (byte)(bPointer - buffer);
                Serial.println();
                Serial.println();
                digitalClockDisplay();
                Serial.println(F("NEW RS-485 IntelliChlor FRAMES RECEIVED"));
                if (len == 8)
                {
                    saltBytes1 = (buffer[2] + buffer[3] + buffer[4] + 18);
                    Serial.print(F("Short salt byte sum +18. "));
                    Serial.println(saltBytes1);
                    Serial.print(F("Short Salt checksum is.. "));
                    Serial.println(buffer[5]);
                    if (saltBytes1 == buffer[5])
                    {
                        salt = true;
                        oldSaltPct = saltPct;
                        saltPct = buffer[4];
                        Serial.println(F("Checksum is............. GOOD"));
                        Serial.print(F("Chlorinator Load........ "));
                        Serial.print(saltPct);
                        Serial.println(F("%"));
                        //Serial.println();
                        printFrameData(buffer, len);
                        //if (oldSaltPct != saltPct) { //disabled b/c when idle the output doesn't toggle, but provides interval updates anyway for other vars
                        //saltSetpointTrigger = true;
                        xivelyTrigger = true;
                        saltOutputToggle++;
                        //}
                    }
                    else
                    {
                        Serial.println(F("Checksum is............. INVALID"));
                        salt = true;
                        printFrameData(buffer, len);
                        //Serial.println();
                    }
                    clear485Bus();
                    if (debug == true)
                        Serial.println(F("step 29"));
                    break;
                }
                else
                {
                    saltBytes2 = (buffer[2] + buffer[3] + buffer[4] + buffer[5] + 18);
                    Serial.print(F("Long salt byte sum +18.. "));
                    Serial.println(saltBytes2);
                    Serial.print(F("Long Salt checksum is... "));
                    Serial.println(buffer[6]);
                    if (saltBytes2 == buffer[6])
                    {
                        salt = true;
                        Serial.println(F("Checksum is............. GOOD"));
                        printFrameData(buffer, len);
                        //someVal = buffer[6];
                        digitalWrite(DTR, HIGH);
                        for (byte count = 0; count < 2; count++)
                        { //whenever the long salt string checksum checks out, query the chlorinator setpoint
                            for (byte i = 0; i < sizeof(saltPctQuery); i++)
                            {
                                Serial1.write(saltPctQuery[i]);
                                //Serial.print((byte)saltPctQuery[i], HEX);
                            }
                        }
                        salinityNow = (buffer[4] * 50);
                    }
                    else
                    {
                        Serial.println(F("Checksum is............. INVALID"));
                        salt = true;
                        printFrameData(buffer, len);
                        //Serial.println();
                    }
                    clear485Bus();
                    if (debug == true)
                        Serial.println(F("step 30"));
                    break;
                }
                if (debug == true)
                    Serial.println(F("step 31"));
                break;
            }
            clear485Bus();
            if (debug == true)
                Serial.println(F("step 32"));
            break;
        } // end switch( goToCase )
    }     // while serial available

    //these IF statements update the Vera HA unit with status changes.  May need to slow these down if lock-ups occur.
    if (veraUpdatePending == true)
    { //updates the Vera virtual switches from an HTTP call
        veraUpdatePending = false;
        veraSendVswitch(veraVarDevId, veraVarTargetVal);
    }
    if (oldPoolMode != poolMode)
    { //updates the Vera virtual switches from Pentair remote or console panel
        oldPoolMode = poolMode;
        for (byte x = 0; x < sizeof(veraVarVals); x++)
        {
            veraSendVswitch(veraVarIDs[x], veraVarVals[x]);
        }
    }
    if (oldSaltSetpoint != saltSetpoint)
    {
        oldSaltSetpoint = saltSetpoint;
        //veraUpdateCount++;
        veraSendMultiString(veraMstringChlor, saltSetpoint);
    }
    if (oldPoolTemp != poolTemp && pumpMode > 0)
    {
        oldPoolTemp = poolTemp;
        //veraUpdateCount++;
        veraSendMultiString(veraMstringWaterTemp, poolTemp);
    }
    if (oldPumpRPM != pumpRPM)
    {
        oldPumpRPM = pumpRPM;
        //veraUpdateCount++;
        veraSendMultiString(veraMstringPumpRPM, pumpRPM);
    }
    if (oldPumpWatts != pumpWatts)
    {
        oldPumpWatts = pumpWatts;
        //veraUpdateCount++;
        veraSendMultiString(veraMstringPumpWatts, pumpWatts);
    }
    if (oldSaltPct != saltPct)
    {
        oldSaltPct = saltPct;
        //veraUpdateCount++;
        veraSendMultiString(veraMstringChlorOn, saltPct);
    }

    //  if (finalXivelyPost == false) { //update Xively after transition from on to IDLE so values go to 0
    //    if (poolMode == 0x0 && saltOutputToggle > 2) {  //pool idle but has run
    //      finalXivelyPost = true;
    //      xivelyMillis = millis();
    //    }
    //    if (poolMode == 0x2 || poolMode == 0x4 || poolMode == 0x6) {  //pool running but not generating salt
    //      finalXivelyPost = true;
    //      xivelyMillis = millis();
    //      saltOutputToggle = 3; //just need to force it above threshold of 2
    //    }
    //  }
    //
    ////  if (poolMode == 0x2 || poolMode == 0x4 || poolMode == 0x6) { //update Xively during light (no pump), waterfall & waterfall+light because chlorinator isn't generating in this mode
    ////    finalXivelyPost = true;
    ////    xivelyMillis = millis();
    ////  }
    //
    ////  if (((millis() - xivelyMillis) > 29995) && ((millis() - xivelyMillis) < 30005) && finalXivelyPost == true) {    //wait 30 seconds for all variables to update
    ////    xivelyMillis = millis();
    ////    Serial.print(F("30303030303030303030303030303030303030303030"));
    ////    //xivelyTrigger = true;
    ////  }
    //
    //  if (finalXivelyPost == true) {
    //    if (poolMode != 0x2 && poolMode != 0x4 && poolMode != 0x6) {
    //      finalXivelyPost = false; //pool back on, posted final to Xively, re-arm final post to Xively
    //    }
    //    if (millis() - xivelyMillis > 29000) {
    //      xivelyMillis = millis();
    //      xivelyTrigger = true;
    //    }
    //  }

    if (xivelyTrigger == true)
    {
        xivelyTrigger = false;
        noInterrupts(); //disabled 3-16-16 >> re-enabled 5-18-16
        //delay(5);
        if (saltOutputToggle > 2)
        {
            sendToRasPi();
        }
        //xivelyPost();   //disabled 5-20-16 renabled 6-1-16 disabled 6-4-16
        //delay(5);
        //xivelyStatus(); //disabled 5-20-16 renabled 6-1-16 disabled 6-4-16
        //delay(5);
        veraPost();   //re-enabled 5-18-16
                      //delay(5);
        interrupts(); //disabled 3-16-16 >> re-enabled 5-18-16
        saltSetpointTrigger = true;
        //      digitalWrite(DTR, HIGH);
        //      for (byte count = 0; count < 1; count++) {  //whenever update Xively occurs, query the chlorinator setpoint
        //        for(byte i = 0; i < sizeof(saltPctQuery); i++) {
        //          Serial1.write(saltPctQuery[i]);
        //          //Serial.print((saltPctQuery[i]),HEX);
        //        }
        //        //delay(5);
        //      }
    }
    currentMillis = millis();
} // end void loop

void veraPost()
{
    //  if (oldSaltSetpoint != saltSetpoint) {
    //    oldSaltSetpoint = saltSetpoint;
    //    //veraUpdateCount++;
    //    veraSendMultiString(veraMstringChlor, saltSetpoint);
    //  }
    //  if (oldPoolTemp != poolTemp && pumpMode > 0) {
    //    oldPoolTemp = poolTemp;
    //    //veraUpdateCount++;
    //    veraSendMultiString(veraMstringWaterTemp, poolTemp);
    //  }
    //  if (oldPumpRPM != pumpRPM) {
    //    oldPumpRPM = pumpRPM;
    //    //veraUpdateCount++;
    //    veraSendMultiString(veraMstringPumpRPM, pumpRPM);
    //  }
    //  if (oldPumpWatts != pumpWatts) {
    //    oldPumpWatts = pumpWatts;
    //    //veraUpdateCount++;
    //    veraSendMultiString(veraMstringPumpWatts, pumpWatts);
    //  }
    //  if (oldSaltPct != saltPct) {
    //    oldSaltPct = saltPct;
    //    //veraUpdateCount++;
    //    veraSendMultiString(veraMstringChlorOn, saltPct);
    //  }
}

void clear485Bus()
{
    memset(buffer, 0, sizeof(buffer));
    bPointer = buffer;
    memset(bufferOfBytes, 0, sizeof(bufferOfBytes));
    bPointerOfBytes = bufferOfBytes;
    byteNum = 0;
    bytesOfDataToGet = 0;
    remainingBytes = 0;
    chkSumBits = 0;
    saltBytes1 = 0;
    saltBytes2 = 0;
    salt = false;
    sumOfBytesInChkSum = 0;
    chkSumValue = 0;
}

void sendToRasPi()
{
    //analogWrite(A8,255);
    //delay(250); //This one keeps it from hanging

    if (php.connect(phpServer, 8080))
    {
        phpStart = millis();
        Serial.print(F("Connected to RasPi >> "));
        php.print(F("GET /xivelyPool?poolTemp="));
        php.print(poolTemp);
        php.print(F("&airTemp="));
        php.print(airTemp);
        php.print(F("&poolMode="));
        php.print(poolMode);
        php.print(F("&pumpMode="));
        php.print(pumpMode);
        php.print(F("&pumpWatts="));
        php.print(pumpWatts);
        php.print(F("&pumpRPM="));
        php.print(pumpRPM);
        php.print(F("&chlorOutput="));
        php.print(saltPct);
        php.print(F("&chlorSetpoint="));
        php.print(saltSetpoint);
        php.print(F("&lightState="));
        php.print(lightState);
        php.print(F("&chlorError="));
        php.print(saltStateResult);
        if (salinityNow > 0)
        { //prevents posting 0ppm to Xively
            php.print(F("&salinity="));
            php.print(salinityNow);
        }
        php.println(F(" HTTP/1.1"));
        php.println(F("Host: www.sdyoung.com"));
        php.println();
        Serial.print(F("### SUCCESSFULLY POSTED TO RASPBERRYPI.SDYOUNG.COM IN "));
        //delay(100);
    }
    else
    {
        Serial.println(F("### FAILED POSTING TO RASPBERRYPI.SDYOUNG.COM ###\r\n"));
    }
    //stop client
    php.stop();
    while (php.status() != 0)
    {
        delay(5);
    }
    //analogWrite(A8,0);
    phpStop = millis();
    Serial.print(phpStop - phpStart);
    Serial.println(F("ms ###"));
}

void printFrameData(uint8_t *buffer, byte len)
{
    int i = 0;
    if (salt == false)
    { // salt does it's own check sum calculations
        Serial.print(F("Sum of bytes........... "));
        Serial.println(sumOfBytesInChkSum);
        Serial.print(F("Check sum is........... "));
        Serial.println(chkSumValue);
        if (sumOfBytesInChkSum == chkSumValue)
        {
            Serial.println(F("Check sum result....... GOOD"));
        }
        else
        {
            Serial.println(F("Check sum result....... INVALID"));
        }
    }

    int lenChkSumValue = (int)(bPointerOfBytes - bufferOfBytes);
    while (i < len)
    {
        printByteData(buffer[i++]); // Dumps the bytes to the serial mon
        Serial.print(F(" "));
    }
    Serial.println();
    if (sumOfBytesInChkSum == chkSumValue)
    {
        if (bufferOfBytes[5] == 0x1D)
        { // 29 byte message is for broadcast display updates
            oldPoolTemp = poolTemp;
            poolTemp = bufferOfBytes[20];
            airTemp = bufferOfBytes[24];
            oldPoolMode = poolMode;
            poolMode = bufferOfBytes[8];
            panelHour = bufferOfBytes[6];
            panelMinute = bufferOfBytes[7];
            Serial.print(F("Water Temp............. "));
            Serial.print(poolTemp);
            Serial.print((char)176);
            Serial.println(F("F"));
            Serial.print(F("Air Temp............... "));
            Serial.print(airTemp);
            Serial.print((char)176);
            Serial.println(F("F"));
            Serial.print(F("Panel time............. "));
            if (panelHour < 10)
            {
                //printByteData(panelHour)
            }
            Serial.print(panelHour);
            Serial.print(F(":"));
            if (panelMinute < 10)
            {
                //printByteData(panelMinute);
            }
            Serial.println(panelMinute);
            Serial.print(F("Pool Mode.............. "));
            if (poolMode == 0x1)
            {
                Serial.println(F("CLEANER"));
                lightState = 0;
                waterfallState = 0;
                circulationState = 0;
                cleanerState = 1;
            }
            else if (poolMode == 0x2)
            {
                Serial.println(F("LIGHT ON"));
                lightState = 1;
                waterfallState = 0;
                circulationState = 0;
                cleanerState = 0;
            }
            else if (poolMode == 0x3)
            {
                Serial.println(F("CLEANER & LIGHT ON"));
                lightState = 1;
                waterfallState = 0;
                circulationState = 0;
                cleanerState = 1;
            }
            else if (poolMode == 0x4 || poolMode == 0x24)
            {
                Serial.println(F("WATERFALL"));
                lightState = 0;
                waterfallState = 1;
                circulationState = 0;
                cleanerState = 0;
            }
            else if (poolMode == 0x5)
            {
                Serial.println(F("CLEANER & WATERFALL"));
                lightState = 0;
                waterfallState = 1;
                circulationState = 0;
                cleanerState = 1;
            }
            else if (poolMode == 0x6 || poolMode == 0x26)
            {
                Serial.println(F("WATERFALL & LIGHT ON"));
                lightState = 1;
                waterfallState = 1;
                circulationState = 0;
                cleanerState = 0;
            }
            else if (poolMode == 0x7)
            {
                Serial.println(F("CLEANER, WATERFALL & LIGHT ON"));
                lightState = 1;
                waterfallState = 1;
                circulationState = 0;
                cleanerState = 1;
            }
            else if (poolMode == 0x20)
            {
                Serial.println(F("CIRCULATION"));
                lightState = 0;
                waterfallState = 0;
                circulationState = 1;
                cleanerState = 0;
            }
            else if (poolMode == 0x22)
            {
                Serial.println(F("CIRCULATION & LIGHT ON"));
                lightState = 1;
                waterfallState = 0;
                circulationState = 1;
                cleanerState = 0;
            }
            else
            {
                Serial.println(F("IDLE"));
                lightState = 0;
                waterfallState = 0;
                circulationState = 0;
                cleanerState = 0;
            }
        }
        else if (bufferOfBytes[2] == 0x10 && bufferOfBytes[3] == 0x60 && bufferOfBytes[5] == 0xF)
        { // 15 byte message is for pump updates
            oldPumpWatts = pumpWatts;
            oldPumpRPM = pumpRPM;
            Serial.print(F("Pump Watts............. "));
            if (buffer[9] > 0)
            {
                pumpWatts = ((bufferOfBytes[9] * 256) + bufferOfBytes[10]); //high bit
                Serial.println(pumpWatts);
                Serial.print(F("Pump RPM............... "));
                pumpRPM = ((bufferOfBytes[11] * 256) + bufferOfBytes[12]);
                Serial.println(pumpRPM);
            }
            else
            {
                Serial.println(bufferOfBytes[9]); //low bit
                Serial.print(F("Pump RPM............... "));
                pumpRPM = Serial.println((bufferOfBytes[11] * 256) + bufferOfBytes[12]);
            }
            pumpMode = bufferOfBytes[18];
            Serial.print(F("Pump Mode.............. "));
            if (pumpMode == 0x1)
            {
                Serial.println(F("RUN"));
            }
            else if (pumpMode == 0x0B)
            {
                Serial.println(F("PRIMING"));
            }
            else
            {
                Serial.println(F("OFF"));
            }
        }
        else if (bufferOfBytes[2] == 0xF && bufferOfBytes[3] == 0x10 && bufferOfBytes[24] == 0x2D && bufferOfBytes[25] == 0x2D)
        {
            oldSaltSetpoint = saltSetpoint;
            saltSetpoint = bufferOfBytes[7];
            salinityNow = (bufferOfBytes[9] * 50);
            Serial.print(F("Chlorinator setpoint... "));
            Serial.print(saltSetpoint);
            Serial.println(F("%"));
            Serial.print(F("Chlorinator errors..... "));
            saltStateLow = bufferOfBytes[8];
            saltStateHigh = bufferOfBytes[10];
            if (saltStateLow == 0x81 && saltStateHigh == 0x81)
            {
                saltStateResult = 0x2;
                Serial.println(F("RUNNING - LOW FLOW"));
            }
            else if (saltStateLow == 0x81 && saltStateHigh == 0x80)
            {
                saltStateResult = 0x0;
                Serial.println(F("RUNNING - NO ERRORS"));
            }
            else if (saltStateLow == 0x80 && saltStateHigh == 0x81)
            {
                saltStateResult = 0x1;
                Serial.println(F("COMM ERROR"));
            }
            else if (saltStateLow == 0x80 && saltStateHigh == 0x80)
            {
                saltStateResult = 0x3;
                Serial.println(F("DISABLED - PUMP OFF"));
            }
        }
        // these are the virtual switches, not the multistring variables
        veraVarVals[0] = lightState;
        veraVarVals[1] = waterfallState;
        veraVarVals[2] = circulationState;
        veraVarVals[3] = cleanerState;
    }
}

// routine to take binary numbers and show them as two bytes hex
void printByteData(uint8_t Byte)
{
    Serial.print((uint8_t)Byte >> 4, HEX);
    Serial.print((uint8_t)Byte & 0x0f, HEX);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48;     // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
    while (Udp.parsePacket() > 0)
        ; // discard any previously received packets
    Serial.println(F("<--- Transmit NTP Request"));
    sendNTPpacket(timeServer);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500)
    {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE)
        {
            Serial.println(F("---> Received NTP Response"));
            Udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 = (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
        }
    }
    Serial.println(F("No NTP Response :-("));
    return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011; // LI, Version, Mode
    packetBuffer[1] = 0;          // Stratum, or type of clock
    packetBuffer[2] = 6;          // Polling Interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}

void digitalClockDisplay()
{
    // digital clock display of the time
    ntpHours = hour();
    ntpMinutes = minute();

    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial.print(F(" "));
    Serial.print(month());
    Serial.print(F("/"));
    Serial.print(day());
    Serial.print(F("/"));
    Serial.print(year());
    Serial.println();
    //for you non-Arizona folks.... weirdos
    if (((month() == 11) && (day() >= 3) || (month() == 12) || (month() > 0) && (month() < 3) || (month() == 3) && (day() >= 10)))
    {
        timeZone = timeZone++;
    }
}

void printDigits(int digits)
{ // utility for digital clock display: prints preceding colon and leading 0
    Serial.print(F(":"));
    if (digits < 10)
        Serial.print(F("0"));
    Serial.print(digits);
}

// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++)
    {
        str[i] = 0;
    }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind)
{
    char found = 0;
    char index = 0;
    char len;
    len = strlen(str);
    if (strlen(sfind) > len)
    {
        return 0;
    }
    while (index < len)
    {
        if (str[index] == sfind[found])
        {
            found++;
            if (strlen(sfind) == found)
            {
                return 1;
            }
        }
        else
        {
            found = 0;
        }
        index++;
    }
    return 0;
}

void uptime()
{
    days = 0;
    hours = 0;
    mins = 0;
    secs = 0;
    secs = currentMillis / 1000; //convect milliseconds to seconds
    mins = secs / 60;            //convert seconds to minutes
    hours = mins / 60;           //convert minutes to hours
    days = hours / 24;           //convert hours to days
    secs = secs - (mins * 60);   //subtract the coverted seconds to minutes in order to display 59 secs max
    mins = mins - (hours * 60);  //subtract the coverted minutes to hours in order to display 59 minutes max
    hours = hours - (days * 24); //subtract the coverted hours to days in order to display 23 hours max
    //Display results
    Serial.print(F("Current Uptime is "));
    if (days > 0)
    { // days will displayed only if value is greater than zero
        Serial.print(days);
        Serial.print(" days and ");
    }
    Serial.print(hours);
    Serial.print(":");
    Serial.print(mins);
    Serial.print(":");
    Serial.println(secs);
}

void uptimeHttp()
{
    days = 0;
    hours = 0;
    mins = 0;
    secs = 0;
    secs = currentMillis / 1000; //convect milliseconds to seconds
    mins = secs / 60;            //convert seconds to minutes
    hours = mins / 60;           //convert minutes to hours
    days = hours / 24;           //convert hours to days
    secs = secs - (mins * 60);   //subtract the coverted seconds to minutes in order to display 59 secs max
    mins = mins - (hours * 60);  //subtract the coverted minutes to hours in order to display 59 minutes max
    hours = hours - (days * 24); //subtract the coverted hours to days in order to display 23 hours max
    //Display results
    strUptime = String(strUptime + days);
    if (days == 1)
    {
        strUptime = String(strUptime + " day ");
    }
    else
    {
        strUptime = String(strUptime + " days ");
    }
    if (hours < 10)
    {
        strUptime = String(strUptime + '0' + hours);
    }
    else
    {
        strUptime = String(strUptime + hours);
    }
    strUptime = String(strUptime + ":");
    if (mins < 10)
    {
        strUptime = String(strUptime + '0' + mins);
    }
    else
    {
        strUptime = String(strUptime + mins);
    }
    strUptime = String(strUptime + ":");
    if (secs < 10)
    {
        strUptime = String(strUptime + '0' + secs);
    }
    else
    {
        strUptime = String(strUptime + secs);
    }
    strUptime = String(strUptime + " (DD HH:MM:SS)");
}

void headerNotes()
{
    Serial.println(s1);
    Serial.println();
    Serial.print(F("\n This code is listening for HTTP traffic at the URL HTTP://<IP Address>:<Port> printed above"));
    Serial.print(F("\n Most commands should be self-explanatory.  Ambiguous commands have notes."));
    Serial.println();
    Serial.print(F("\n /pool/light/on"));
    Serial.print(F("\n /pool/light/off"));
    Serial.print(F("\n /pool/clean/on <--------------------sets pump to in-floor cleaner RPM"));
    Serial.print(F("\n /pool/clean/off"));
    Serial.print(F("\n /pool/waterfall/on"));
    Serial.print(F("\n /pool/waterfall/off"));
    Serial.print(F("\n /pool/circulate/on <----------------sets pump to low speed circulation"));
    Serial.print(F("\n /pool/circulate/off"));
    Serial.print(F("\n /pool/chlorinator/set/X <-----------where X is 1-8 to change chlor setpoint"));
    Serial.print(F("\n /pool/chlorinator/getsetpoint <-----returns JSON formatted setpoint"));
    Serial.print(F("\n /pool/chlorinator/getsalinity <-----returns JSON formatted salinity"));
    Serial.print(F("\n /pool/chlorinator/error <-----------returns JSON formatted error level"));
    Serial.print(F("\n /pool/water/temp <------------------returns JSON formatted water temp"));
    Serial.print(F("\n /pool/pump/rpm <--------------------returns JSON formatted pump RPM"));
    Serial.print(F("\n /pool/pump/watts <------------------returns JSON formatted pump watts"));
    Serial.print(F("\n /pool/light/state <-----------------returns JSON formatted light status"));
    Serial.print(F("\n /pool/status <----------------------returns HTML formatted pool summary"));
    Serial.print(F("\n /avr/uptime <-----------------------returns JSON formatted arduino uptime"));
    Serial.print(F("\n /avr/api/usage <--------------------returns HTML formatted HTTP API calls"));
    Serial.print(F("\n /xively/update <--------------------manually force Xively update"));
    Serial.println();
    Serial.println();
}