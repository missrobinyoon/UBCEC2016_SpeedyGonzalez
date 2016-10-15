
#include <Servo.h>

#define LEFT_TAPE_SENSOR 5
#define RIGHT_TAPE_SENSOR 0
#define RIGHT_INT_SENSOR 1
#define LEFT_INT_SENSOR 4

#define LEFT_MOTOR_OUT 10
#define RIGHT_MOTOR_OUT 9

int leftTapeVal, rightTapeVal, leftIntVal, rightIntVal;
int tapeThresh = 600;

Servo motorLeft;
Servo motorRight;

// Right: Clockwise, Left: CCW
int baseSpeed = 30;
int leftBaseSpeed = 90 - baseSpeed;
int rightBaseSpeed = 90 + baseSpeed;
int speedLeft, speedRight;

int prevError, pastError, recError, error = 0;
int q,m = 0;
int p,d = 0;
int correction = 0;

// Gains
int kp = 15;
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

  leftTapeVal = analogRead(LEFT_TAPE_SENSOR) > tapeThresh;
  rightTapeVal = analogRead(RIGHT_TAPE_SENSOR) > tapeThresh;
  leftIntVal = analogRead(LEFT_INT_SENSOR) > tapeThresh;
  rightIntVal = analogRead(RIGHT_INT_SENSOR) > tapeThresh;


  tapeFollow();
  if(loopsSinceLastInt > 1000){
    //detectIntersection();
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
    if(detectIntersectionCount <= detectIntersectionThresh){
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

  //pastError = error;
  // Look at outside QRDs and adjust the error
  /*if ((leftIntVal == HIGH || rightIntVal == HIGH) && (nextIntersection == 0 || nextIntersection == 3 || nextIntersection > 5)){
    error = 0;
    /*if (leftIntVal == HIGH){
      pastError = 1;
    }else{
      pastError = -1
    }
    pastError = error;
  }else if(leftIntVal == HIGH){
    error = 8;
    pastError = error;
  }else if(rightIntVal == HIGH){
    error = -8;
    pastError = error;
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

  //correction = 0; // GET RID OF THIS LATER

  // set speed
  // need to scale so that it varies from slow to fast
  // from ~120 - 180 for CW
  // from ~75 - 0 for CCW
  speedLeft = leftBaseSpeed + correction;
  speedRight = rightBaseSpeed + correction;

  motorLeft.write(speedLeft);
  motorRight.write(speedRight);
}
