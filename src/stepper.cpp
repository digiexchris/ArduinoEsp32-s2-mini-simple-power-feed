#include "FastAccelStepper.h"
#include "stepper.h"

Stepper::Stepper(int dirPin, int enablePin, int stepPin) {
    myEngine.init();
    myStepper = std::make_shared<FastAccelStepper>(myEngine.stepperConnectToPin(stepPin));
    
    if (myStepper) {
        myStepper->setDirectionPin(dirPin);
        myStepper->setEnablePin(enablePin);
        myStepper->setAutoEnable(true);
        myStepper->setDelayToDisable(200);
        myStepper->setAcceleration(1000);
    }
}

void Stepper::UpdateSpeed(int32_t aSpeed) {
if(myStepper->getSpeedInMilliHz() <= aSpeed + (aSpeed*0.05) && myStepper->getSpeedInMilliHz() >= aSpeed - (aSpeed*0.05)) {
    //within 0.5% of the target speed, good enough.
    return;
  }
  else {
    myStepper->setSpeedInMilliHz(aSpeed);
  }
}