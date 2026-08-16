int lastDetectedRightEnc = -1;
int lastDetectedLeftEnc = -1;

int onLineLeftEnc = 0;
int onLineRightEnc = 0;

int distMoved = -1;
bool skipBoxes = false;

int bufferRight[WINDOW_SIZE] = { 0 };
int indexRight = 0;
long sumRight = 0;
int countRight = 0;

int bufferLeft[WINDOW_SIZE] = { 0 };
int indexLeft = 0;
long sumLeft = 0;
int countLeft = 0;

float getAverageRight(int newValue) {
  sumRight -= bufferRight[indexRight];
  bufferRight[indexRight] = newValue;
  sumRight += newValue;

  indexRight = (indexRight + 1) % WINDOW_SIZE;

  if (countRight < WINDOW_SIZE) countRight++;

  return (float)sumRight / countRight;
}

float getAverageLeft(int newValue) {
  sumLeft -= bufferLeft[indexLeft];
  bufferLeft[indexLeft] = newValue;
  sumLeft += newValue;

  indexLeft = (indexLeft + 1) % WINDOW_SIZE;

  if (countLeft < WINDOW_SIZE) countLeft++;

  return (float)sumLeft / countLeft;
}

int moveAlongLine(int targetPWM) {
  const int basePWM = targetPWM;

  const float Kp = 3;
  const float Kd = 0.0;
  const float Ki = 0.01;

  long prevError = 0;
  long integral = 0;

  int rightCount = 0;
  int leftCount = 0;

  const float Kp_wall = 0.2;
  const float Kd_wall = 0.02;
  const float Ki_wall = 0.01;

  int prevWallError = 0;
  int wallIntegral = 0;

  bool firstEdgeDetected = false;
  bool secondEdgeDetected = false;

  int slowPWM = 30;

  int currentPWM = basePWM;
  int firstEdgeDetectedDir = -1;

  while (true) {
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

    int leftPWM = currentPWM + 12 - correction;
    int rightPWM = currentPWM + correction;


    setMotorPWM(leftMotor, leftPWM);
    setMotorPWM(rightMotor, rightPWM);

    int leftTOF = readTOF(L3_LEFT_TOF);
    int rightTOF = readTOF(L3_RIGHT_TOF);

    int rightIR = analogRead(RIGHT_IR);
    int leftIR = analogRead(LEFT_IR);

    float rightAvg = getAverageRight(rightIR);
    float leftAvg = getAverageLeft(leftIR);


    int fir_dist = analogRead(FRONT_IR);
    if (fir_dist > FRONT_SHARP_BOX_MEAS) {
      skipBoxes = true;
      break;
    }

    noInterrupts();
    int leftEnc = leftMotor.enc;
    int rightEnc = rightMotor.enc;
    interrupts();

    int avgEnc = (float)(leftEnc + rightEnc) / 2.0;

    if (!firstEdgeDetected) {

      if (rightAvg >= RIGHT_SHARP_BOX_MEAS && (lastDetectedRightEnc == -1 || (rightEnc + lastDetectedRightEnc >= distToCounts(BOX_DETECTION_DEADBAND)))) {
        firstEdgeDetected = true;
        firstEdgeDetectedDir = BOX_ON_RIGHT;
        currentPWM = slowPWM;
      }

      if (leftAvg >= LEFT_SHARP_BOX_MEAS && (lastDetectedLeftEnc == -1 || (leftEnc + lastDetectedLeftEnc >= distToCounts(BOX_DETECTION_DEADBAND)))) {
        firstEdgeDetected = true;
        firstEdgeDetectedDir = BOX_ON_LEFT;
        currentPWM = slowPWM;
      }
    }

    if (firstEdgeDetected) {
      if (rightAvg < RIGHT_SHARP_BOX_MEAS && firstEdgeDetectedDir == BOX_ON_RIGHT) {
        firstEdgeDetected = false;
        return BOX_ON_RIGHT;
      }

      if (leftAvg < LEFT_SHARP_BOX_MEAS && firstEdgeDetectedDir == BOX_ON_LEFT) {
        firstEdgeDetected = false;
        return BOX_ON_LEFT;
      }
    }
  }
  delay(LOOP_DELAY);
}

void collectBoxes(int targetPWM, int boxCount) {
  resetEncoders();
  for (int i = 0; i < boxCount; i++) {
    if (!skipBoxes) {
      collectBox(targetPWM);

    } else break;
  }


  lineFollow_tofDist(targetPWM, 100);
  delay(1000);
  turnLeft(TURN_LEFT_PWM);
}

void collectBox(int targetPWM) {
  int box_condition = moveAlongLine(targetPWM);

  if (!skipBoxes) {

    if (box_condition == BOX_ON_RIGHT) {
      forwardEncoders_dist(SLOW_BASE_PWM, 20);
    } else {
      forwardEncoders_dist(SLOW_BASE_PWM, 25);
    }

    noInterrupts();
    onLineLeftEnc += leftMotor.enc;
    onLineRightEnc += rightMotor.enc;
    interrupts();

    delay(1000);

    if (box_condition == BOX_ON_RIGHT) {
      turnRight(TURN_RIGHT_PWM);
    } else {
      turnLeft(TURN_LEFT_PWM);
    }
    delay(1000);

    sendCommand('D');

    forwardAligning_dist(targetPWM, 310);

    delay(1000);

    sendCommand('B');
    waitForDone();

    turnBack(TURN_BACK_PWM);
    delay(1000);

    forwardEncoders_dist(targetPWM, distMoved);
    delay(1000);

    if (box_condition == BOX_ON_RIGHT) {
      turnRight(TURN_RIGHT_PWM);
    } else {
      turnLeft(TURN_LEFT_PWM);
    }
    delay(1000);
    sendCommand('2');
  }
}

void eliminationTask() {
  // Move from White Box to Line Facing Along the Line
  forwardEncoders_dist(BASE_PWM, 500);
  forwardEncoders_allWhite(BASE_PWM);
  forwardEncoders_dist(BASE_PWM, 70);
  turnLeft(TURN_LEFT_PWM);

  // Collect All boxes and face right
  collectBoxes(BASE_PWM, 6);

  // Move until
  forwardEncoders_allWhite(BASE_PWM);

  lineFollow_leftWhite(BASE_PWM, J2_LINE_DETECTION_DEADBAND);
  forwardEncoders_dist(BASE_PWM, J_LINE_TURN_DIST);

  delay(1000);
  turnLeft(TURN_LEFT_PWM);

  lineFollow_allWhite(BASE_PWM);
  delay(1000);

  // turnBack(BASE_PWM + 10);
  turnBackFake(BASE_PWM + 10);
  delay(1000);

  backwardEncoders_limitSW(BASE_PWM);
  sendCommand('H');
  delay(5000);
  sendCommand('4');
}