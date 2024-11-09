#ifndef PWM_HPP
#define PWM_HPP

#include "types.hpp"

void increaseDutyCycle(int* OCR1A_reg);
void dereaseDutyCycle(int* OCR1A_reg);
void increaseDutyCycleBy(int delta, int *OCR1A_reg);
void decreaseDutyCycleBy(int delta, int *OCR1A_reg);
void setDutyCycle(DutyCycle_t dutyCycle, int* OCR1A_reg);

#endif