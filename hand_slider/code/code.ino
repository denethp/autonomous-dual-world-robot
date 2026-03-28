#define PWMA 5
#define AIN1 8
#define AIN2 7

#define SHDN1 4
#define SHDN2 6
// #define STBY 
     
#define LIMIT_SWITCH 12

#include <Servo.h>
#include <Ramp.h>
#include <Wire.h>
#include "Adafruit_VL6180X.h"

volatile long encoderCount = 0;
unsigned long lastPrintTime = 0;

long lastEncoder = 0;
unsigned long lastCheckTime = 0;

Servo myServo;
Servo base;
Servo kalle;
rampLong baseRamp;

long positions[4] = {0, 5500, 9233, 13300};

int box_num = 0;

long targetPosition = 0;
bool moveMotor = false;
bool homing = false;
bool box_flag = false;
bool full_flag =false;
bool relese_flag = false;
int relese_step = 0; 
int box_step = 0;


Adafruit_VL6180X sensor1 = Adafruit_VL6180X();
Adafruit_VL6180X sensor2 = Adafruit_VL6180X();

void setup() {
  Serial.begin(9600);
  Wire.begin();
  baseRamp.go(2600, 1500, NONE, ONCEFORWARD);

  pinMode(SHDN1, OUTPUT);
  pinMode(SHDN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  digitalWrite(SHDN1, LOW);
  digitalWrite(SHDN2, LOW);
  delay(10);
  digitalWrite(SHDN1, HIGH);
  delay(10);
  if (!sensor1.begin()) {
    Serial.println("Sensor1 failed");
    
  }
  sensor1.setAddress(0x30);

  // === SENSOR 2 ===
  digitalWrite(SHDN2, HIGH);
  delay(10);

  if (!sensor2.begin()) {
    Serial.println("Sensor2 failed");
    
  }
  sensor2.setAddress(0x31);

  // Serial.println("Both sensors ready");

  myServo.attach(11);  
  kalle.attach(10);
  base.attach(9);
  
  pinMode(3, INPUT);
  pinMode(2, INPUT);
  pinMode(LIMIT_SWITCH, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(2), readEncoder, CHANGE);

  Serial.println("SLAVE READY");
  base_up();
  relese();
  kalla_open();
  delay(1000);
  
}


void loop() {
  readCommand();

  if (homing) {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, 250);

    if (digitalRead(LIMIT_SWITCH) == LOW) {
      stopMotor();
      encoderCount = 0;
      homing = false;
    }
  }
  else if (moveMotor) {
    move_motor();
  }
  else if (box_flag) {
    box_sequence();
  }
  else if(relese_flag){
    relese_sequence();
  }


  if (millis() - lastPrintTime > 200) {
    lastPrintTime = millis();
  }
}

void readCommand() {
  if (Serial.available()) {
    char cmd = Serial.read();
    Serial.println(cmd);
    

    switch (cmd) {

      case 'H':
        homing = true;
        break;

      case 'B':
        box_flag = true;
        box_step = 0;
        break;

      case 'R':
        relese_flag = true;
        relese_step = 0;     
        break;
      case 'U':
        base_up();
        Serial.println("DONE");
        break;
      case 'D':
        base_down();
        Serial.println("DONE");
        break;
      case 'O':
        kalla_open();
        Serial.println("DONE");
      break;
      case 'G':
        grab();
        Serial.println("DONE");
      break;
      case 'E':
        
        Serial.println("DONE");
        box_flag = true;
        box_step = 3;
      break;      

      case '1':
      case '2':
      case '3':
      case '4': {
        int index = cmd - '1';
        targetPosition = positions[index];
        moveMotor = true;
        break;
      }
    }
  }
}


void move_motor() {
  long error = targetPosition - encoderCount;

  // Stall detection
  if (millis() - lastCheckTime > 200) {

    if (abs(encoderCount - lastEncoder) < 2) {

      stopMotor();
      delay(500);

      // BURST
      if (error > 0) {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
        analogWrite(PWMA, 255);
      } else {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        analogWrite(PWMA, 255);
      }

      delay(100);
      stopMotor();
    }

    lastEncoder = encoderCount;
    lastCheckTime = millis();
  }

  // Normal control
  if (abs(error) < 5) {
    stopMotor();
    moveMotor = false;
  } else {
    int speed = constrain(abs(error), 180, 250);

    if (error > 0) {
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      analogWrite(PWMA, speed);
    } else {
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      analogWrite(PWMA, speed);
    }
  }
}


void box_sequence() {

  switch (box_step) {

    case 0:
      
      homing = true;
      box_step++;
      break;

    case 1:
      if (!homing) {
        base_down();
        box_step++;
      }
      break;

    case 2:
      grab();
      box_step++;
      break;

    case 3:
      base_up();
      box_step++;
      break;
    case 4:
      
      homing = true;
      box_step++;
      break;
   
   
    case 5:
 if (!homing) {
      relese();
      delay(400);
      check_close();
      if (full_flag) {
      
       kalle.write(30);
       delay(500);

      }
      box_step++;}
      
      break;
      

    case 6:
      if (full_flag) {
        base_up();

        box_flag = false;
        box_num++;
        Serial.println("DONE");
        break;
      }
      targetPosition = positions[1];
      moveMotor = true;
      box_step++;
      break;

    case 7:
      if (!moveMotor) {
        box_flag = false;
        box_num++;
        Serial.println("DONE");
      }
      break;
  }
}

void relese_sequence() {

  switch (relese_step) {

    case 0:
      
      homing = true;
      relese_step++;
      break;

    case 1:
      if (!homing) {
      targetPosition = positions[3];
      moveMotor = true;
      relese_step++;
      }
      break;

    case 2:
      homing = true;
      relese_flag = false;
      box_num=0;
      relese_step++;
      break;

    case 3:

      if (!homing) {
      kalla_open();
      targetPosition = positions[3];
      moveMotor = true;
      relese_flag = false;
      Serial.println("DONE");
      box_num=0;
      }
      break;

  }
}




void readEncoder() {
  int A = digitalRead(3);
  int B = digitalRead(2);

  if (A == B) encoderCount--;
  else encoderCount++;
}

void stopMotor() {
  analogWrite(PWMA, 0);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
}

void relese() {
  myServo.write(60);
  delay(3000);
  myServo.write(90);
}

void grab() {
  myServo.write(120);
  delay(3000);
  myServo.write(90);
}

void base_up() {
  baseRamp.go(400, 1500, QUADRATIC_INOUT, ONCEFORWARD);
  while(!baseRamp.isFinished()){
    base.writeMicroseconds(baseRamp.update());
  }

}

void base_down() {
  // base.writeMicroseconds(2600);
  // delay(800);
  baseRamp.go(2600, 1500, QUADRATIC_INOUT, ONCEFORWARD);
  // base.writeMicroseconds(400);
  // delay(1500);
  while(!baseRamp.isFinished()){
    // Serial.println(baseRamp.update());
    base.writeMicroseconds(baseRamp.update());
  }
}

void kalla_close(){
  
  kalle.write(30);


}
void kalla_open(){
kalle.write(130);
delay(1000);
kalle.write(123);
delay(1000);
kalle.write(130);
}

void check_close() {
  int avg1 = getAvg(sensor1);
  int avg2 = getAvg(sensor2);



  if (avg1 < 40 && avg2 < 40) {
    kalla_close();
    delay(500);
    full_flag = true;
  }
}

int getAvg(Adafruit_VL6180X &sensor) {
  int sum = 0;
  int samples = 5;
  int no_of_sample=0;

  for (int i = 0; i < samples; i++) {
    uint8_t range = sensor.readRange();
    uint8_t status = sensor.readRangeStatus();

    if (status == VL6180X_ERROR_NONE) {
      sum += range;
      no_of_sample+=1;
    } else {
      //i--; // retry this sample
      continue;
    }

    delay(10); // small delay for stability
  }

  return sum / samples;
}



