#include <AccelStepper.h>
#include <TMCStepper.h>
#include <SoftwareSerial.h>
#include <math.h>

#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00

// ---------------------------------------------------------------
// PIN DEFINITIONS
// ---------------------------------------------------------------
const int heaterPin = 2;       // moved off pins used by steppers
const int tempPin   = A0;

const int dirPins[]  = {7, 10, 13};
const int stepPins[] = {6, 9, 12};
const int uartPins[] = {5, 8, 11};  // FIXED: moved off hardware serial pins 1 and 2
const int enPins[]   = {A3, A2, A1}; // FIXED: changed to analog pins to free up pin 2 for heater

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
const float tubeID = 0.0005; // meters
const float critR  = 0.00717; // meters
const float k_pump = 1; 

// ---------------------------------------------------------------
// SERIAL INPUT
// ---------------------------------------------------------------
String inputString   = "";
bool stringComplete  = false;

// ---------------------------------------------------------------
// TEMPERATURE CONTROL
// ---------------------------------------------------------------
float targetTemp = 37;

// ---------------------------------------------------------------
// PUMP CONTROL
// ---------------------------------------------------------------
float flowRate1 = 1.67e-11; // m^3/s
float flowRate2 = 1.67e-11; // m^3/s
float flowRate3 = 1.67e-11; // m^3/s
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

float calculateSpeed(float flowRatemlh) {
  if (flowRatemlh == 0) return 0;
  float flowRate = k_pump * flowRatemlh / 3.6E12; // m^3/s to microliters/hr times experimental correction factor k
  float area       = PI * pow(tubeID / 2.0, 2); // m^2
  float flowVel    = flowRate / area; // m/s
  float rpm        = (flowVel / (2.0 * PI * critR)) * 60.0; // rev/min
  float stepsPerSec = (rpm / 60.0) * 200.0 * 32;  // steps/s
  return stepsPerSec;
}

void handleSerial() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      stringComplete = true;
    } else {
      inputString += c;
    }
  }
}

// ---------------------------------------------------------------
// SERIAL BUFFER
// ---------------------------------------------------------------
//String inputString = "";
//bool stringComplete = false;

// ---------------------------------------------------------------
// ACK FUNCTION (NEW)
// ---------------------------------------------------------------
void sendACK(String cmd, float val) {
  Serial.print("ACK,");
  Serial.print(cmd);
  Serial.print(",");
  Serial.println(val);
}

// ---------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Arduino Ready");

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
  driver1.rms_current(100);
  driver1.microsteps(32);
  delay(200);
  //uart1.end();

 /* Serial.print("Connection: ");
  Serial.println(driver1.test_connection());
  Serial.print("Microsteps: ");
  Serial.println(driver1.microsteps());*/

  uart2.begin(115200);
  driver2.begin();
  driver2.toff(5);
  driver2.rms_current(100);
  driver2.microsteps(32);
  delay(200);
  //uart2.end();

  /*Serial.print("Connection: ");
  Serial.println(driver2.test_connection());
  Serial.print("Microsteps: ");
  Serial.println(driver2.microsteps());*/

  uart3.begin(115200);
  driver3.begin();
  driver3.toff(5);
  driver3.rms_current(100);
  driver3.microsteps(32);
  delay(200);
  //uart3.end();

  /*Serial.print("Connection: ");
  Serial.println(driver3.test_connection());
  Serial.print("Microsteps: ");
  Serial.println(driver3.microsteps());*/

  Serial.print("Driver1 current: "); Serial.println(driver1.rms_current());
  Serial.print("Driver2 current: "); Serial.println(driver2.rms_current());
  Serial.print("Driver3 current: "); Serial.println(driver3.rms_current());

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

  handleSerial();
  // Handle serial commands from UI
  if (stringComplete) {
    int commaIndex = inputString.indexOf(',');

    if (commaIndex > 0) {
      String cmd    = inputString.substring(0, commaIndex);
      String valStr = inputString.substring(commaIndex + 1);
      float val     = valStr.toFloat();

      if (cmd == "T") {
        targetTemp = val;
        sendACK(cmd, val);
      }
      else if (cmd == "F1") {
        flowRate1    = val; // microliter/hr
        sendACK(cmd, val);
        stepsPerSec1 = calculateSpeed(flowRate1);
        pump1.setSpeed(stepsPerSec1);
      }
      else if (cmd == "F2") {
        flowRate2    = val; // microliter/hr
        sendACK(cmd, val);
        stepsPerSec2 = calculateSpeed(flowRate2);
        pump2.setSpeed(stepsPerSec2);
      }
      else if (cmd == "F3") {
        flowRate3    = val; // microliter/hr
        sendACK(cmd, val);
        stepsPerSec3 = calculateSpeed(flowRate3);
        pump3.setSpeed(stepsPerSec3);
      }
      else if (cmd == "P1") {
        pump1_on = (val > 0);
        digitalWrite(enPins[0], pump1_on ? LOW : HIGH);
        sendACK(cmd, val);
      }
      else if (cmd == "P2") {
        pump2_on = (val > 0);
        digitalWrite(enPins[1], pump2_on ? LOW : HIGH);
        sendACK(cmd, val);
      }
      else if (cmd == "P3") {
        pump3_on = (val > 0);
        digitalWrite(enPins[2], pump3_on ? LOW : HIGH);
        sendACK(cmd, val);
      }
    }

    inputString    = "";
    stringComplete = false;
  }

  // Temperature reading every 500ms
  if (millis() - lastTemp > tempInterval) {
    lastTemp = millis();
    tempC    = readTempCelsius();
    tempC    = 0.0629*tempC + 29.7; //correction for temperature decline between thermistor and top reservoir

    // Send temp to UI
    //Serial.print("TEMP,");
    Serial.println(tempC);
  }

  float error = targetTemp - tempC;
  float kp = 25; 
  int pwmValue = constrain(kp*error, 0, 51);

  // Heater hysteresis control
  if (tempC < targetTemp - 0.2) {
    digitalWrite(heaterPin, pwmValue);
  }
  if (tempC > targetTemp + 0.2) {
    digitalWrite(heaterPin, LOW);
  }

  pump1.setSpeed(stepsPerSec1);
  pump2.setSpeed(stepsPerSec2);
  pump3.setSpeed(stepsPerSec3);
  
  // Run pumps
  if (pump1_on) pump1.runSpeed();
  if (pump2_on) pump2.runSpeed();
  if (pump3_on) pump3.runSpeed();
}
