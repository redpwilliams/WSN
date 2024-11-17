#include "Thermistor.hpp"
#include "math.h"

void processTemperature(Temperature_t* temps, Voltage_t analogIn, Resistance_t thermistance) {
  // Set struct's Kelvin, Celsius, and Fahrenheit
  temps->K = pow(A + B * thermistance + C * pow(thermistance, 3.0), -1); // Steinhart-Hart equation
  temps->C = (temps->K) - 273.15; // Kelvin to Celsius
  temps->F = (temps->C * 9.0f) / 5.0f + 32.0f; // Celsius to Fahrenheit 

}