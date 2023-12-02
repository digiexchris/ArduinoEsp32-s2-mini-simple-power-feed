#ifndef STEPPER_H
#define STEPPER_H

#include "FastAccelStepper.h"
#include <memory>
#include <mutex>

class Stepper {
    public:
    //TODO implement mutexes in all of these!!!
        Stepper();
        void Init(int dirPin, int enablePin, int stepPin, uint16_t rapidSpeed);
        void UpdateNormalSpeed(int16_t speed);
        void UpdateRapidSpeed(int16_t speed);
        void MoveLeft();
        void MoveRight();
        void Stop();
        void SetRapidSpeed();
        void SetNormalSpeed();
        
        bool IsStopped();
    private:
        FastAccelStepper* myStepper;
        void UpdateActiveSpeed();
        
        FastAccelStepperEngine myEngine = FastAccelStepperEngine();
        bool myUseRapidSpeed = false;
        
        uint16_t myRapidSpeed;
        uint16_t myNormalSpeed;
        
        std::mutex myStepperMutex;
};

#endif // STEPPER_H
