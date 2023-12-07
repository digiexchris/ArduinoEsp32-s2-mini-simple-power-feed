#ifndef STEPPER_H
#define STEPPER_H
#include "config.h"

#ifdef USE_DENDO_STEPPER
#include "DendoStepper.h"
#elif USE_FASTACCELSTEPPER
#include "FastAccelStepper.h"
#endif
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
        void UpdateSpeeds(uint16_t aNormalSpeed, uint16_t aRapidSpeed);
        void MoveLeft();
        void MoveRight();
        void Stop();
        void SetRapidSpeed();
        void SetNormalSpeed();

		uint16_t GetCurrentSpeed() {
			return myStepper.getSpeed();
		}

		std::string GetState();

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
};

#endif // STEPPER_H
