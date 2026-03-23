#include "config.h"
#include "globals.h"

ir_array_t qtrArray = {
  QTR_PINS,   
  { 0 },   
  { false }, 
  0,             
  QTR_THRESHOLDS,
  QTR_WEIGHTS
};

void readIRArray() {
  long numerator = 0;
  long denominator = 0;

  for (int i = 0; i < NUM_IR; i++) {
    qtrArray.values[i] = analogRead(qtrArray.pins[i]);
    qtrArray.detected[i] = (qtrArray.values[i] < qtrArray.thresholds[i]);

    numerator += (long)qtrArray.values[i] * i * 1000;
    denominator += qtrArray.values[i];
  }

  qtrArray.position = (denominator == 0) ? -1 : (numerator / denominator);
}

bool allWhite() {
  readIRArray(); 

  for (int i = 0; i < NUM_IR; i++) {
    if (!qtrArray.detected[i]) {  
      return false;               
    }
  }
  return true;  
}

bool leftWhite() {
  readIRArray();  

  for (int i = 0; i < NUM_IR / 2; i++) {
    if (!qtrArray.detected[i]) { 
      return false;              
    }
  }
  return true; 
}