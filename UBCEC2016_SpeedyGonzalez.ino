
#include <Servo.h>

#define LEFT_TAPE_SENSOR 0
#define RIGHT_TAPE_SENSOR 1
#define RIGHT_INT_SENSOR -1
#define LEFT_INT_SENSOR -1

#define LEFT_MOTOR_OUT 9
#define RIGHT_MOTOR_OUT 10
#define FULLY_ON_THRESH 600
#define HALF_ON_THRESH 300


int leftTapeVal, rightTapeVal, leftIntVal, rightIntVal;
int tapeThresh = 600;

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

// Intersection stuff
int detectIntersectionCount = 0;
int detectIntersectionThresh = 20;
int intersectionDetected = 0;
int nextIntersection = 0;
int loopsSinceLastInt = 0;

int loopCount = 0;
int serialFreq = 10;

void setup() {
  // put your setup code here, to run once:
  // Do we need to set pin modes?? ie. input/output/pwm
  Serial.begin(9600);
  motorLeft.attach(LEFT_MOTOR_OUT);
  motorRight.attach(RIGHT_MOTOR_OUT);

}

void loop() {
  loopCount++;
  loopsSinceLastInt++;
  // put your main code here, to run repeatedly:
  leftTapeVal = analogRead(LEFT_TAPE_SENSOR) > tapeThresh;
  rightTapeVal = analogRead(RIGHT_TAPE_SENSOR) > tapeThresh;
  leftIntVal = analogRead(LEFT_INT_SENSOR) > tapeThresh;
  rightIntVal = analogRead(RIGHT_INT_SENSOR) > tapeThresh;


  tapeFollow();
  if(loopsSinceLastInt > 1000){
    detectIntersection();
  }
  
  if(loopCount%serialFreq == 0){
    Serial.print(leftTapeVal);
    Serial.print(" ");
    Serial.print(rightTapeVal);
    Serial.print(":  ");
    Serial.println(correction);
  }
}

void detectIntersection(){
  if(leftIntVal == HIGH || rightIntVal == HIGH){
    if(detectIntersectionThresh <= detectIntersectionThresh){
      detectIntersectionCount++;
    }
  }else if (detectIntersectionCount > 0){
    detectIntersectionCount--;
  }
  if(!intersectionDetected && detectIntersectionCount > detectIntersectionThresh){
    intersectionDetected = 1;
  }
  if(intersectionDetected && detectIntersectionCount < detectIntersectionThresh/4){
    intersectionDetected = 0;
    loopsSinceLastInt = 0;
    nextIntersection++;
  }
}

void tapeFollow() {
  // Determine the Error Value, Error is negative if to the left of the tape, positive if to the right
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

  /*// Look at outside QRDs and adjust the error
  if ((leftIntVal == HIGH || rightIntVal == HIGH) && nextIntersection == 0 || nextIntersection == 3){
    error = 0;
  }else if(leftIntVal == HIGH){
    error = 8;
  }else if(rightIntVal == HIGH){
    error = -8;
  }*/
  



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
