#ifndef TYPES_H

#include "config.h" 
#include <Adafruit_VL53L0X.h>

typedef struct {
  int pin_m1;
  int pin_m2;
  int pwm_pin;

  int enc_a;
  int enc_b;

  volatile long enc;
  volatile double pwm;
} motor_t;

typedef struct {
    int pins[NUM_IR];              
    uint16_t values[NUM_IR];      
    bool detected[NUM_IR];  
    int position;                  
    uint16_t thresholds[NUM_IR];   
    int16_t weights[NUM_IR];
} ir_array_t;

typedef struct {
  uint8_t xshutPin;
  uint8_t address;
  Adafruit_VL53L0X sensor;
  bool initialized;
} tof_t;
#endif