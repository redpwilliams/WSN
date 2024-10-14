// Reference: https://github.com/T-622/25KHz-Arduino-Fan-PWM-Control/blob/main/25KHz_PWM_UNO.ino

/* 
  TODOs:
    1. Reset duty cycle to 0 if there is no input voltage to the ADC,
        Meaning, the boost converter is not operating. Don't want to see
        huge voltage spikes when reconnecting the load and the duty cycle is already high
    2. Create custom debug logger object. Should be able to, anywhere in the code, set a debug
       level (info, warn, error, etc) and log it to the serial monitor
       
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
typedef byte DutyCycle_t;       // As whole ints (30% duty cycle: DutyCycle_t dc = 30;)

// TODO: Analog/Input Pin definitions
// Pin_t PIN_Thermistor = AX;
Pin_t PIN_BOOST_REF = A0;   // Connects to R6 node in voltage divider/ADC sub circuit

// Timer parameters
const unsigned long PWM_FREQ_HZ = 50e3;               // Switching frequency
const unsigned long TCNT1_TOP = 16e6/(2*PWM_FREQ_HZ); // Sets up timer
Pin_t PIN_PWM = 9;                                    // Arduino pin for pwm signal

// Duty cycle info
DutyCycle_t currentDutyCycle = 0; // Start at 0%
const DutyCycle_t MAX_DC = 60;    // Acts as a software-based kill switch

// Circuit components (reference schematic)
Resistance_t R5 = 780;  // ADC voltage divider
Resistance_t R6 = 1000; // ADC voltage divider
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

  // Enable the timer to drive the 50 kHz signal
  configureTimer();

  // Applies the starting duty cycle to the PWM signal
  setPWM_DutyCycle(currentDutyCycle);
}

void loop() {
  // Get Boost Converter Output
  Voltage_t boostOutput = measureBoostVoltage();

  // TODO #1

  DutyCycle_t nextDutyCycle;

  if (boostOutput < NOMINAL_BATTERY) {
    // Increase duty cycle
    nextDutyCycle = currentDutyCycle + 1;
  }
  else if (boostOutput > NOMINAL_BATTERY) {
    // Decrease duty cycle
    nextDutyCycle = currentDutyCycle - 1;
  }
  
  // Ensure the newly calculated duty cycle is below the max
  nextDutyCycle = minimum(nextDutyCycle, MAX_DC);

  // Set and apply
  setPWM_DutyCycle(nextDutyCycle);
  currentDutyCycle = nextDutyCycle;
  
  delay(500); // Delay for next analog read
}

void configureTimer() {
  pinMode(PIN_PWM, OUTPUT);

  // Clear Timer1 control and count registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  TCCR1A |= (1 << COM1A1) | (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << CS10);
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
  

  // Calculate current through
  //return voltageRead * (R5 + R6) / R6; // TODO: Account for D3 diode drop
}

void setPWM_DutyCycle(DutyCycle_t dc) {
  OCR1A = (unsigned long) (dc * TCNT1_TOP) / 100; // Sets to pin 9
}

template <typename T>
T minimum(T a, T b) {
  return (a < b) ? a : b;
}