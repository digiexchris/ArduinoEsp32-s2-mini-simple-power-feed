#pragma once
#include "FastAccelStepper.h"
#include <memory>

class Stepper {
    public:
        Stepper(int dirPin, int enablePin, int stepPin, uint16_t rapidSpeed);
        void UpdateNormalSpeed(int16_t speed);
        void UpdateRapidSpeed(int16_t speed);
        void MoveLeft();
        void MoveRight();
        void Stop();
        void SetRapidSpeed();
        void SetNormalSpeed();
        void UpdateActiveSpeed();
        uint16_t GetNormalSpeed();
    private:
        std::shared_ptr<FastAccelStepper> myStepper;
        FastAccelStepperEngine myEngine = FastAccelStepperEngine();
        bool myUseRapidSpeed = false;
        uint16_t myRapidSpeed;
        uint16_t myNormalSpeed;
};