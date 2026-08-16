#ifndef CONFIG_H
#define CONFIG_H

#define SERIAL_BAUD 115200

// PWM Settings
#define PWM_FREQ 15000
#define PWM_MAX 255

// Motor Settings
#define LEFT_M1 47
#define LEFT_M2 49
#define LEFT_PWM 4

#define RIGHT_M1 51
#define RIGHT_M2 53
#define RIGHT_PWM 5

// Encoder Settings
#define LEFT_ENC_A 19
#define LEFT_ENC_B 18

#define RIGHT_ENC_A 3
#define RIGHT_ENC_B 2

#define CPR 900

// QTR Sensor Settings
#define NUM_IR 8
#define CALIBRATION_SAMPLES 2000
#define QTR_PINS \
  { A8, A9, A10, A11, A12, A13, A14, A15 }
#define QTR_THRESHOLDS \
  { 500, 500, 500, 500, 500, 500, 500, 500 }
#define QTR_WEIGHTS \
  { -16, -9, -4, -1, 1, 4, 9, 16 }

// Control loop settings
#define LOOP_DELAY 1

#define TRACK_WIDTH 147
#define WHEEL_DIAMETER 64.5

#define DECELERATION_DIST 50
#define ACCELERATION_DIST 50

#define MIN_PWM 30
#define SLIPPING_OFFSET 0

#define BASE_SPEED 50

#define NUM_TOF 6

#define TOF_XSHUT_PINS \
  { 33, 31, 29, 35, 27, 37 }  
#define TOF_ADDRESSES \
  { 0x30, 0x31, 0x32, 0x33, 0X34, 0x35 }


#define L1_LEFT_TOF 0
#define L1_RIGHT_TOF 1
#define L3_FLEFT_TOF 5
#define L3_FRIGHT_TOF 4
#define L3_LEFT_TOF 3
#define L3_RIGHT_TOF 2

#define RIGHT_TOF_BOX_MEAS 495
#define LEFT_TOF_BOX_MEAS 485

#define RIGHT_SHARP_BOX_MEAS 250
#define LEFT_SHARP_BOX_MEAS 166
#define FRONT_SHARP_BOX_MEAS 50

#define BOX_ON_LEFT 0
#define BOX_ON_RIGHT 1

#define LEFT_IR A7
#define RIGHT_IR A6
#define FRONT_IR A2

#define WINDOW_SIZE 10

#define TURN_LEFT_PWM 60
#define TURN_RIGHT_PWM 60
#define TURN_BACK_PWM 60

#define BASE_PWM 45
#define SLOW_BASE_PWM 50

#define LIMIT_SW 25

#define BOX_DETECTION_DEADBAND 70
#define J2_LINE_DETECTION_DEADBAND 300
#define J_LINE_TURN_DIST 70

#endif