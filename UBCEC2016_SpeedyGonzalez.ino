#include <Servo.h>

#define LEFT_TAPE_SENSOR 0
#define RIGHT_TAPE_SENSOR 0
#define MIDDLE_TAPE_SENSOR 0

#define LEFT_MOTOR_OUT 9
#define RIGHT_MOTOR_OUT 10
#define FULLY_ON_THRESH 600
#define HALF_ON_THRESH 300

Servo motorLeft;
Servo motorRight;

int speedLeft;
int speedRight;

int leftTapeVal, rightTapeVal, middleTapeVal;

int prevError, pastError, recError, error = 0;
int q,m = 0;
int p,d = 0;
int correction = 0;

// Gains
int kp = 5;
int kd = 5;

void setup() {
  motorLeft.attach(LEFT_MOTOR_OUT);
  motorRight.attach(RIGHT_MOTOR_OUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  leftTapeVal = digitalRead(LEFT_TAPE_SENSOR);
  rightTapeVal = digitalRead(RIGHT_TAPE_SENSOR);
  middleTapeVal = digitalRead(MIDDLE_TAPE_SENSOR);

  tapeFollow(); 
}

void tapeFollow() {
  // Determine the Error Value Error is negative if to the left of the tape, positive if to the right
  // Two QRD Logic
  if (leftTapeVal == LOW && rightTapeVal == LOW) {
    if (pastError < 0) {
      error = -5;
    } else if (pastError > 0) {
      error = 5;
    } else if (pastError == 0) {
      // Should anything be done here??
    }
  } else if (rightTapeVal == HIGH) {
    error = -1;
  } else if (leftTapeVal == HIGH) {
    error = 1;
  } else {// Impossible to reach
    error = 0;
  }

  // Three QRD Logic
  if (leftTapeVal == LOW && rightTapeVal == LOW) {
    if(middleTapeVal == HIGH){
      error = 0;
    } else if (pastError < 0) { // All off - determine error based on pastError
      error = -5;
    } else if (pastError > 0) {
      error = 5;
    } else if (pastError == 0) {
      // Should anything be done here??
    }
  } else{
    if(middleTapeVal == HIGH){ // Outside and middle sensor on tape
      if (rightTapeVal == HIGH) {
        error = -1;
      } else if (leftTapeVal == HIGH) {
        error = 1;
      } else {// Impossible to reach
        error = 0;
      }
    }else{ // Just one outside sensor on tape
      if (rightTapeVal == HIGH) {
        error = -3;
      } else if (leftTapeVal == HIGH) {
        error = 3;
      } else {// Impossible to reach
        error = 0;
      }
    }
  }

  //P-D Calculations
  if (!error == pastError) {
    recError = prevError;
    q = m;
    m = 1;
  }

  p = kp * error;
  d = (int)((float)kd * (float)(error - recError) / (float)(q + m));
  correction = p + d;

  pastError = error;
  m++;

  // what value?
  int tapeFollowVel = 0;

  // set speed
  // need to scale so that it varies from slow to fast
  // from ~120 - 180 for CW
  // from ~75 - 0 for CCW
  speedLeft = tapeFollowVel - correction;
  speedRight = tapeFollowVel + correction;

  motorLeft.write(speedLeft);
  motorRight.write(speedRight);
}
