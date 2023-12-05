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
#include <esp_log.h>

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
    //myStepper.setSpeed(0, FULL_SPEED_ACCELERATION_LINEAR_TIME, FULL_SPEED_DECELERATION_LINEAR_TIME);
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
    #ifdef USE_DENDO_STEPPER
    return(myStepper.getState() == IDLE || myStepper.getState() == DISABLED);
    #elif USE_FASTACCELSTEPPER
    return (!myStepper->isRunning() && !myStepper->isStopping());
    #endif
    
}

void Stepper::UpdateActiveSpeed() {
    //NOTE do not put a lock_guard here, it will cause a deadlock. Many things call this function.
    double targetSpeed = myUseRapidSpeed ? myRapidSpeed : myNormalSpeed;
    double currentSpeed = myStepper.getTargetSpeed();

    if(targetSpeed == currentSpeed) {
        return;
    }

    #ifdef USE_DENDO_STEPPER
    double accTime = myStepper.getAcc();
    double decTime = myStepper.getDec();

    if (targetSpeed > currentSpeed) {
        // Accelerating
        double speedDifference = targetSpeed - currentSpeed;
        accTime = (speedDifference / MAX_DRIVER_STEPS_PER_SECOND) * FULL_SPEED_ACCELERATION_LINEAR_TIME;

        myStepper.setSpeed(targetSpeed, accTime, decTime * 1000);
    } else {
        // Decelerating
        double speedDifference = currentSpeed - targetSpeed;
        decTime = (speedDifference / MAX_DRIVER_STEPS_PER_SECOND) * FULL_SPEED_DECELERATION_LINEAR_TIME;

        myStepper.setSpeed(targetSpeed, accTime, decTime);
    }

 #elif USE_FASTACCELSTEPPER   
    const uint16_t curSpeed = myStepper->getSpeedInMilliHz()/1000;
    const bool isRunning = myStepper->isRunning();
    if(aSpeed != 0 && (curSpeed <= aSpeed - (aSpeed*0.05) || curSpeed >= aSpeed + (aSpeed*0.05))) {
        ESP_LOGI("Stepper", "Speed is not within 0.5%% of target speed, updating");
        ESP_ERROR_CHECK(myStepper->setSpeedInHz(aSpeed));
        myStepper->applySpeedAcceleration();
        #endif
}

void Stepper::UpdateSpeeds(uint16_t aNormalSpeed, uint16_t aRapidSpeed) {
    myNormalSpeed = aNormalSpeed;
    myRapidSpeed = aRapidSpeed;
    UpdateActiveSpeed();
}

void Stepper::MoveLeft() {
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
    myUseRapidSpeed = true;
    UpdateActiveSpeed();
    ESP_LOGI("Stepper", "Rapid speed");
}

void Stepper::SetNormalSpeed() {
    myUseRapidSpeed = false;
    UpdateActiveSpeed();
    ESP_LOGI("Stepper", "Normal speed");
}