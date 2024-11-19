#include "HardwareSerial.h"
#include "Bluetooth.hpp"
#include "Arduino.h"
#include "SoftwareSerial.h"

SoftwareSerial BTserial(2, 3);

void configureBluetooth() {
  BTserial.begin(9600);
}

void transmitBluetooth(String message) {

  BTserial.println(message);
  for (int i = 0; i < 30; i++) {
    delay(1000);
  }
}