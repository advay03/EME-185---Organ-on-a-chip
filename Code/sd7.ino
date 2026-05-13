#include <AccelStepper.h>
#include <TMCStepper.h>
#include <SoftwareSerial.h>
#include <math.h>

#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00

// ---------------------------------------------------------------
// PIN DEFINITIONS
// ---------------------------------------------------------------
const int heaterPin = 8;       // moved off pins used by steppers
const int tempPin   = A0;

const int dirPins[]  = {6, 9, 12};
const int stepPins[] = {7, 10, 13};
const int uartPins[] = {3, 4, 5};  // FIXED: moved off hardware serial pins 1 and 2
const int enPins[]   = {A1, A2, A3}; // FIXED: moved off pin 3 conflict with heater

// ---------------------------------------------------------------
// THERMISTOR CONSTANTS
// ---------------------------------------------------------------
const int   R0  = 10000;
const float Tn  = 25.0;
const int   Rn  = 10000;
const int   B   = 3950;

// ---------------------------------------------------------------
// PUMP GEOMETRY
// ---------------------------------------------------------------
const float tubeID = 0.0005;
const float critR  = 0.00717;

// ---------------------------------------------------------------
// SERIAL INPUT
// ---------------------------------------------------------------
String inputString   = "";
bool stringComplete  = false;

// ---------------------------------------------------------------
// TEMPERATURE CONTROL
// ---------------------------------------------------------------
float targetTemp = 37.0;

// ---------------------------------------------------------------
// PUMP CONTROL
// ---------------------------------------------------------------
float flowRate1 = 1.67e-11;
float flowRate2 = 1.67e-11;
float flowRate3 = 1.67e-11;
bool pump1_on = false;
bool pump2_on = false;
bool pump3_on = false;

float stepsPerSec1 = 0;
float stepsPerSec2 = 0;
float stepsPerSec3 = 0;

unsigned long lastTemp = 0;
const unsigned long tempInterval = 500;
float tempC = 0;

// ---------------------------------------------------------------
// DRIVER + STEPPER SETUP
// ---------------------------------------------------------------
SoftwareSerial uart1(uartPins[0], uartPins[0]);
TMC2209Stepper driver1(&uart1, R_SENSE, DRIVER_ADDRESS);

SoftwareSerial uart2(uartPins[1], uartPins[1]);
TMC2209Stepper driver2(&uart2, R_SENSE, DRIVER_ADDRESS);

SoftwareSerial uart3(uartPins[2], uartPins[2]);
TMC2209Stepper driver3(&uart3, R_SENSE, DRIVER_ADDRESS);

AccelStepper pump1(AccelStepper::DRIVER, stepPins[0], dirPins[0]);
AccelStepper pump2(AccelStepper::DRIVER, stepPins[1], dirPins[1]);
AccelStepper pump3(AccelStepper::DRIVER, stepPins[2], dirPins[2]);

// ---------------------------------------------------------------
// FUNCTIONS
// ---------------------------------------------------------------
float readTempCelsius() {
  int   adc      = analogRead(tempPin);
  float v        = adc * (5.0 / 1023.0);
  float Rt       = R0 * ((5.0 / v) - 1.0);
  float T_kelvin = 1.0 / ((1.0 / (Tn + 273.15)) + (1.0 / B) * log(Rt / Rn));
  return T_kelvin - 273.15;
}

float calculateSpeed(float flowRate) {
  if (flowRate == 0) return 0;
  float area       = PI * pow(tubeID / 2.0, 2);
  float flowVel    = flowRate / area;
  float rpm        = (flowVel / (2.0 * PI * critR)) * 60.0;
  float stepsPerSec = (rpm / 60.0) * 200.0 * 8;  
  return stepsPerSec;
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}

// ---------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  inputString.reserve(50);

  // Pin modes
  pinMode(heaterPin, OUTPUT);
  pinMode(tempPin, INPUT);

  pinMode(dirPins[0], OUTPUT);  pinMode(dirPins[1], OUTPUT);  pinMode(dirPins[2], OUTPUT);
  pinMode(stepPins[0], OUTPUT); pinMode(stepPins[1], OUTPUT); pinMode(stepPins[2], OUTPUT);
  pinMode(enPins[0], OUTPUT);   pinMode(enPins[1], OUTPUT);   pinMode(enPins[2], OUTPUT);

  digitalWrite(heaterPin, LOW);

  // Initialize TMC2209 drivers via UART
  uart1.begin(115200);
  driver1.begin();
  driver1.toff(5);
  driver1.rms_current(600);
  driver1.microsteps(256);
  delay(50);
  uart1.end();

  uart2.begin(115200);
  driver2.begin();
  driver2.toff(5);
  driver2.rms_current(600);
  driver2.microsteps(256);
  delay(50);
  uart2.end();

  uart3.begin(115200);
  driver3.begin();
  driver3.toff(5);
  driver3.rms_current(600);
  driver3.microsteps(256);
  delay(50);
  uart3.end();

  // FIXED: setMaxSpeed must be high enough for 256 microsteps
  pump1.setMaxSpeed(10000);
  pump2.setMaxSpeed(10000);
  pump3.setMaxSpeed(10000);

  // Enable drivers (LOW = enabled)
  digitalWrite(enPins[0], LOW);
  digitalWrite(enPins[1], LOW);
  digitalWrite(enPins[2], LOW);

  // All pumps clockwise
  digitalWrite(dirPins[0], HIGH);
  digitalWrite(dirPins[1], HIGH);
  digitalWrite(dirPins[2], HIGH);

}

// ---------------------------------------------------------------
// MAIN LOOP
// ---------------------------------------------------------------
void loop() {

  // Handle serial commands from UI
  if (stringComplete) {
    int commaIndex = inputString.indexOf(',');

    if (commaIndex > 0) {
      String cmd    = inputString.substring(0, commaIndex);
      String valStr = inputString.substring(commaIndex + 1);
      float val     = valStr.toFloat();

      if (cmd == "T") {
        targetTemp = val;
      }
      else if (cmd == "F1") {
        flowRate1    = val;
        stepsPerSec1 = calculateSpeed(flowRate1);
        pump1.setSpeed(stepsPerSec1);
      }
      else if (cmd == "F2") {
        flowRate2    = val;
        stepsPerSec2 = calculateSpeed(flowRate2);
        pump2.setSpeed(stepsPerSec2);
      }
      else if (cmd == "F3") {
        flowRate3    = val;
        stepsPerSec3 = calculateSpeed(flowRate3);
        pump3.setSpeed(stepsPerSec3);
      }
      else if (cmd == "P1") {
        pump1_on = (val > 0);
        digitalWrite(enPins[0], pump1_on ? LOW : HIGH);
      }
      else if (cmd == "P2") {
        pump2_on = (val > 0);
        digitalWrite(enPins[1], pump2_on ? LOW : HIGH);
      }
      else if (cmd == "P3") {
        pump3_on = (val > 0);
        digitalWrite(enPins[2], pump3_on ? LOW : HIGH);
      }
    }

    inputString    = "";
    stringComplete = false;
  }

  // Temperature reading every 500ms
  if (millis() - lastTemp > tempInterval) {
    lastTemp = millis();
    tempC    = readTempCelsius();

    // Send temp to UI
    Serial.print("TEMP,");
    Serial.println(tempC);
  }

  // Heater hysteresis control
  if (tempC < targetTemp - 0.2) {
    digitalWrite(heaterPin, HIGH);
  }
  if (tempC > targetTemp + 0.2) {
    digitalWrite(heaterPin, LOW);
  }

  // Run pumps
  if (pump1_on) pump1.runSpeed();
  if (pump2_on) pump2.runSpeed();
  if (pump3_on) pump3.runSpeed();
}
