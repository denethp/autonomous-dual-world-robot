#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"

extern motor_t leftMotor;
extern motor_t rightMotor;

extern ir_array_t qtrArray;
extern int distMoved;
extern bool skipBoxes;

void leftA_ISR();
void leftB_ISR();
void rightA_ISR();
void rightB_ISR();

void resetEncoders();
void setupMotors();
void setupEncoders();
void setMotorPWM(motor_t &motor, int pwm);
void moveForwardLineDistNoStop(int targetPWM, int dist);
void showText(String text, int size = 2, int x = 0, int y = 20);
void showTwoLines(String line1, String line2, int size = 2);
void lineFollow_leftWhite(int targetPWM, int detectionDeadband = 0);

void collectBox(int targetPWM);
void setupTOF();

#endif