// Reference: https://github.com/T-622/25KHz-Arduino-Fan-PWM-Control/blob/main/25KHz_PWM_UNO.ino

/* 
  TODOs:
    1. Reset duty cycle to 0 if there is no input voltage to the ADC,
        Meaning, the boost converter is not operating. Don't want to see
        huge voltage spikes when reconnecting the load and the duty cycle is already high
    2. Create custom debug logger object. Should be able to, anywhere in the code, set a debug
       level (info, warn, error, etc) and log it to the serial monitor
    3. Create FSM for monitoring output voltage. States are OverVoltage, UnderVoltage, Steady
       
*/

/*
  REVIEWs:
    1. Refer to https://www.baldengineer.com/5-voltage-divider-circuits.html
        Specifically, the part on adding transistors to divider circuits. To prevent leakage into the
        Arduino pin, we can use another pin to turn off that voltage divider circuit when we aren't
        reading from it, saving power.
*/

#include "Regulator.hpp"
#include "PWM.hpp"
#include "Debug.hpp"
#include "types.hpp"

// Pin_t PIN_Thermistor = AX;
struct Pins {
  Pin_t BOOST_REF = A0; // Connects to R6 node in voltage divider/ADC sub circuit
  Pin_t SOURCE = 2;
  Pin_t PWM = 9;    // Arduino pin for pwm signal
} Pins;

// Timer parameters
const unsigned long PWM_FREQ_HZ = 50e3;                   // Switching frequency
const unsigned long TCNT1_TOP = 16e6 / (2 * PWM_FREQ_HZ); // Period/number of clock cycles of the timer

// Duty cycle info
DutyCycle_t currentDutyCycle = 0.0f; // Start at 0%

// Circuit components (reference schematic)
Resistance_t R5 = 3900; // ADC voltage divider
Resistance_t R6 = 1950; // ADC voltage divider
const Voltage_t D1 = 0.206f; // Measure with multimeter
const Voltage_t D5 = 0.580f; // Measure with multimeter
const Voltage_t D3 = 0.580f; // Measure with multimeter

// Circuit parameters
const Voltage_t NOMINAL_BATTERY = 9.0f;   // What the battery voltage should be
const Voltage_t BOOST_STD_INPUT = 5.0f;   // What the input of the boost converter should be
const Voltage_t BOOST_STD_OUTPUT = 10.4f; // What the output of the boost converter should be

void setup() {
  // Pin & timer setup
  configurePins();
  configureTimer();
  
  // Logging
  Serial.begin(9600);
  Debug::setDebugLevel(DebugLevel::WARN);

  // Applies the starting duty cycle to the PWM signal
  setDutyCycle(&OCR1A, currentDutyCycle);
}

void loop() {
  regulateBoostVoltage(&currentDutyCycle, BOOST_STD_OUTPUT, &OCR1A);
  delay(100); // Delay for next analog read
}

void configureTimer() {
  // Clear Timer1 control and count registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  TCCR1A |= (1 << COM1A1) | (1 << WGM11); // Non-inverting PWM
  TCCR1B |= (1 << WGM13) | (1 << CS10);   // Fast PWM, TOP=ICR1, No prescaler
  ICR1 = TCNT1_TOP;
}

void configurePins() {
  pinMode(Pins.PWM, OUTPUT);
  pinMode(Pins.BOOST_REF, INPUT);
  pinMode(Pins.SOURCE, INPUT);
}

/// Determines whether the source is connected or not.
/// `digitalRead` returns either HIGH (logic 1/true) or LOW (logic 0/false).
bool sourceConnected() {
  return digitalRead(Pins.SOURCE);
}

/// Returns the output to the Boost Converter
/// Refer to the "Boost Converter Output Voltage" equation/derivation in the README
Voltage_t measureBoostVoltage() {
  // Reads the reference voltage, then calculates output voltage
  Voltage_t Ref = analogRead(Pins.BOOST_REF) * 5 / 1023.0f;
  Voltage_t boostOutputVoltage = (Ref * (R5 + R6)) / R6 + D3;
  return boostOutputVoltage;
}
