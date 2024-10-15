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

// Custom types for readability
typedef const byte Pin_t;       // Arduino in/out pins
typedef const float Voltage_t;  // Constant & variable/measured voltage sources
typedef const int Resistance_t; // Resistors
typedef float DutyCycle_t;      // As decimals 0 - 100. Ex: 69.1% duty cycle: DutyCycle_t dc = 69.1f;

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
} regulator_t;
const Voltage_t errorStateThreshold = 1.0f;       // Error state if error is bigger than this
const Voltage_t adjustingStateThreshold = 0.5f;  // Adjustment state if error is between this and error threshold
const Voltage_t stabilizingStateThreshold = 0.1f; // Stabilizing state if error is between this and adjustment threshold
// In steady state if error is less than all thresholds

// TODO: Analog/Input Pin definitions
// Pin_t PIN_Thermistor = AX;
Pin_t PIN_BOOST_REF = A0;   // Connects to R6 node in voltage divider/ADC sub circuit
Pin_t PIN_SOURCE = 2;

// Timer parameters
const unsigned long PWM_FREQ_HZ = 50e3;                     // Switching frequency
const byte DECIMAL_PRECISION = 10;                         // Either 1, 10 or 100, Ex: 10 = ##.#%, 1000 = ##.##% 
const unsigned long TCNT1_TOP = 16e6/(2*PWM_FREQ_HZ);  // Period/number of clock cycles of the timer
Pin_t PIN_PWM = 9;                                          // Arduino pin for pwm signal

// Duty cycle info
DutyCycle_t currentDutyCycle = 0.0f; // Start at 0%
const DutyCycle_t MAX_DC = 60.0f;    // Acts as a software-based kill switch

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

/// Runs once at 
void setup() {
  // Configure the voltage divider as an input
  pinMode(PIN_BOOST_REF, INPUT);

  // Configure input source checker
  pinMode(PIN_SOURCE, INPUT);

  Serial.begin(9600);

  // Enable the timer to drive the 50 kHz signal
  configureTimer();

  // Applies the starting duty cycle to the PWM signal
  setPWM_DutyCycle(currentDutyCycle);
}

void loop() {
 
  // TODO #1

  regulateBoostVoltage();
  
  delay(500); // Delay for next analog read
}

void regulateBoostVoltage() {
  // Static variable keeps track of state between calls
  static regulator_t regulationState = IDLE;

  // Prepare to set new duty cycle
  DutyCycle_t nextDutyCycle;
  
  // Get Boost Converter output and calculate error 
  Voltage_t boostOutput = measureBoostVoltage();
  Voltage_t error = boostOutput - BOOST_STD_OUTPUT;

  regulationState = determineRegulationState(error);
  
  switch (regulationState) {
    case IDLE:
      Serial.println("Current State: IDLE");
      nextDutyCycle = 0;
      break;
    case ERROR_NEGATIVE:
      Serial.println("Current State: ERROR_NEGATIVE");
      nextDutyCycle = currentDutyCycle + errorStateThreshold;
      break;
    case ERROR_POSITIVE:
      Serial.println("Current State: ERROR_POSITIVE");
      nextDutyCycle = currentDutyCycle - errorStateThreshold;
      break;
    case ADJUSTING_NEGATIVE:
      Serial.println("Current State: ADJUSTING_NEGATIVE");
      nextDutyCycle = currentDutyCycle + adjustingStateThreshold;
      break;
    case ADJUSTING_POSITIVE:
      Serial.println("Current State: ADJUSTING_POSITIVE");
      nextDutyCycle = currentDutyCycle - adjustingStateThreshold;
      break;
    case STABILIZING_NEGATIVE:
      Serial.println("Current State: STABILIZING_NEGATIVE");
      nextDutyCycle = currentDutyCycle + stabilizingStateThreshold;
      break;
    case STABILIZING_POSITIVE:
      Serial.println("Current State: STABILIZING_POSITIVE");
      nextDutyCycle = currentDutyCycle - stabilizingStateThreshold;
      break;
    case STEADY:
    default:
      Serial.println("Current state: STEADY");
      break;
  }

  // Ensure the newly calculated duty cycle is below the max
  nextDutyCycle = minimum(nextDutyCycle, MAX_DC);

  // Set and apply
  setPWM_DutyCycle(nextDutyCycle);
  
  Serial.println(nextDutyCycle);
  currentDutyCycle = nextDutyCycle;
}

regulator_t determineRegulationState(Voltage_t error) {
  if (!sourceConnected()) return IDLE;
  if (error <= -errorStateThreshold) return ERROR_NEGATIVE;
  if (error >= errorStateThreshold) return ERROR_POSITIVE;
  if (error <= -adjustingStateThreshold) return ADJUSTING_NEGATIVE;
  if (error >= adjustingStateThreshold) return ADJUSTING_POSITIVE;
  if (error <= -stabilizingStateThreshold) return STABILIZING_NEGATIVE;
  if (error >= stabilizingStateThreshold) return STABILIZING_POSITIVE;
  
  // Steady state detected
  return STEADY;
}

bool sourceConnected() {
  int x = digitalRead(PIN_SOURCE);
  return x;

}

void configureTimer() {
  pinMode(PIN_PWM, OUTPUT);

  // Clear Timer1 control and count registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  TCCR1A |= (1 << COM1A1) | (1 << WGM11); // Non-inverting PWM
  TCCR1B |= (1 << WGM13) | (1 << CS10);   // Fast PWM, TOP=ICR1
  ICR1 = TCNT1_TOP;
}

/// Reads 
void configureDutyCycle() {
  // Bias to further vary output voltage
  // More positive bias = higher duty cycle = higher output voltage
  Voltage_t bias = -100e-3; // 100mV bias
  currentDutyCycle = 1 - (BOOST_STD_INPUT / (BOOST_STD_OUTPUT + D1 + bias));// REVIEW - Only considers Schottky diode drop
  currentDutyCycle *= 100; // Convert from decimal to percentage
}

/// Returns the output to the Boost Converter
/// Refer to the "Boost Converter Output Voltage" equation/derivation in the README
Voltage_t measureBoostVoltage() {

  // Read the reference voltage. Resistors chosen such that this input is always less than 5V
  Voltage_t Ref = analogRead(PIN_BOOST_REF) * 5 / 1023.0f;

  // Use equation to get output voltage
  Voltage_t boostOutputVoltage = (Ref * (R5 + R6)) / R6 + D3;
  
  return boostOutputVoltage;
}

// DEPRECATED
Voltage_t calculateBatteryVoltage() {
  
  return 0;
  // Calculate current through
  //return voltageRead * (R5 + R6) / R6; // TODO: Account for D3 diode drop
}

/// Sets the duty cycle of the timer 
void setPWM_DutyCycle(DutyCycle_t dc) {
  unsigned long o = (dc * DECIMAL_PRECISION * TCNT1_TOP) / 1000.0f; // Sets to pin 9
  Serial.println(o);
  OCR1A = o;
}

template <typename T>
T minimum(T a, T b) {
  return (a < b) ? a : b;
}