#include "config.h"
#ifdef USE_FASTACCELSTEPPER
#include "FastAccelStepper.h"
#elif USE_DENDO_STEPPER
#include "DendoStepper.h"
#else
#error "No stepper library defined"
#endif

#include "stepper.h"
#include <mutex>
#include <algorithm>

Stepper::Stepper() {
    myUseRapidSpeed = false;
    // myUseRapidSpeedLock = new std::lock_guard<std::mutex>(myUseRapidSpeedMutex);
    myRapidSpeed = 0;
    myNormalSpeed = 0;
    // myRapidSpeedLock = new std::lock_guard<std::mutex>(myRapidSpeedMutex);
    // myNormalSpeedLock = new std::lock_guard<std::mutex>(myNormalSpeedMutex);
}

void Stepper::Init(uint8_t dirPin, uint8_t enablePin, uint8_t stepPin, uint16_t rapidSpeed){
    #ifdef USE_DENDO_STEPPER

    myStepperCfg = {
        .stepPin = stepPin,
        .dirPin = dirPin,
        .enPin = enablePin,
        .timer_group = TIMER_GROUP_0,
        .timer_idx = TIMER_0,
        .miStep = MICROSTEP_1,
        .stepAngle = 1.8};

    myStepper.config(&myStepperCfg);
    myStepper.init();
    //TODO I don't know if it will accept a value of zero
    myStepper.setSpeed(0, FULL_SPEED_ACCELERATION_LINEAR_TIME, FULL_SPEED_DECELERATION_LINEAR_TIME);
    #elif USE_FASTACCELSTEPPER
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
    #endif

    ESP_LOGI("Stepper", "Stepper init complete");
}

bool Stepper::IsStopped() {
    std::lock_guard<std::mutex> stepperLock(myStepperMutex);
    #ifdef USE_DENDO_STEPPER
    return(myStepper.getState() == IDLE);
    #elif USE_FASTACCELSTEPPER
    return (!myStepper->isRunning() && !myStepper->isStopping());
    #endif
    
}

void Stepper::UpdateActiveSpeed() {
    //NOTE do not put a lock_guard here, it will cause a deadlock. Many things call this function.
    const uint16_t aSpeed = myUseRapidSpeed ? myRapidSpeed : myNormalSpeed;

    #ifdef USE_DENDO_STEPPER
    const uint16_t curSpeed = myStepper.getTargetSpeed();
    const uint16_t lowBound = curSpeed - (MAX_DRIVER_STEPS_PER_SECOND * 0.05);
    const uint16_t highBound = curSpeed + (MAX_DRIVER_STEPS_PER_SECOND * 0.05);
        
    if(std::clamp(aSpeed, lowBound, highBound) != aSpeed) {
        if(curSpeed > aSpeed) {
            //decelerating
            const uint16_t accTime = myStepper.getAcc();
            const uint16_t decTime = FULL_SPEED_DECELERATION_LINEAR_TIME*(aSpeed/MAX_DRIVER_STEPS_PER_SECOND);
            //#error major error, this needs to be in ms, currently passed in seconds I think
            myStepper.setSpeed(aSpeed, accTime, decTime);
        } else {
            //accelerating
            const uint16_t accTime = FULL_SPEED_ACCELERATION_LINEAR_TIME*(aSpeed/MAX_DRIVER_STEPS_PER_SECOND);
            const uint16_t decTime = myStepper.getDec();
            myStepper.setSpeed(aSpeed, accTime, decTime);
        }
 #elif USE_FASTACCELSTEPPER   
    const uint16_t curSpeed = myStepper->getSpeedInMilliHz()/1000;
    const bool isRunning = myStepper->isRunning();
    if(aSpeed != 0 && (curSpeed <= aSpeed - (aSpeed*0.05) || curSpeed >= aSpeed + (aSpeed*0.05))) {
        ESP_LOGI("Stepper", "Speed is not within 0.5%% of target speed, updating");
        ESP_ERROR_CHECK(myStepper->setSpeedInHz(aSpeed));
        myStepper->applySpeedAcceleration();
        #endif
    } else {
        ESP_LOGI("Stepper", "Speed is close enough to current speed, ignore");
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
    #ifdef USE_DENDO_STEPPER
    UpdateActiveSpeed();
    myStepper.runInf(static_cast<bool>(StepperDirection::Left));
    #elif USE_FASTACCELSTEPPER
    UpdateActiveSpeed();
    myStepper->runForward();
    #endif
    UpdateActiveSpeed();
    ESP_LOGI("Stepper", "Moving left");
}

void Stepper::MoveRight() {
    std::lock_guard<std::mutex> stepperLock(myStepperMutex);
    #ifdef USE_DENDO_STEPPER
    UpdateActiveSpeed();
    myStepper.runInf(static_cast<bool>(StepperDirection::Right));
    #elif USE_FASTACCELSTEPPER
    UpdateActiveSpeed();
    myStepper->runForward();
    #endif
    UpdateActiveSpeed();
    ESP_LOGI("Stepper", "Moving right");
}

void Stepper::Stop() {
    std::lock_guard<std::mutex> stepperLock(myStepperMutex);
    #ifdef USE_DENDO_STEPPER
    myNormalSpeed = 0;
    myRapidSpeed = 0;
    UpdateActiveSpeed();
    #elif USE_FASTACCELSTEPPER
    myStepper->stopMove();
    #endif
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