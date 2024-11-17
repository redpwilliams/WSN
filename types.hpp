#ifndef types_hpp
#define types_hpp

#include <stdint.h>

// Custom types for readability
typedef const uint8_t Pin_t;       // Arduino in/out pins
typedef const float Voltage_t;  // Constant & variable/measured voltage sources
typedef const int Resistance_t; // Resistors
typedef float DutyCycle_t;      // As decimals 0 - 100. Ex: 69.1% duty cycle: DutyCycle_t dc = 69.1f;
typedef struct Temperature_t { // A temperature reading from the thermistor. 
  float F;
  float C;
  float K;
}; 

#endif