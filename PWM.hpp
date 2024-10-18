#ifndef PWM_HPP
#define PWM_HPP

#include "types.hpp"

void increaseDutyCycle(int* OCR1A_reg);
void dereaseDutyCycle(int* OCR1A_reg);
void setDutyCycle(int* OCR1A_reg, DutyCycle_t dutyCycle);

#endif