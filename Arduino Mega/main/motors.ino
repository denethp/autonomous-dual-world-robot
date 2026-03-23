#include "globals.h"
#include "config.h"

motor_t leftMotor;
motor_t rightMotor;

void setMotorPWM(motor_t &motor, int pwm) {
  pwm = constrain(pwm, -PWM_MAX, PWM_MAX);
  motor.pwm = pwm;

  if (pwm > 0) {
    digitalWrite(motor.pin_m1, HIGH);
    digitalWrite(motor.pin_m2, LOW);
    analogWrite(motor.pwm_pin, pwm);
  } else if (pwm < 0) {
    digitalWrite(motor.pin_m1, LOW);
    digitalWrite(motor.pin_m2, HIGH);
    analogWrite(motor.pwm_pin, -pwm);
  } else {
    digitalWrite(motor.pin_m1, HIGH);
    digitalWrite(motor.pin_m2, HIGH);
    analogWrite(motor.pwm_pin, 0);
  }
}

void leftA_ISR() {
  int a = digitalRead(leftMotor.enc_a);
  int b = digitalRead(leftMotor.enc_b);
  if (a == b) leftMotor.enc++;
  else leftMotor.enc--;
}

void leftB_ISR() {
  int a = digitalRead(leftMotor.enc_a);
  int b = digitalRead(leftMotor.enc_b);
  if (a != b) leftMotor.enc++;
  else leftMotor.enc--;
}

void rightA_ISR() {
  int a = digitalRead(rightMotor.enc_a);
  int b = digitalRead(rightMotor.enc_b);
  if (a == b) rightMotor.enc++;
  else rightMotor.enc--;
}

void rightB_ISR() {
  int a = digitalRead(rightMotor.enc_a);
  int b = digitalRead(rightMotor.enc_b);
  if (a != b) rightMotor.enc++;
  else rightMotor.enc--;
}

void resetEncoders() {
  noInterrupts();
  leftMotor.enc = 0;
  rightMotor.enc = 0;
  interrupts();
}

void setupMotors() {
  leftMotor.pin_m1 = LEFT_M1;
  leftMotor.pin_m2 = LEFT_M2;
  leftMotor.pwm_pin = LEFT_PWM;

  rightMotor.pin_m1 = RIGHT_M1;
  rightMotor.pin_m2 = RIGHT_M2;
  rightMotor.pwm_pin = RIGHT_PWM;

  pinMode(leftMotor.pin_m1, OUTPUT);
  pinMode(leftMotor.pin_m2, OUTPUT);
  pinMode(leftMotor.pwm_pin, OUTPUT);

  pinMode(rightMotor.pin_m1, OUTPUT);
  pinMode(rightMotor.pin_m2, OUTPUT);
  pinMode(rightMotor.pwm_pin, OUTPUT);
}

void setupEncoders() {
  leftMotor.enc_a = LEFT_ENC_A;
  leftMotor.enc_b = LEFT_ENC_B;
  rightMotor.enc_a = RIGHT_ENC_A;
  rightMotor.enc_b = RIGHT_ENC_B;

  pinMode(leftMotor.enc_a, INPUT_PULLUP);
  pinMode(leftMotor.enc_b, INPUT_PULLUP);

  pinMode(rightMotor.enc_a, INPUT_PULLUP);
  pinMode(rightMotor.enc_b, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(leftMotor.enc_a), leftA_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(leftMotor.enc_b), leftB_ISR, CHANGE);

  attachInterrupt(digitalPinToInterrupt(rightMotor.enc_a), rightA_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rightMotor.enc_b), rightB_ISR, CHANGE);
}