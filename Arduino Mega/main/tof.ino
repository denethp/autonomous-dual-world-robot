#include "config.h"
#include "globals.h"

tof_t tofSensors[NUM_TOF];

void setupTOF() {
  uint8_t pins[NUM_TOF] = TOF_XSHUT_PINS;
  uint8_t addrs[NUM_TOF] = TOF_ADDRESSES;

  for (int i = 0; i < NUM_TOF; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
  delay(10);

  for (int i = 0; i < NUM_TOF; i++) {
    digitalWrite(pins[i], HIGH);
    delay(10);

    tofSensors[i].xshutPin = pins[i];
    tofSensors[i].address = addrs[i];

    if (!tofSensors[i].sensor.begin()) {
      tofSensors[i].initialized = false;
      Serial.print("TOF ");
      Serial.print(i);
      Serial.println(" failed");
    } else {
      tofSensors[i].sensor.setAddress(addrs[i]);
      tofSensors[i].initialized = true;
      Serial.print("TOF ");
      Serial.print(i);
      Serial.println(" OK");
    }
  }
}

int readTOF(int id) {
  if (id < 0 || id >= NUM_TOF) return -1;
  if (!tofSensors[id].initialized) return -1;

  VL53L0X_RangingMeasurementData_t measure;
  tofSensors[id].sensor.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    return measure.RangeMilliMeter;
  } else {
    return 65355; // invalid
  }
}