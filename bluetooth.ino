#include <SoftwareSerial.h>

int rx = 2;
int tx = 3;

SoftwareSerial BTserial(rx, tx); // SRX | STX
// Connect the HC-05 TX to Arduino pin 2(as SRX).
// Connect the HC-05 RX to Arduino pin 3 (as STX).

void setup()
{
 Serial.begin(9600);
 BTserial.begin(9600);
}

void loop()
{
 if (BTserial.available())
 {
   byte x = BTserial.read();
   Serial.write(x);
   BTserial.write("You sent " + x);
 }

 if (Serial.available())
 {
   byte y = Serial.read();
   BTserial.write(y);
 }
}