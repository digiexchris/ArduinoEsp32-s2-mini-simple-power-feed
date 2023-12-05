#ifndef STEPPER_H
#define STEPPER_H

#include "FastAccelStepper.h"
#include <memory>
#include <mutex>

class Stepper {
    public:

    enum class StepperDirection {
        Left = true,
        Right = false
    };
    //TODO implement mutexes in all of these!!!
        Stepper();
        void Init(uint8_t dirPin, uint8_t enablePin, uint8_t stepPin, uint16_t rapidSpeed);
        void UpdateNormalSpeed(int16_t speed);
        void UpdateRapidSpeed(int16_t speed);
        void MoveLeft();
        void MoveRight();
        void Stop();
        void SetRapidSpeed();
        void SetNormalSpeed();
        
        bool IsStopped();
    private:
        
        void UpdateActiveSpeed();
        
        #ifdef USE_DENDO_STEPPER
        DendoStepper myStepper;
        DendoStepper_config_t myStepperCfg;
        #elif USE_FASTACCELSTEPPER
        FastAccelStepperEngine myEngine = FastAccelStepperEngine();
        FastAccelStepper* myStepper;
        #endif
        bool myUseRapidSpeed = false;
        
        uint16_t myRapidSpeed;
        uint16_t myNormalSpeed;
        
        std::mutex myStepperMutex;
};

#endif // STEPPER_H
