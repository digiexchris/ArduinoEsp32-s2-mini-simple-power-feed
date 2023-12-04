#include "FastAccelStepper.h"
#include "stepper.h"
#include <mutex>

Stepper::Stepper() {
    myUseRapidSpeed = false;
    // myUseRapidSpeedLock = new std::lock_guard<std::mutex>(myUseRapidSpeedMutex);
    myRapidSpeed = 0;
    myNormalSpeed = 0;
    // myRapidSpeedLock = new std::lock_guard<std::mutex>(myRapidSpeedMutex);
    // myNormalSpeedLock = new std::lock_guard<std::mutex>(myNormalSpeedMutex);
}

void Stepper::Init(int dirPin, int enablePin, int stepPin, uint16_t rapidSpeed){
    myEngine.init();
    myStepper = myEngine.stepperConnectToPin(stepPin);
    myRapidSpeed = rapidSpeed;
    if (myStepper) {
        myStepper->setDirectionPin(dirPin);
        myStepper->setEnablePin(enablePin);
        myStepper->setAutoEnable(true);
        myStepper->setDelayToDisable(200);
        myStepper->setDelayToEnable(50); // this is the time the enable pin must be active before the stepper starts moving
        myStepper->setAcceleration(20000);
        //myStepper->enableOutputs();
        
    }

    ESP_LOGI("Stepper", "Stepper init complete");
}

bool Stepper::IsStopped() {
    std::lock_guard<std::mutex> stepperLock(myStepperMutex);
    return (!myStepper->isRunning() && !myStepper->isStopping());
}

void Stepper::UpdateActiveSpeed() {
    //NOTE do not put a lock_guard here, it will cause a deadlock. Many things call this function.
    const uint16_t aSpeed = myUseRapidSpeed ? myRapidSpeed : myNormalSpeed;
    const uint16_t curSpeed = myStepper->getSpeedInMilliHz()/1000;
    const bool isRunning = myStepper->isRunning();

    if(isRunning && aSpeed != 0 && (curSpeed <= aSpeed - (aSpeed*0.05) || curSpeed >= aSpeed + (aSpeed*0.05))) {
        ESP_LOGI("Stepper", "Speed is not within 0.5%% of target speed, updating");
        ESP_ERROR_CHECK(myStepper->setSpeedInHz(aSpeed));
        myStepper->applySpeedAcceleration();
    } else {
        ESP_LOGI("Stepper", "Speed is 0 or stepper not running, not updating");
    }
}

void Stepper::UpdateNormalSpeed(int16_t aSpeed) {
    std::lock_guard<std::mutex> stepperLock(myStepperMutex);
    myNormalSpeed = aSpeed;
    UpdateActiveSpeed();
}

void Stepper::UpdateRapidSpeed(int16_t aSpeed) {
    std::lock_guard<std::mutex> stepperLock(myStepperMutex);
    myRapidSpeed = aSpeed;
    UpdateActiveSpeed();
}

void Stepper::MoveLeft() {
    std::lock_guard<std::mutex> stepperLock(myStepperMutex);
    myStepper->runForward();
    UpdateActiveSpeed();
    ESP_LOGI("Stepper", "Moving left");
}

void Stepper::MoveRight() {
    std::lock_guard<std::mutex> stepperLock(myStepperMutex);
    myStepper->runBackward();
    UpdateActiveSpeed();
    ESP_LOGI("Stepper", "Moving right");
}

void Stepper::Stop() {
    std::lock_guard<std::mutex> stepperLock(myStepperMutex);
    myStepper->stopMove();
    ESP_LOGI("Stepper", "Stopping");
}

void Stepper::SetRapidSpeed() {
    std::lock_guard<std::mutex> stepperLock(myStepperMutex);
    myUseRapidSpeed = true;
    UpdateActiveSpeed();
    ESP_LOGI("Stepper", "Rapid speed");
}

void Stepper::SetNormalSpeed() {
    std::lock_guard<std::mutex> stepperLock(myStepperMutex);
    myUseRapidSpeed = false;
    UpdateActiveSpeed();
    ESP_LOGI("Stepper", "Normal speed");
}