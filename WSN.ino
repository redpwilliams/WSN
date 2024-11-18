// Wireless Sensor Node
// ECE 2804 - Inegrated Design Project
// Red Williams, Ahmed Yousif

/* 
  TODOs:
    1. All clear!     
*/

/*
  REVIEWs:
    1. Refer to https://www.baldengineer.com/5-voltage-divider-circuits.html
        Specifically, the part on adding transistors to divider circuits. To prevent leakage into the
        Arduino pin, we can use another pin to turn off that voltage divider circuit when we aren't
        reading from it, saving power.
*/

#include "Regulator.hpp"  // Boost output regulation
#include "PWM.hpp"        // Changes to duty cycle
#include "Thermistor.hpp" // Thermistor readings
#include "Bluetooth.hpp"  // Bluetooth receive/transmit
#include "Debug.hpp"      // Clever system logging
#include "types.hpp"      // Types for readability

struct Pins {
  Pin_t BOOST_REF = A0; // Connects to R6 node in voltage divider/ADC sub circuit
  Pin_t SOURCE = 2;
  Pin_t PWM = 9;    // Arduino pin for pwm signal
  Pin_t THERMISTOR = A2; // Analog input for thermistor
  Pin_t BLUETOOTH_RX = 3;
  Pin_t BLUETOOTH_TX = 4;
} Pins;

struct Temperature_t temps;

// Timer parameters
const unsigned long PWM_FREQ_HZ = 50e3;                   // Switching frequency
const unsigned long TCNT1_TOP = 16e6 / (2 * PWM_FREQ_HZ); // Period/number of clock cycles of the timer

// Duty cycle info
DutyCycle_t currentDutyCycle = 0.0f; // Start at 0%

// Circuit components (reference schematic)
Resistance_t R5 = 19.16e3; // ADC voltage divider
Resistance_t R6 = 10e3;   // ADC voltage divider
Resistance_t R10 = 10e3; // Thermistor series resistance
const Voltage_t D1 = 0.206f; // Measure with multimeter
const Voltage_t D5 = 0.580f; // Measure with multimeter
const Voltage_t D3 = 0.595f; // Measure with multimeter

// Circuit parameters
const Voltage_t NOMINAL_BATTERY = 9.0f;   // What the battery voltage should be
const Voltage_t BOOST_STD_INPUT = 5.0f;   // What the input of the boost converter should be
const Voltage_t BOOST_STD_OUTPUT = 10.4f; // What the output of the boost converter should be

// Main Arduino functions --------------------------------------------------------------------

/// Sets up the pins, timer, debug instructions.
/// Sets the initial duty cycle to 0.
void setup() {
  // Pin & timer setup
  configurePins();
  configureTimer();
  configureBluetooth();

  // Logging
  Serial.begin(9600);
  
  // Applies the starting duty cycle to the PWM signal
  setDutyCycle(currentDutyCycle, &OCR1A);
}

/// As of this commit, this program continuously monitors the output
/// of the boost converter 10x each second and keeps it at a stable output.
void loop() {
  // regulateBoostVoltage(&currentDutyCycle, BOOST_STD_OUTPUT, &OCR1A);
  
  // Measure thermistor temp readings
  // measureTemperature(&temps);  // Pass in temperatures struct defined earlier in file
  // Serial.print("Fahrenheit: ");
  // Serial.println(temps.F);
  // Serial.print("Celsius:    ");
  // Serial.println(temps.C);
  // Serial.print("Kelvin:     ");
  // Serial.println(temps.K);
  // Serial.println();
  transmitBluetooth();
  sendMessage();

  delay(1000); // Delay for next analog read
}

// Helper functions --------------------------------------------------------------------------

/// Sets each pin defined in the Pins struct
/// as an input or an output.
void configurePins() {
  pinMode(Pins.BOOST_REF,   INPUT);
  pinMode(Pins.THERMISTOR,  INPUT);
  pinMode(Pins.SOURCE,      INPUT);
  pinMode(Pins.PWM,         OUTPUT);
}

/// Configures the 16-bit timer on the ATmega328p using predefined macros.
/// No prescaler is used, and ICR1 is set so that it generates a 50 kHz square wave.
/// The ratio of register OCR1A to ICR1 is the duty cycle the timer operates at.
void configureTimer() {
  // Clear Timer1 control and count registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  TCCR1A |= (1 << COM1A1) | (1 << WGM11); // Non-inverting PWM
  TCCR1B |= (1 << WGM13) | (1 << CS10);   // Fast PWM, TOP=ICR1, No prescaler
  ICR1 = TCNT1_TOP;
}

/// Determines whether the source is connected or not.
/// `digitalRead` returns either HIGH (logic 1/true) or LOW (logic 0/false).
bool sourceConnected() {
  return digitalRead(Pins.SOURCE);
}

Temperature_t measureTemperature(Temperature_t* tempsRef) {
  // Voltage read at ADC
  Voltage_t ref = analogRead(Pins.THERMISTOR) * 5 / 1023.0f;
  
  // Thermistor internal resistance
  Resistance_t thermistance = R10 * (5 / ref - 1);
  
  // Apply Steinhart-Hart equation
  processTemperature(&temps, ref, thermistance); // temps struct is updated
}

/// Returns the output to the Boost Converter
/// Refer to the "Boost Converter Output Voltage" equation/derivation in the README
Voltage_t measureBoostVoltage() {
  // Reads the reference voltage, then calculates output voltage
  Voltage_t Ref = analogRead(Pins.BOOST_REF) * 5 / 1023.0f;
  Voltage_t boostOutputVoltage = (Ref * (R5 + R6)) / R6 + D3;
  Serial.println(Ref);
  return boostOutputVoltage;
}
