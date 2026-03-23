int bufferRightCallibrate[WINDOW_SIZE] = { 0 };
int indexRightCallibrate = 0;
long sumRightCallibrate = 0;
int countRightCallibrate = 0;

float getAverageRightCallibrate(int newValue) {
  sumRightCallibrate -= bufferRightCallibrate[indexRightCallibrate];
  bufferRightCallibrate[indexRightCallibrate] = newValue;
  sumRightCallibrate += newValue;

  indexRightCallibrate = (indexRightCallibrate + 1) % WINDOW_SIZE;

  if (countRightCallibrate < WINDOW_SIZE) countRightCallibrate++;

  return (float)sumRightCallibrate / countRightCallibrate;
}

int bufferLeftCallibrate[WINDOW_SIZE] = { 0 };
int indexLeftCallibrate = 0;
long sumLeftCallibrate = 0;
int countLeftCallibrate = 0;

float getAverageLeftCallibrate(int newValue) {
  sumLeftCallibrate -= bufferLeftCallibrate[indexLeftCallibrate];
  bufferLeftCallibrate[indexLeftCallibrate] = newValue;
  sumLeftCallibrate += newValue;

  indexLeftCallibrate = (indexLeftCallibrate + 1) % WINDOW_SIZE;

  if (countLeftCallibrate < WINDOW_SIZE) countLeftCallibrate++;

  return (float)sumLeftCallibrate / countLeftCallibrate;
}

void callibrateSharpIR() {
  while (true) {
    int leftIR = analogRead(LEFT_IR);
    int rightIR = analogRead(RIGHT_IR);

    int leftAvg = getAverageLeftCallibrate(leftIR);
    int rightAvg = getAverageRightCallibrate(rightIR);
    
    showTwoLines(String(leftAvg), String(rightAvg));
    delay(5);
  }
}

int ir_min[NUM_IR];
int ir_max[NUM_IR];

void calibrateIR() {
  Serial.println("Starting IR calibration...");

  for (int i = 0; i < NUM_IR; i++) {
    ir_min[i] = 1023;
    ir_max[i] = 0;
  }

  for (int t = 0; t < CALIBRATION_SAMPLES; t++) {
    for (int i = 0; i < NUM_IR; i++) {
      int val = analogRead(qtrArray.pins[i]);

      if (val < ir_min[i]) ir_min[i] = val;
      if (val > ir_max[i]) ir_max[i] = val;
    }

    delay(2);
  }

  for (int i = 0; i < NUM_IR; i++) {
    qtrArray.thresholds[i] = (ir_min[i] + ir_max[i]) / 2;
  }

  // for (int i = 0; i < NUM_IR; i++) {
  //   Serial.print("IR ");
  //   Serial.print(i);
  //   Serial.print(" Min: "); Serial.print(ir_min[i]);
  //   Serial.print(" Max: "); Serial.print(ir_max[i]);
  //   Serial.print(" Thr: "); Serial.println(qtrArray.thresholds[i]);
  // }
}