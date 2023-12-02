#pragma once
#include "FastAccelStepper.h"
#include <memory>

class Stepper {
    public:
        Stepper(int dirPin, int enablePin, int stepPin);
        void UpdateSpeed(int32_t speed);
    private:
        std::shared_ptr<FastAccelStepper> myStepper;
        FastAccelStepperEngine myEngine = FastAccelStepperEngine();
};