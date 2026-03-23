#include "config.h"
#include "globals.h"


void setup() {

  setupMotors();
  setupEncoders();
  setupTOF();
  initOLED();
  pinMode(LIMIT_SW, INPUT_PULLUP);

  Serial.begin(9600);
  Serial2.begin(9600);

  eliminationTask();
}

void loop() {

}

/*
---x Shut Pin Scanner--- 


#include <Wire.h>

#define TOF_ADDR 0x29

// Digital pins to scan (exclude 0,1,20,21)
int digitalPins[] = {
  2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
  14, 15, 16, 17, 18, 19,
  22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
  42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53
};

// Analog pins A0–A15
int analogPins[] = {
  A0, A1, A2, A3, A4, A5, A6, A7,
  A8, A9, A10, A11, A12, A13, A14, A15
};

int numDPins = sizeof(digitalPins) / sizeof(digitalPins[0]);
int numAPins = sizeof(analogPins) / sizeof(analogPins[0]);

bool isToFAlive() {
  Wire.beginTransmission(0x29);
  return (Wire.endTransmission() == 0);
}

void turnAllLow() {
  for (int i = 0; i < numDPins; i++) {
    pinMode(digitalPins[i], OUTPUT);
    digitalWrite(digitalPins[i], LOW);
  }
  for (int i = 0; i < numAPins; i++) {
    pinMode(analogPins[i], OUTPUT);
    digitalWrite(analogPins[i], LOW);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  Serial.println("=== MEGA SHUT PIN SCAN ===");

  turnAllLow();  // turn OFF all sensors
  delay(300);
}

void loop() {

  Serial.println("\n--- Digital Pins ---");

  for (int i = 0; i < numDPins; i++) {
    int pin = digitalPins[i];

    turnAllLow();

    digitalWrite(pin, HIGH);  // turn ON one sensor
    delay(100);

    if (isToFAlive()) {
      Serial.print("🎯 SHUT PIN FOUND (D): ");
      Serial.println(pin);
    }

    delay(150);
  }

  Serial.println("\n--- Analog Pins ---");

  for (int i = 0; i < numAPins; i++) {
    int pin = analogPins[i];

    turnAllLow();

    digitalWrite(pin, HIGH);
    delay(100);

    if (isToFAlive()) {
      Serial.print("🎯 SHUT PIN FOUND (A): ");
      Serial.println(pin);
    }

    delay(150);
  }

  Serial.println("\nScan complete...\n");
  delay(4000);
}
*/
