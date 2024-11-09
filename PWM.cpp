#include "PWM.hpp"

/// Increases the duty cycle by its smallest increment (0.625%)
void increaseDutyCycle(int* OCR1A_reg) {
  (*OCR1A_reg)++;
}

/// Decreases the duty cycle by its smallest increment (0.625%)
void dereaseDutyCycle(int* OCR1A_reg) {
  (*OCR1A_reg)--;
}

/// Increases the duty cycle by a certain amount
void increaseDutyCycleBy(int delta, int *OCR1A_reg) {
  (*OCR1A_reg) += delta;
}

/// Increases the duty cycle by a certain amount
void decreaseDutyCycleBy(int delta, int *OCR1A_reg) {
  (*OCR1A_reg) -= delta;
}

/// ATTEMPS to set the duty cycle to a specific value.
/// Rounds down to the closest value.
void setDutyCycle(DutyCycle_t dutyCycle, int* OCR1A_reg) {
  //(*OCR1A_reg) = dutyCycle * 160 / 100.0f;
  (*OCR1A_reg) = dutyCycle;
} 