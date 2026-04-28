#include <AccelStepper.h>
#include <TMCStepper.h>
#include <SoftwareSerial.h>
#include <math.h>

#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00

const int heaterPin = 3; // CHANGE TO 1
const int tempPin   = A0;

/*
READ THIS
i'm planning on changing the wiring diagram from report 2 
to follow the pins in the commented code. it will be cleaner.
uart pins also cannot use pins 1-2 because of the arduino rx/tx pins 
are wired to 1/2 and it will break. 
(the driver uses a one-wire uart, so both rx and tx are in the same uart pin)
*/
const int dirPins[] = {6, 9, 12}; // CHANGE TO {2, 6, 10}
const int stepPins[]  = {7, 10, 13}; // CHANGE TO {3, 7, 11}
const int uartPins[] = {1, 2, 5}; // CHANGE TO {4, 8, 12} // CANNOT BE PINS 1-2
const int enPins[] = {4, 3, 8}; // CHANGE TO {5, 9, 13}


const int   R0  = 10000;       // Reference resistance (ohms)
const float Tn  = 25.0;        // Nominal temperature (Celsius)
const int   Rn  = 10000;       // Nominal resistance (ohms)
const int   B   = 3950;        // Beta value


const float tubeID = 0.0005;   // Tube inner diameter (meters)
const float critR  = 0.00717;  // Pump radius along which tubing runs (meters)

//initialize tkinter communication
String inputString = "";
bool stringComplete = false;      

float targetTemp = 37.0;
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


//initialize pumps +
SoftwareSerial uart1(uartPins[0], uartPins[0]);  // RX and TX on SAME PIN (conceptual single-wire)
TMC2209Stepper driver1(&uart1, R_SENSE, DRIVER_ADDRESS);

SoftwareSerial uart2(uartPins[1], uartPins[1]);  
TMC2209Stepper driver2(&uart2, R_SENSE, DRIVER_ADDRESS);

SoftwareSerial uart3(uartPins[2], uartPins[2]);  
TMC2209Stepper driver3(&uart3, R_SENSE, DRIVER_ADDRESS);

AccelStepper pump1(AccelStepper::DRIVER, stepPins[0], dirPins[0]);
AccelStepper pump2(AccelStepper::DRIVER, stepPins[1], dirPins[1]);
AccelStepper pump3(AccelStepper::DRIVER, stepPins[2], dirPins[2]);


// thermistor reading function
float readTempCelsius() {
  int   adc     = analogRead(tempPin);
  float v       = adc * (5.0 / 1023.0);              
  float Rt      = R0 * ((5.0 / v) - 1.0);            
  float T_kelvin = 1.0 / ((1.0 / (Tn + 273.15)) + (1.0 / B) * log(Rt / Rn)); 
  return T_kelvin - 273.15;
}

//calculate required steps per second based on flow rate
float calculateSpeed(float flowRate) {
  if (flowRate == 0) {
    return 0;
  }
  
  float area    = PI * pow(tubeID / 2.0, 2); // Cross-sectional area of tube (m^2)
  float flowVel = flowRate / area;            // Linear velocity of fluid (m/s)
  float rpm     = (flowVel / (2.0 * PI * critR)) * 60.0; // Motor RPM
  float stepsPerSec = (rpm / 60.0) * 200.0 * 256;  // Steps per second (200 steps/rev)

  return stepsPerSec;
}

//auto trigger that senses ui input
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


void setup() {
  //serial connection for ui
  Serial.begin(115200);
  inputString.reserve(50);


  //define pin modes
  pinMode(heaterPin, OUTPUT);
  pinMode(tempPin, INPUT);

  pinMode(dirPins[0], OUTPUT);
  pinMode(dirPins[1], OUTPUT);
  pinMode(dirPins[2], OUTPUT);

  pinMode(stepPins[0], OUTPUT);
  pinMode(stepPins[1], OUTPUT);
  pinMode(stepPins[2], OUTPUT);
  
  pinMode(enPins[0], OUTPUT);
  pinMode(enPins[1], OUTPUT);
  pinMode(enPins[2], OUTPUT);

  //initialize heater as off
  digitalWrite(heaterPin, LOW);

  //microsteps 256 + assumes 0.6 A current, can mess with amperage later
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


  //turns pumps on
  digitalWrite(enPins[0], LOW);
  digitalWrite(enPins[1], LOW);
  digitalWrite(enPins[2], LOW);

  //all pumps rotate clockwise
  digitalWrite(dirPins[0], HIGH);
  digitalWrite(dirPins[1], HIGH);
  digitalWrite(dirPins[2], HIGH);


}

void loop() {

  if (stringComplete) {

    int commaIndex = inputString.indexOf(',');

    if (commaIndex > 0) {

      String cmd = inputString.substring(0, commaIndex);
      String valStr = inputString.substring(commaIndex + 1);
      float val = valStr.toFloat();

      // ---- TEMPERATURE ----
      if (cmd == "T") {
        targetTemp = val;
      }

      // ---- FLOW ----
      else if (cmd == "F1") {
        flowRate1 = val;
        stepsPerSec1 = calculateSpeed(flowRate1);
      }

      else if (cmd == "F2") {
        flowRate2 = val;
        stepsPerSec2 = calculateSpeed(flowRate2);

      }

      else if (cmd == "F3") {
        flowRate3 = val;
        stepsPerSec3 = calculateSpeed(flowRate3);
      }

      // ---- PUMPS ----
      else if (cmd == "P1") {
        digitalWrite(enPins[0], val > 0 ? LOW : HIGH);
        pump1_on = (val>0);
      }

      else if (cmd == "P2") {
        digitalWrite(enPins[1], val > 0 ? LOW : HIGH);
        pump2_on = (val>0);
      }

      else if (cmd == "P3") {
        digitalWrite(enPins[2], val > 0 ? LOW : HIGH);
        pump3_on = (val>0);
      }
    }

    inputString = "";
    stringComplete = false;
  }

  //call thermistor reading every 500 ms
  if (millis() - lastTemp > tempInterval) {
    lastTemp = millis();
    tempC = readTempCelsius();
  }

  //hysteresis band control
  if (tempC < targetTemp - 0.2) {
    digitalWrite(heaterPin, HIGH); // Heater on
  } 
  if (tempC > targetTemp + 0.2) {
    digitalWrite(heaterPin, LOW); // Heater off
  }

  //set pump speed
  pump1.setSpeed(stepsPerSec1);
  pump2.setSpeed(stepsPerSec2);
  pump3.setSpeed(stepsPerSec3);

  //run pump
  pump1.runSpeed();
  pump2.runSpeed();
  pump3.runSpeed();
}
