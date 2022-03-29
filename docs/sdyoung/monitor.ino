int onBoardLed = 13;
int DTR = 2; //set low for Rx, high for Tx
int inBytes;

void setup()
{
    pinMode(onBoardLed, OUTPUT);
    pinMode(DTR, OUTPUT);
    Serial.begin(115200);
    Serial1.begin(9600); //Begin Serial to talk to the EasyTouch Panel
}

void loop()
{
    digitalWrite(DTR, LOW); //Enable Receiving Data
    while (Serial1.available())
    {
        inBytes = Serial1.read();
        if (inBytes < 0x10)
        {
            Serial.write('0'); // if you don't do this, values 0 to 15 only print 1 character instead of 2
        }
        Serial.print(inBytes, HEX); // all hex here should print as 2 characters 00 to FF
        Serial.print(" ");
    }
}