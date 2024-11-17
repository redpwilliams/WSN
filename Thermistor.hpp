#ifndef THERMISTOR_HPP
#define THERMISTOR_HPP

#include "types.hpp"

// Steinhart-Hart Constants
const float A = 1.009249522e-03;
const float B = 2.378405444e-04;
const float C = 2.019202697e-07;

void processTemperature(Temperature_t* temps, Voltage_t analogIn, Resistance_t thermistance);

#endif