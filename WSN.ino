// Reference: https://github.com/T-622/25KHz-Arduino-Fan-PWM-Control/blob/main/25KHz_PWM_UNO.ino

/*
  Extra notes & consideration:
  - Throughout this code, the byte primitive is used more
    often than ints because ints are 2 bytes wide.
*/

// Custom types for readability
typedef const byte Pin_t; // For arduino in/out pins
typedef int Voltage_t;
typedef const int Resistance_t;
typedef byte DutyCycle_t; // As whole ints (30% duty cycle: DutyCycle_t dc = 30;)

// TODO: Pin definitions
// Pin_t PIN_Thermistor = x;
Pin_t PIN_CUTOFF = A0;

// Timer parameters
const unsigned long PWM_FREQ_HZ = 50e3; // Adjust this value to adjust the frequency
const unsigned long TCNT1_TOP = 16000000/(2*PWM_FREQ_HZ);
Pin_t OC1A_PIN = 9; // Use arduino pin 9 for pwm signal

// Duty cycle info
DutyCycle_t currentDutyCycle = 0; // Start at 0%
const DutyCycle_t MAX_DC = 60;

// Circuit components (reference schematic)
Resistance_t R5 = 780;
Resistance_t R6 = 1000;
const Voltage_t D1 = 0.206; // Measure with multimeter
const Voltage_t D5 = 0.580; // Measure with multimeter
const Voltage_t D3 = 0.580; // Measure with multimeter

// Circuit parameters
const Voltage_t NOMINAL_BATTERY = 9;
const Voltage_t V_INPUT = 5;
const Voltage_t V_OUTPUT = 9;

// Analog Input (to read battery voltage)

void setup() {
  pinMode(PIN_CUTOFF, INPUT);
  configureTimer();
  // configureDutyCycle();
  setPWM_DutyCycle(currentDutyCycle);
}

void loop() {

  // Calculate actual battery voltage
  Voltage_t reference = calculateBatteryVoltage();
  DutyCycle_t nextDutyCycle;

  if (reference < NOMINAL_BATTERY) {
    // Increase duty cycle
    nextDutyCycle = currentDutyCycle + 1;
  }
  else if (reference > NOMINAL_BATTERY) {
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
  pinMode(OC1A_PIN, OUTPUT);

  // Clear Timer1 control and count registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  TCCR1A |= (1 << COM1A1) | (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << CS10);
  ICR1 = TCNT1_TOP;
}

// Reads 
void configureDutyCycle() {
  // Bias to further vary output voltage
  // More positive bias = higher duty cycle = higher output voltage
  const Voltage_t bias = -100e-3; // 100mV bias
  currentDutyCycle = 1 - (V_INPUT / (V_OUTPUT + D1 + bias));// REVIEW - Only considers Schottky diode drop
  currentDutyCycle *= 100; // Convert from decimal to percentage
}

Voltage_t calculateBatteryVoltage() {
  const Voltage_t voltageRead = analogRead(PIN_CUTOFF) * 5 / 1023.0;
  return voltageRead * (R5 + R6) / R6; // TODO: Account for D3 diode drop
}

void setPWM_DutyCycle(DutyCycle_t dc) {
  OCR1A = (unsigned long) (dc * TCNT1_TOP) / 100; // Sets to pin 9
}

template <typename T>
T minimum(T a, T b) {
  return (a < b) ? a : b;
}