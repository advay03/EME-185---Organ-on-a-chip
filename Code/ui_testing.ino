#include <AccelStepper.h>
#include <TMCStepper.h>
#include <SoftwareSerial.h>
#include <math.h>

#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00

// ---------------------------------------------------------------
// PIN DEFINITIONS
// ---------------------------------------------------------------
const int heaterPin = 2;
const int tempPin   = A0;

// const int dirPins[]  = {2, 6, 10};
const int stepPins[] = {3, 7, 11};
const int uartPins[] = {4, 8, 12};
const int enPins[]   = {5, 9, 13};

// ---------------------------------------------------------------
// THERMISTOR CONSTANTS
// ---------------------------------------------------------------
const int   R0 = 10000;
const float Tn = 25.0;
const int   Rn = 10000;
const int   B  = 3950;

// ---------------------------------------------------------------
// CONTROL VARIABLES
// ---------------------------------------------------------------
float targetTemp = 37.0;

float flowRate1 = 1.67e-11;
float flowRate2 = 1.67e-11;
float flowRate3 = 1.67e-11;

bool pump1_on = false;
bool pump2_on = false;
bool pump3_on = false;

float tempC = 0;

// ---------------------------------------------------------------
// TIMERS
// ---------------------------------------------------------------
unsigned long lastDebug = 0;
unsigned long lastTempUpdate = 0;

const unsigned long debugInterval = 500;
const unsigned long tempInterval  = 500;

// ---------------------------------------------------------------
// SERIAL BUFFER
// ---------------------------------------------------------------
String inputString = "";
bool stringComplete = false;

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
// TEMP FUNCTION
// ---------------------------------------------------------------
float readTempCelsius() {
  int adc = analogRead(tempPin);
  float v = adc * (5.0 / 1023.0);

  if (v <= 0.01) return -100;

  float Rt = R0 * ((5.0 / v) - 1.0);

  float T_kelvin = 1.0 / (
    (1.0 / (Tn + 273.15)) +
    (1.0 / B) * log(Rt / Rn)
  );

  return T_kelvin - 273.15;
}

// ---------------------------------------------------------------
// SPEED FUNCTION
// ---------------------------------------------------------------
float calculateSpeed(float flowRate_ul_hr) {
  if (flowRate_ul_hr == 0) return 0;

  float flowRate = flowRate_ul_hr / 3.6E12;
  float area     = PI * pow(0.0005 / 2.0, 2);
  float flowVel  = flowRate / area;

  float rpm = (flowVel / (2.0 * PI * 0.00717)) * 60.0;

  return (rpm / 60.0) * 200.0 * 32;
}

// ---------------------------------------------------------------
// SERIAL HANDLER
// ---------------------------------------------------------------
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
// SETUP
// ---------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Arduino Ready");

  pinMode(heaterPin, OUTPUT);
  pinMode(tempPin, INPUT);

  digitalWrite(heaterPin, LOW);
}

// ---------------------------------------------------------------
// LOOP
// ---------------------------------------------------------------
void loop() {

  handleSerial();

  // -----------------------------------------------------------
  // COMMAND PARSING + ACK
  // -----------------------------------------------------------
  if (stringComplete) {

    int commaIndex = inputString.indexOf(',');

    if (commaIndex > 0) {

      String cmd = inputString.substring(0, commaIndex);
      String valStr = inputString.substring(commaIndex + 1);
      float val = valStr.toFloat();

      // ------------------- TEMP SET -------------------
      if (cmd == "T") {
        targetTemp = val;
        sendACK(cmd, val);
      }

      // ------------------- FLOW RATES -------------------
      else if (cmd == "F1") {
        flowRate1 = val;
        sendACK(cmd, val);
      }

      else if (cmd == "F2") {
        flowRate2 = val;
        sendACK(cmd, val);
      }

      else if (cmd == "F3") {
        flowRate3 = val;
        sendACK(cmd, val);
      }

      // ------------------- PUMPS -------------------
      else if (cmd == "P1") {
        pump1_on = val > 0;
        digitalWrite(enPins[0], pump1_on ? LOW : HIGH);
        sendACK(cmd, val);
      }

      else if (cmd == "P2") {
        pump2_on = val > 0;
        digitalWrite(enPins[1], pump2_on ? LOW : HIGH);
        sendACK(cmd, val);
      }

      else if (cmd == "P3") {
        pump3_on = val > 0;
        digitalWrite(enPins[2], pump3_on ? LOW : HIGH);
        sendACK(cmd, val);
      }
    }

    inputString = "";
    stringComplete = false;
  }

  // -----------------------------------------------------------
  // TEMP UPDATE
  // -----------------------------------------------------------
  if (millis() - lastTempUpdate > tempInterval) {
    lastTempUpdate = millis();

    tempC = readTempCelsius();

    Serial.print("TEMP,");
    Serial.println(tempC);
  }

  // -----------------------------------------------------------
  // DEBUG STATUS
  // -----------------------------------------------------------
  if (millis() - lastDebug > debugInterval) {
    lastDebug = millis();

    Serial.print("STATUS | Tset:");
    Serial.print(targetTemp);

    Serial.print(" | F1:");
    Serial.print(flowRate1);

    Serial.print(" F2:");
    Serial.print(flowRate2);

    Serial.print(" F3:");
    Serial.print(flowRate3);

    Serial.print(" | P1:");
    Serial.print(pump1_on);

    Serial.print(" P2:");
    Serial.print(pump2_on);

    Serial.print(" P3:");
    Serial.println(pump3_on);
  }

  // -----------------------------------------------------------
  // HEATER CONTROL
  // -----------------------------------------------------------
  if (tempC < targetTemp - 0.2) {
  digitalWrite(heaterPin, HIGH);
  Serial.println("HEATER ON");
  }
  if (tempC > targetTemp + 0.2) {
  digitalWrite(heaterPin, LOW);
  Serial.println("HEATER OFF");
  }

}
