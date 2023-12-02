#include "FastAccelStepper.h"
#include "stepper.h"

void Stepper::Init(int dirPin, int enablePin, int stepPin, uint16_t rapidSpeed){
    myEngine.init();
    myStepper = myEngine.stepperConnectToPin(stepPin);
    myRapidSpeed = rapidSpeed;
    if (myStepper) {
        myStepper->setDirectionPin(dirPin);
        myStepper->setEnablePin(enablePin);
        myStepper->setAutoEnable(true);
        myStepper->setDelayToDisable(200);
        myStepper->setAcceleration(1000);
    }
}

void Stepper::UpdateActiveSpeed() {
  int16_t aSpeed = myUseRapidSpeed ? myRapidSpeed : myNormalSpeed;

  if(myStepper->isRunning() || myStepper->isRunningContinuously()) {
    if(myStepper->getSpeedInMilliHz() <= aSpeed + (aSpeed*0.05) && myStepper->getSpeedInMilliHz() >= aSpeed - (aSpeed*0.05)) {
        //within 0.5% of the target speed, good enough.
        //todo do this at the event phase
        return;
    }
    else {
        myStepper->setSpeedInMilliHz(aSpeed);
    }
  }
}

void Stepper::UpdateNormalSpeed(int16_t aSpeed) {
    myNormalSpeed = aSpeed;
    UpdateActiveSpeed();
}

void Stepper::UpdateRapidSpeed(int16_t aSpeed) {
    myRapidSpeed = aSpeed;
    UpdateActiveSpeed();
}



void Stepper::MoveLeft() {
    myStepper->runForward();
}

void Stepper::MoveRight() {
    myStepper->runBackward();
}

void Stepper::Stop() {
    myStepper->stopMove();
}

void Stepper::SetRapidSpeed() {
    myUseRapidSpeed = true;
    UpdateActiveSpeed();
}

void Stepper::SetNormalSpeed() {
    myUseRapidSpeed = false;
    UpdateActiveSpeed();
}

uint16_t Stepper::GetNormalSpeed() {
    return myNormalSpeed;
}