#include "HardwareSerial.h"
#include "WString.h"
#include "Regulator.hpp"
#include "Arduino.h"
#include "PWM.hpp"
#include "Debug.hpp"

// Thresholds that define the bounds of each regulation stage
struct Thresholds {
  const Voltage_t ERROR     = 1.0f;   // Error state if error is bigger than this
  const Voltage_t ADJUST    = 0.5f;   // Adjustment state if error is between this and error threshold
  const Voltage_t STABILIZE = 0.25f;  // Stabilizing state if error is between this and adjustment threshold
} Thresholds;


/// Alters the duty cycle by reading the output voltage, comparing it to BOOST_STD_OUTPUT,
/// then increasing/decreasing the OCR1A register.
void regulateBoostVoltage(DutyCycle_t* currDC_ptr, const Voltage_t BOOST_STD_OUTPUT, int* OCR1A_reg) {

  // Static variable keeps track of state between calls
  static RegulationState_t state = IDLE;

  // Prepare to set new duty cycle
  DutyCycle_t nextDutyCycle;

  // Get Boost Converter output and calculate error 
  Voltage_t boostOutput = measureBoostVoltage();
  Voltage_t error = boostOutput - BOOST_STD_OUTPUT;
  //Serial.println(boostOutput);
  // Determine which state to be in 
  state = determineRegulationState(error);
  //Serial.println((RegulationState_t)state);
  Debug::setDebugLevel(DebugLevel::NONE);

  switch (state) {
    case IDLE:
      Debug::Log(DebugLevel::INFO, "Current State: IDLE");
      setDutyCycle(0.0, OCR1A_reg);
      break;
    case ERROR_NEGATIVE:
      Debug::Log(DebugLevel::INFO, "Current State: ERROR_NEGATIVE");
      increaseDutyCycleBy(3, OCR1A_reg);
      break;
    case ERROR_POSITIVE:
      Debug::Log(DebugLevel::INFO, "Current State: ERROR_POSITIVE");
      decreaseDutyCycleBy(3, OCR1A_reg);
      break;
    case ADJUSTING_NEGATIVE:
      Debug::Log(DebugLevel::INFO, "Current State: ADJUSTING_NEGATIVE");
      increaseDutyCycleBy(2, OCR1A_reg);
      break;
    case ADJUSTING_POSITIVE:
      Debug::Log(DebugLevel::INFO, "Current State: ADJUSTING_POSITIVE");
      decreaseDutyCycleBy(2, OCR1A_reg);
      break;
    case STABILIZING_NEGATIVE:
      Debug::Log(DebugLevel::INFO, "Current State: STABILIZING_NEGATIVE");
      increaseDutyCycleBy(1, OCR1A_reg);
      break;
    case STABILIZING_POSITIVE:
      Debug::Log(DebugLevel::INFO, "Current State: STABILIZING_POSITIVE");
      decreaseDutyCycleBy(1, OCR1A_reg);
      break;
    case STEADY:
    default:
      Debug::Log(DebugLevel::INFO, "Current state: STEADY");
      break;
  }

  // Ensure the newly calculated duty cycle is below the max
  nextDutyCycle = min(nextDutyCycle, MAX_DC);
  // Set and apply
  //setDutyCycle(OCR1A_reg, nextDutyCycle);
  (*currDC_ptr) = nextDutyCycle;
}

/// Given an error from the desired Boost Converter output, which can be either positive or negative,
/// this returns the current state of the Boost Converter
RegulationState_t determineRegulationState(Voltage_t error) {
  if (!sourceConnected()) return IDLE;
  if (error <= -Thresholds.ERROR    ) return ERROR_NEGATIVE;
  if (error >=  Thresholds.ERROR    ) return ERROR_POSITIVE;
  if (error <= -Thresholds.ADJUST   ) return ADJUSTING_NEGATIVE;
  if (error >=  Thresholds.ADJUST   ) return ADJUSTING_POSITIVE;
  if (error <= -Thresholds.STABILIZE) return STABILIZING_NEGATIVE;
  if (error >=  Thresholds.STABILIZE) return STABILIZING_POSITIVE;
  
  // Steady state detected
  return STEADY;
}

