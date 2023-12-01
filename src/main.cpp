#include <Arduino.h>

#include "FastAccelStepper.h"

#define dirPinStepper 18
#define enablePinStepper 21
#define stepPinStepper 16
#define speedPin 17 //front knob pot
#define maxSpeedPin 33 //trimpot to select max speed
#define leftPin 34
#define rightPin 35
#define rapidPin 36
#define stopLeftPin 37
#define stopRightPin 38

//TODO add stop positions to oled display

const int maxDriverFreq = 20000000; // 20kHz max pulse freq in millihz at 25/70 duty cycle, 13kHz at 50/50
const int maxRpm = 160; // 160 rpm max as per Align power feed
const int stepsPerRev = 200;

volatile bool isLeft = false;
volatile bool isRight = false;
volatile bool isRapid = false;

int speed = 0;
int rapidSpeed = 0;

int32_t leftStop = INT32_MIN;
int32_t rightStop = INT32_MAX;
bool isLeftStopSet = false;
bool isRightStopSet = false;

bool requiresStop = false;

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

enum State {
  STOPPED,
  RUNNING,
  STOPPING
};

State state = STOPPED;

void leftInterrupt() {
  bool leftState = digitalRead(leftPin);
  if(!isLeft && leftState && !stepper->isRunning()) {
    //from stop to run left
    isLeft = true;
    requiresStop = false;
  } else if(isLeft && !leftState && stepper->isRunning()) {
    //from left to middle
    isLeft = false;
    requiresStop = true;
  }
}

void rightInterrupt() {
  bool rightState = digitalRead(rightPin);
  if(!isRight && rightState && !stepper->isRunning()) {
    //from stop to run right
    isRight = true;
    requiresStop = false;
  } else if(isRight && !rightState && stepper->isRunning()) {
    //from right to middle
    isRight = false;
    requiresStop = true;
  }
}

void rapidInterrupt() {
  isRapid = digitalRead(rapidPin) == HIGH;
}

void leftStopInterrupt() {
  if(stepper->isRunning()) {
    //ignore, we are moving
    //TODO flash warning on oled
    return;
  }
  if(isLeftStopSet) {
    leftStop = stepper->getCurrentPosition();
    isLeftStopSet = true;
  }
  else {
    leftStop = INT32_MIN;
    isLeftStopSet = false;
  }
}

void rightStopInterrupt() {
  if(stepper->isRunning()) {
    //ignore, we are moving
    //TODO flash warning on oled
    return;
  }

  if(isRightStopSet) {
    rightStop = stepper->getCurrentPosition();
    isRightStopSet = true;
  }
  else {
    rightStop = INT32_MAX;
    isRightStopSet = false;
  }
}

void updateSpeed(int32_t aSpeed){
  if(stepper->getSpeedInMilliHz() <= aSpeed + (aSpeed*0.05) && stepper->getSpeedInMilliHz() >= aSpeed - (aSpeed*0.05)) {
    //within 0.5% of the target speed, good enough.
    return;
  }
  else {
    stepper->setSpeedInMilliHz(aSpeed);
  }
}

void setup() {
  engine.init();
  stepper = engine.stepperConnectToPin(stepPinStepper);
  if (stepper) {
    stepper->setDirectionPin(dirPinStepper);
    stepper->setEnablePin(enablePinStepper);
    stepper->setAutoEnable(true);
    stepper->setDelayToDisable(200);
    stepper->setAcceleration(1000);
  }
  pinMode(leftPin, INPUT_PULLUP);
  pinMode(rightPin, INPUT_PULLUP);
  pinMode(rapidPin, INPUT_PULLUP);
  pinMode(stopLeftPin, INPUT_PULLUP);
  pinMode(stopRightPin, INPUT_PULLUP);\

  attachInterrupt(digitalPinToInterrupt(stopLeftPin), leftStopInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(stopRightPin), rightStopInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(leftPin), leftInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rightPin), rightInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rapidPin), rapidInterrupt, CHANGE);
}

void loop() {
  speed = map(analogRead(speedPin), 0, 1023, 0, maxDriverFreq);
  rapidSpeed = map(analogRead(maxSpeedPin), 0, 1023, 0, maxDriverFreq);

  if (stepper) {

  if(speed == 0 && stepper->isRunning()) {
    stepper->stopMove();
    state = STOPPING;
    return;
  }

  if(requiresStop && stepper->isRunning()) {
    stepper->stopMove();
    state = STOPPING;
    return;
  }

  //let it come to a complete stop before changing direction
  if(stepper->isStopping()) {
    return;
  }
  
  if(!stepper->isRunning() && !stepper->isStopping()) {
    if(isLeft) {
      stepper->moveTo(leftStop);
    } else if(isRight) {
      stepper->moveTo(rightStop);
    }
  }

  if(stepper->isRunning() && isRapid) {
    updateSpeed(rapidSpeed);
  } else if (stepper->isRunning() && !isRapid) {
    updateSpeed(speed);
  }
}