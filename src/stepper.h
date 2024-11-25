#ifndef STEPPER_H
#define STEPPER_H
#include "Event.h"
#include "EventTypes.h"
#include "config.h"

#include <StepTiming.hpp>
#include <memory>
#include <mutex>

#include "Event.h"

class Stepper : public EventPublisher
{
public:
	enum class StepperDirection
	{
		Left = true,
		Right = false
	};
	Stepper();
	Stepper(int16_t aRapidSpeed, int16_t aNormalSpeed);
	void Init(uint8_t dirPin, uint8_t enablePin, uint8_t stepPin, int16_t rapidSpeed, int16_t normalSpeed);
	void UpdateNormalSpeed(int16_t aNormalSpeedDelta);
	void UpdateRapidSpeed(int16_t aRapidSpeedDelta);
	void MoveLeft();
	void MoveRight();
	void Stop();
	void SetRapidSpeed();
	void SetNormalSpeed();

	/**
	 **@brief Get the current speed in hz
	 **@return The current speed in hz
	 **/
#ifdef USE_DENDO_STEPPER
	uint16_t GetCurrentSpeed()
	{
		return myStepper.getSpeed();
	}
#elif USE_FASTACCELSTEPPER
	uint16_t GetCurrentSpeed()
	{
		return myStepper->getCurrentSpeedInMilliHz() / 1000;
	}
#endif

#ifdef USE_DENDO_STEPPER
	uint16_t GetTargetSpeed()
	{
		return myStepper.getTargetSpeed();
	}
#elif USE_FASTACCELSTEPPER
	uint16_t GetTargetSpeed()
	{
		return myStepper->getSpeedInMilliHz() / 1000;
	}
#endif

	std::string GetState();

	bool IsStopped();

private:
	void UpdateActiveSpeed();

#ifdef USE_DENDO_STEPPER
	DendoStepper myStepper;
	DendoStepper_config_t myStepperCfg;
#elif USE_FASTACCELSTEPPER
	FastAccelStepperEngine myEngine = FastAccelStepperEngine();
	FastAccelStepper *myStepper;
#endif
	bool myUseRapidSpeed = false;

	int16_t myRapidSpeed;
	int16_t myNormalSpeed;
};

#endif // STEPPER_H
