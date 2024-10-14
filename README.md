# Wireless Sensor Node
This repository contains current and working code for the Wireless Sensor Node project for ECE 2084, meant to run on an Arduino UNO.

# Equation Derivations

## Boost Converter Output Voltage
The boost converter output voltage changes with the load. To keep the output consistent across different loads, it needs to be monitored, and increased/decreased in proportion to the change in load by changing the duty cycle that drives the mosfet gate. The Arduino analog pins (AX) cannot read voltages higher than 5 V to 5.5 V, so the circuit uses a voltage divider, as well as a diode to protect the arduino against reverse polarity. The circuit parameters are as follows:
* R1 - First resistor (between boost output and diode)
* D - Diode forward voltage drop
* R2 - Second resistor (between diode's cathode/A0 pin and ground)
Let V represent the boost converter's true output voltage and Ref represent the voltage read by the Arduino (at the A0 pin). Using the Thevenin equivalent circuit, the current through the system is $I = \frac{V - D}{R1 + R2}$. In the original circuit, that current flows through R2, and is described by $I = \frac{Ref}{R2}$. Together, $\frac{V - D}{R1 + R2} = \frac{Ref}{R2}$. Solving for V, $V = \frac{Ref \times (R1 + R2)}{R2} + D$.
As a note, high resistance values (~10k) should be chosen to limit current draw to the Arduino. The only requirement should be a proper ratio of R1 and R2 to keep the input measured by the Arduino below 5 V, despite the maximum and minimums of the boost output and DC source input.

## Boost Converter Output Voltage Divider Resistor Selection
The ratio of resistors R1 and R2 (defined in the above section), is defined with this derivation. This follows the expression of the boost converter output voltage via the voltage divider: $V = \frac{Ref \times (R1 + R2)}{R2} + D$. Given V and Ref, solving for R1/R2, $\frac{V - D}{Ref} - 1 = \frac{R1}{R2}$. Choose resistor values that would create an ideal voltage divider given a sample V and Ref. For example, ideally, a 10.4 V input to the ADC voltage divider (which is the output of the boost converter) makes a 3.3V input to the ADC. Therefore, (using a D = 0.7), the ratio $R1/R2 = \frac{10.4 - 0.7}{3.3} - 1 = 1.939$. If R1 = 5 kΩ, then R2 = 2.578 kΩ.
