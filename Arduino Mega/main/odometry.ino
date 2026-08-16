void forwardEncoders(float targetPWM) {
  float basePWM = 0;

  const float Kp = 2;
  const float Kd = 1;

  long prevError = 0;

  while (true) {
    constrain(basePWM, 0, targetPWM);

    noInterrupts();
    long leftEnc = leftMotor.enc;
    long rightEnc = rightMotor.enc;
    interrupts();

    int error = leftEnc - rightEnc;
    int dError = error - prevError;

    float correction = Kp * error + Kd * dError;

    int leftPWM = basePWM + correction;
    int rightPWM = basePWM - correction;

    prevError = error;

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    delay(LOOP_DELAY);
  }
}

void forwardEncoders_dist(float targetPWM, float dist_mm) {
  float basePWM = targetPWM;

  const float Kp = 2;
  const float Kd = 0;

  long prevError = 0;

  float targetCount = distToCounts(dist_mm);

  resetEncoders();

  while (true) {

    noInterrupts();
    long leftEnc = leftMotor.enc;
    long rightEnc = rightMotor.enc;
    interrupts();

    float encAvg = (leftEnc + rightEnc) / 2.0;

    int error = leftEnc - rightEnc;
    int dError = error - prevError;

    float correction = Kp * error + Kd * dError;

    int leftPWM = basePWM + correction;
    int rightPWM = basePWM - correction;

    prevError = error;

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    if (encAvg > targetCount - SLIPPING_OFFSET) {
      setMotorPWM(leftMotor, 0);
      setMotorPWM(rightMotor, 0);
      break;
    }

    delay(LOOP_DELAY);
  }
}

void forwardEncoders_allWhite(float targetPWM) {
  float basePWM = targetPWM;

  const float Kp = 2;
  const float Kd = 1;

  long prevError = 0;

  while (true) {

    noInterrupts();
    long leftEnc = leftMotor.enc;
    long rightEnc = rightMotor.enc;
    interrupts();

    int error = leftEnc - rightEnc;
    int dError = error - prevError;

    float correction = Kp * error + Kd * dError;

    int leftPWM = basePWM + correction;
    int rightPWM = basePWM - correction;

    prevError = error;

    if (allWhite()) {
      setMotorPWM(leftMotor, 0);
      setMotorPWM(rightMotor, 0);
      break;
    }

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    delay(LOOP_DELAY);
  }
}

void backwardEncoders(float targetPWM) {
  float basePWM = targetPWM;

  const float Kp = 2;
  const float Kd = 1;

  long prevError = 0;

  while (true) {

    noInterrupts();
    long leftEnc = leftMotor.enc;
    long rightEnc = rightMotor.enc;
    interrupts();

    int error = leftEnc - rightEnc;
    int dError = error - prevError;

    float correction = Kp * error + Kd * dError;

    int leftPWM = -(basePWM - correction);
    int rightPWM = -(basePWM + correction);

    prevError = error;

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    delay(LOOP_DELAY);
  }
}

void backwardEncoders_limitSW(float targetPWM) {
  float basePWM = targetPWM;

  const float Kp = 2;
  const float Kd = 1;

  long prevError = 0;

  while (true) {

    if (digitalRead(LIMIT_SW) == 0) {
      setMotorPWM(leftMotor, 0);
      setMotorPWM(rightMotor, 0);
      break;
    }

    noInterrupts();
    long leftEnc = leftMotor.enc;
    long rightEnc = rightMotor.enc;
    interrupts();

    int error = leftEnc - rightEnc;
    int dError = error - prevError;

    float correction = Kp * error + Kd * dError;

    int leftPWM = -(basePWM - correction);
    int rightPWM = -(basePWM + correction);

    prevError = error;

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    delay(LOOP_DELAY);
  }
}

void turnLeft(float targetPWM) {
  float basePWM = 0;

  float targetCounts = distToCounts(TRACK_WIDTH * PI / 4);

  const float Kp = 2;
  const float Kd = 1;

  long prevError = 0;

  resetEncoders();

  while (true) {
    noInterrupts();
    long leftEnc = leftMotor.enc;
    long rightEnc = rightMotor.enc;
    interrupts();

    int error = leftEnc + rightEnc;
    int dError = error - prevError;

    float correction = Kp * error + Kd * dError;

    int leftPWM = -targetPWM + correction;
    int rightPWM = targetPWM + correction;

    prevError = error;

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    double left = abs(leftMotor.enc);
    double right = abs(rightMotor.enc);

    if (left >= targetCounts + 17 || right >= targetCounts + 17) {
      setMotorPWM(leftMotor, 0);
      setMotorPWM(rightMotor, 0);
      resetEncoders();

      break;
    }

    delay(LOOP_DELAY);
  }
}

void turnRight(float targetPWM) {
  float basePWM = 0;

  float targetCounts = distToCounts(TRACK_WIDTH * PI / 4);

  const float Kp = 2;
  const float Kd = 1;

  long prevError = 0;

  resetEncoders();

  while (true) {
    noInterrupts();
    long leftEnc = leftMotor.enc;
    long rightEnc = rightMotor.enc;
    interrupts();

    int error = leftEnc + rightEnc;
    int dError = error - prevError;

    float correction = Kp * error + Kd * dError;

    int leftPWM = targetPWM + correction;
    int rightPWM = -targetPWM + correction;

    prevError = error;

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    double left = abs(leftMotor.enc);
    double right = abs(rightMotor.enc);

    if (left >= targetCounts + 20 || right >= targetCounts + 20) {
      setMotorPWM(leftMotor, 0);
      setMotorPWM(rightMotor, 0);
      resetEncoders();
      break;
    }

    delay(LOOP_DELAY);
  }
}

void turnBack(int targetPWM) {
  float basePWM = 0;

  float targetCounts = distToCounts(TRACK_WIDTH * PI / 2);

  const float Kp = 2;
  const float Kd = 1;

  long prevError = 0;

  resetEncoders();

  while (true) {
    noInterrupts();
    long leftEnc = leftMotor.enc;
    long rightEnc = rightMotor.enc;
    interrupts();

    int error = leftEnc + rightEnc;
    int dError = error - prevError;

    float correction = Kp * error + Kd * dError;

    int leftPWM = targetPWM + correction;
    int rightPWM = -targetPWM + correction;

    prevError = error;

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    double left = abs(leftMotor.enc);
    double right = abs(rightMotor.enc);

    if (left >= targetCounts + 43 || right >= targetCounts + 43) {
      setMotorPWM(leftMotor, 0);
      setMotorPWM(rightMotor, 0);

      resetEncoders();
      break;
    }

    delay(LOOP_DELAY);
  }
}

void turnBackFake(int targetPWM) {
  float basePWM = 0;

  float targetCounts = distToCounts(TRACK_WIDTH * PI / 2);

  const float Kp = 2;
  const float Kd = 1;

  long prevError = 0;

  resetEncoders();

  while (true) {
    noInterrupts();
    long leftEnc = leftMotor.enc;
    long rightEnc = rightMotor.enc;
    interrupts();

    int error = leftEnc + rightEnc;
    int dError = error - prevError;

    float correction = Kp * error + Kd * dError;

    int leftPWM = targetPWM + correction;
    int rightPWM = -targetPWM + correction;

    prevError = error;

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    double left = abs(leftMotor.enc);
    double right = abs(rightMotor.enc);

    if (left >= targetCounts + 23 || right >= targetCounts + 23) {
      setMotorPWM(leftMotor, 0);
      setMotorPWM(rightMotor, 0);

      resetEncoders();
      break;
    }

    delay(LOOP_DELAY);
  }
}

void lineFollow(int targetPWM) {
  const int basePWM = targetPWM;

  const float Kp = 3;
  const float Kd = 0.5;
  const float Ki = 0.01;

  long prevError = 0;
  long integral = 0;

  while (true) {
    readIRArray();

    long error = 0;
    int activeSensors = 0;

    for (int i = 0; i < NUM_IR; i++) {
      int val = qtrArray.detected[i] ? 1 : 0;
      error += qtrArray.weights[i] * val;
      activeSensors += val;
    }

    error = error / activeSensors;

    integral += error;
    if (integral > 100) integral = 100;
    if (integral < -100) integral = -100;

    long derivative = error - prevError;
    int correction = Kp * error + Kd * derivative + Ki * integral;

    prevError = error;

    int leftPWM = basePWM + correction;
    int rightPWM = basePWM - +correction;

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    delay(LOOP_DELAY);
  }
}

void lineFollow_dist(int targetPWM, int dist) {
  const int basePWM = targetPWM;

  const float Kp = 3;
  const float Kd = 0.0;
  const float Ki = 0.01;

  long prevError = 0;
  long integral = 0;

  float targetCount = distToCounts(dist);

  resetEncoders();

  while (true) {

    noInterrupts();
    long leftEnc = leftMotor.enc;
    long rightEnc = rightMotor.enc;
    interrupts();

    float encAvg = (leftEnc + rightEnc) / 2.0;


    readIRArray();

    long error = 0;
    int activeSensors = 0;

    for (int i = 0; i < NUM_IR; i++) {
      int detected = qtrArray.detected[i] ? 1 : 0;
      error += qtrArray.weights[i] * detected;
      activeSensors += detected;
    }

    if (activeSensors > 0) {
      error /= activeSensors;
      integral += error;
    } else {
      integral = 0;
    }

    integral = constrain(integral, -100, 100);

    long derivative = error - prevError;
    int correction = Kp * error + Kd * derivative + Ki * integral;

    prevError = error;

    int leftPWM = basePWM - correction;
    int rightPWM = basePWM + correction;

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    if (encAvg > targetCount - SLIPPING_OFFSET) {
      setMotorPWM(leftMotor, 0);
      setMotorPWM(rightMotor, 0);
      break;
    }
  }

  delay(LOOP_DELAY);
}


void lineFollow_leftWhite(int targetPWM, int detectionDeadband = 0) {
  const int basePWM = targetPWM;

  const float Kp = 3;
  const float Kd = 0.0;
  const float Ki = 0.01;

  long prevError = 0;
  long integral = 0;

  resetEncoders();

  while (true) {

    noInterrupts();
    long leftEnc = leftMotor.enc;
    long rightEnc = rightMotor.enc;
    interrupts();

    int avgEnc = (float)(leftEnc + rightEnc) / 2.0;

    if (leftWhite() && avgEnc > distToCounts(detectionDeadband)) {
      break;
    }

    readIRArray();

    long error = 0;
    int activeSensors = 0;

    for (int i = 0; i < NUM_IR; i++) {
      int val = qtrArray.detected[i] ? 1 : 0;
      error += qtrArray.weights[i] * val;
      activeSensors += val;
    }

    error = error / activeSensors;

    integral += error;
    if (integral > 100) integral = 100;
    if (integral < -100) integral = -100;

    long derivative = error - prevError;
    int correction = Kp * error + Kd * derivative + Ki * integral;

    prevError = error;

    int leftPWM = basePWM + 12 - correction;
    int rightPWM = basePWM + correction;

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    delay(LOOP_DELAY);
  }
}


void lineFollow_allWhite(int targetPWM) {
  const int basePWM = targetPWM;

  const float Kp = 3;
  const float Kd = 0.0;
  const float Ki = 0.01;

  long prevError = 0;
  long integral = 0;
  resetEncoders();

  while (true) {

    noInterrupts();
    long leftEnc = leftMotor.enc;
    long rightEnc = rightMotor.enc;
    interrupts();

    int avgEnc = (float)(leftEnc + rightEnc) / 2.0;

    if (allWhite() && avgEnc > distToCounts(200)) {
      break;
    }

    readIRArray();

    long error = 0;
    int activeSensors = 0;

    for (int i = 0; i < NUM_IR; i++) {
      int val = qtrArray.detected[i] ? 1 : 0;
      error += qtrArray.weights[i] * val;
      activeSensors += val;
    }

    error = error / activeSensors;

    integral += error;
    if (integral > 100) integral = 100;
    if (integral < -100) integral = -100;

    long derivative = error - prevError;
    int correction = Kp * error + Kd * derivative + Ki * integral;

    prevError = error;

    int leftPWM = basePWM + 12 - correction;
    int rightPWM = basePWM + correction;

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    delay(LOOP_DELAY);
  }
}

void lineFollow_tofDist(int targetPWM, int dist_mm) {
  const int basePWM = targetPWM;

  const float Kp = 3;
  const float Kd = 0.0;
  const float Ki = 0.01;

  long prevError = 0;
  long integral = 0;

  while (true) {
    readIRArray();

    long error = 0;
    int activeSensors = 0;

    for (int i = 0; i < NUM_IR; i++) {
      int val = qtrArray.detected[i] ? 1 : 0;
      error += qtrArray.weights[i] * val;
      activeSensors += val;
    }

    error = error / activeSensors;

    integral += error;
    if (integral > 100) integral = 100;
    if (integral < -100) integral = -100;

    long derivative = error - prevError;
    int correction = Kp * error + Kd * derivative + Ki * integral;

    prevError = error;

    int leftPWM = basePWM + 12 - correction;
    int rightPWM = basePWM + correction;

    int leftTOF = readTOF(L3_FLEFT_TOF);
    int rightTOF = readTOF(L3_FRIGHT_TOF);
    int distFront = (float)(leftTOF + rightTOF) / 2.0;

    if (distFront > 0 && distFront <= dist_mm) {
      setMotorPWM(leftMotor, 0);
      setMotorPWM(rightMotor, 0);
      break;
    }

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    delay(LOOP_DELAY);
  }
}

void forwardAligning_tofDist(int targetPWM, int dist_mm) {
  const float Kp = 0.3;  
  const float Kd = 0.0;

  int prevError = 0;

  resetEncoders();

  while (true) {

    noInterrupts();
    int leftEnc = leftMotor.enc;
    int rightEnc = rightMotor.enc;
    interrupts();

    int avgEnc = (float)(leftEnc + rightEnc) / 2.0;


    int fright_tof = readTOF(L3_FRIGHT_TOF);
    int fleft_tof = readTOF(L3_FLEFT_TOF);

    if (fright_tof <= 0 || fleft_tof <= 0) continue;

    int distFront = (fright_tof + fleft_tof) / 2;

    int error = fleft_tof - fright_tof - 22;
    int dError = error - prevError;

    int correction = Kp * error + Kd * dError;

    prevError = error;

    int leftPWM = targetPWM - correction;
    int rightPWM = targetPWM + correction;

    int dist_IR = analogRead(FRONT_IR);

    if (distFront < dist_mm && abs(error) < 10) {
      setMotorPWM(leftMotor, 0);
      setMotorPWM(rightMotor, 0);
      distMoved = countsToDist(avgEnc);
      break;
    }

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    delay(LOOP_DELAY);
  }
}

void forwardAligning_dist(int targetPWM, int dist_mm) {
  const float Kp = 0.3;
  const float Kd = 0.0;

  int prevError = 0;

  resetEncoders();

  while (true) {

    noInterrupts();
    int leftEnc = leftMotor.enc;
    int rightEnc = rightMotor.enc;
    interrupts();

    int avgEnc = (float)(leftEnc + rightEnc) / 2.0;


    int fright_tof = readTOF(L3_FRIGHT_TOF);
    int fleft_tof = readTOF(L3_FLEFT_TOF);


    if (fright_tof <= 0 || fleft_tof <= 0) continue;

    int distFront = (fright_tof + fleft_tof) / 2;

    int error = fleft_tof - fright_tof - 24;
    int dError = error - prevError;

    int correction = Kp * error + Kd * dError;

    prevError = error;

    int leftPWM = targetPWM - correction;
    int rightPWM = targetPWM + correction;

    if (avgEnc >= distToCounts(dist_mm)) {
      setMotorPWM(leftMotor, 0);
      setMotorPWM(rightMotor, 0);
      distMoved = countsToDist(avgEnc);
      break;
    }

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    delay(LOOP_DELAY);
  }
}

/*
void wallFollowRight_allWhite(int targetPWM) {
  const float Kp_wall = 0.6;
  const float Kd_wall = 0.3;
  const float Ki_wall = 0.01;

  int prevWallError = 0;
  int wallIntegral = 0;

  int basePWM = targetPWM;

  while (!allWhite()) {

    int rightTOF = readTOF(L3_RIGHT_TOF);
    Serial.println(rightTOF);

    int wallError = 0;
    if (rightTOF > 0 && rightTOF < 600) {
      const int desiredDistance = 400;
      wallError = desiredDistance - rightTOF;
    }

    wallIntegral += wallError;
    wallIntegral = constrain(wallIntegral, -200, 200);

    int wallDerivative = wallError - prevWallError;

    int correction = Kp_wall * wallError + Kd_wall * wallDerivative + Ki_wall * wallIntegral;

    prevWallError = wallError;

    int leftPWM = basePWM - correction;
    int rightPWM = basePWM + correction;

    if (leftPWM > 0) leftPWM = constrain(leftPWM, -80, -50);
    else leftPWM = constrain(leftPWM, 50, 80);

    if (rightPWM < 0) rightPWM = constrain(leftPWM, -80, -50);
    else rightPWM = constrain(leftPWM, 50, 80);
    if (leftPWM < 0 && rightPWM < 0) {
      leftPWM = -leftPWM;
      rightPWM = -rightPWM;
    }

    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    delay(LOOP_DELAY);
  }
}


void backward_aligningLimitSW(int targetPWM, int limitSW_pin) {
  const int basePWM = targetPWM;

  float Kp = 0.8;
  float Ki = 0.02;
  float Kd = 0.5;

  float error = 0;
  float prevError = 0;
  float integral = 0;

  const float integralLimit = 100;

  while (digitalRead(limitSW_pin) == HIGH) {

    int leftTOF = readTOF(L3_FLEFT_TOF);
    int rightTOF = readTOF(L3_FRIGHT_TOF);

    error = leftTOF - rightTOF;

    if (abs(error) < 5) error = 0;

    integral += error;
    integral = constrain(integral, -integralLimit, integralLimit);

    float derivative = error - prevError;

    float correction = Kp * error + Ki * integral + Kd * derivative;

    prevError = error;

    int leftPWM = basePWM + correction;
    int rightPWM = basePWM - correction;

    leftPWM = constrain(leftPWM, 20, 80);
    rightPWM = constrain(rightPWM, 20, 80);

    setMotorPWM(leftMotor, -leftPWM);
    setMotorPWM(rightMotor, -rightPWM);
  }

  setMotorPWM(leftMotor, 0);
  setMotorPWM(rightMotor, 0);
}*/