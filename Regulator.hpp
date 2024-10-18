#ifndef REGULATOR_HPP
#define REGULATOR_HPP

#include "types.hpp"

// FSM States and parameters
typedef enum { 
  IDLE,                 // No input to measure
  ERROR_NEGATIVE,       // Significantly under set output
  ERROR_POSITIVE,       // Significantly over set output
  ADJUSTING_NEGATIVE,   // Marginally under set output
  ADJUSTING_POSITIVE,   // Marginally over set output
  STABILIZING_NEGATIVE, // Slightly under set output
  STABILIZING_POSITIVE, // Slightly over set output
  STEADY                // Negligibe to no difference compared to output
} RegulationState_t;

// struct Thresholds {
//   const Voltage_t ERROR     = 1.0f;   // Error state if error is bigger than this
//   const Voltage_t ADJUST    = 0.5f;   // Adjustment state if error is between this and error threshold
//   const Voltage_t STABILIZE = 0.25f;  // Stabilizing state if error is between this and adjustment threshold
// } Thresholds;


const DutyCycle_t MAX_DC = 100.0f;    // Acts as a software-based kill switch

void regulateBoostVoltage(DutyCycle_t* currentDutyCycle, const Voltage_t OUT, int* OCR1A_reg);
RegulationState_t determineRegulationState(Voltage_t error);

// Defined in WSN.ino ------------------------------
Voltage_t measureBoostVoltage();
bool sourceConnected();
// -------------------------------------------------

#endif