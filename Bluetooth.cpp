#include "HardwareSerial.h"
#include "Bluetooth.hpp"
#include "Arduino.h"
#include "SoftwareSerial.h"

SoftwareSerial BTserial(2, 3);

void configureBluetooth() {
  BTserial.begin(9600);
}

void transmitBluetooth() {

  // Read incoming data
  int bytesAvailable = BTserial.available();
  for (int i = 0; i < bytesAvailable; i++) {
    byte x = BTserial.read();
    Serial.write(x);
  }

  // Print out an extra line
  if (bytesAvailable) {
    Serial.write('\n');
  }

  // Send outgoing data
  int serialBytesAvailable = Serial.available();
  for (int i = 0; i < serialBytesAvailable; i++) {
    byte y = Serial.read();
    BTserial.write(y);
  }
}

void sendMessage() {
}