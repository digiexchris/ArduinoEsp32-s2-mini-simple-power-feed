#include "stepper.h"
#include "config.h"

#ifdef USE_FASTACCELSTEPPER
//#include "FastAccelStepper.h"
#elif USE_DENDO_STEPPER
#include "DendoStepper.h"
#else
#error "No stepper library defined"
#endif

#include <algorithm>
#include <mutex>

#include "ui.h"

Stepper::Stepper()
{
	myUseRapidSpeed = false;
	myRapidSpeed = 0;
	myNormalSpeed = maxStepsPerSecond;
}

Stepper::Stepper(int16_t aRapidSpeed, int16_t aNormalSpeed)
{
	myUseRapidSpeed = false;
	myRapidSpeed = aRapidSpeed;
	myNormalSpeed = aNormalSpeed;
}

void Stepper::Init(uint8_t dirPin, uint8_t enablePin, uint8_t stepPin, int16_t rapidSpeed, int16_t normalSpeed)
{

	my

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
#elif USE_FASTACCELSTEPPER
		myEngine.init(1);
	myStepper = myEngine.stepperConnectToPin(stepPin);
	myRapidSpeed = rapidSpeed;
	myNormalSpeed = normalSpeed;
	if (myStepper)
	{
		myStepper->setDirectionPin(dirPin);
		myStepper->setEnablePin(enablePin);
		myStepper->setAutoEnable(true);
		myStepper->setDelayToDisable(200);
		myStepper->setDelayToEnable(50); // this is the time the enable pin must be active before the stepper starts moving
		myStepper->setAcceleration(20000);
		// myStepper->enableOutputs();
	}
#endif

	printf("Stepper", "Stepper init complete");
}

bool Stepper::IsStopped()
{
#ifdef USE_DENDO_STEPPER
	return (myStepper.getState() == IDLE || myStepper.getState() == DISABLED);
#elif USE_FASTACCELSTEPPER
	return (!myStepper->isRunning() && !myStepper->isStopping());
#endif
}

void Stepper::UpdateActiveSpeed()
{
	// NOTE do not put a lock_guard here, it will cause a deadlock. Many things call this function.
	double targetSpeed = myUseRapidSpeed ? myRapidSpeed : myNormalSpeed;

#ifdef USE_DENDO_STEPPER
	double currentSpeed = myStepper.getTargetSpeed();

	if (targetSpeed == currentSpeed)
	{
		return;
	}
	double accTime = myStepper.getAcc();
	double decTime = myStepper.getDec();

	if (targetSpeed > currentSpeed)
	{
		// Accelerating
		double speedDifference = targetSpeed - currentSpeed;
		accTime = (speedDifference / MAX_DRIVER_STEPS_PER_SECOND) * FULL_SPEED_ACCELERATION_LINEAR_TIME;

		// this becomes the stopping speed
		decTime = (targetSpeed / MAX_DRIVER_STEPS_PER_SECOND) * FULL_SPEED_DECELERATION_LINEAR_TIME;

		myStepper.setSpeed(targetSpeed, accTime, decTime);
	}
	else
	{
		// Decelerating
		double speedDifference = currentSpeed - targetSpeed;
		decTime = (speedDifference / MAX_DRIVER_STEPS_PER_SECOND) * FULL_SPEED_DECELERATION_LINEAR_TIME;

		myStepper.setSpeed(targetSpeed, accTime, decTime);
	}

#elif USE_FASTACCELSTEPPER
	const double currentSpeed = myStepper->getSpeedInMilliHz() / 1000;
	if (targetSpeed == currentSpeed)
	{
		return;
	}

	//    const bool isRunning = myStepper->isRunning();
	//    if(aSpeed != 0 && (curSpeed <= aSpeed - (aSpeed*0.05) || curSpeed >= aSpeed + (aSpeed*0.05))) {
	//        printf("Stepper", "Speed is not within 0.5%% of target speed, updating");
	myStepper->setSpeedInHz(targetSpeed);
	myStepper->applySpeedAcceleration();
#endif
}

void Stepper::UpdateRapidSpeed(int16_t aRapidSpeedDelta)
{
	myRapidSpeed += aRapidSpeedDelta;

	auto *encoderEventData = new SingleValueEventData<int32_t>(myRapidSpeed);
	PublishEvent(SETTINGS_EVENT, Event::SaveRapidSpeed, encoderEventData);
	auto *uiEncoderEventData = new SingleValueEventData<int32_t>(aRapidSpeedDelta);
	PublishEvent(UI_EVENT, Event::UpdateSpeed, uiEncoderEventData);

	UpdateActiveSpeed();
}

void Stepper::UpdateNormalSpeed(int16_t aNormalSpeedDelta)
{
	// cap it at the rapid speed
	myNormalSpeed += aNormalSpeedDelta;
	if (myNormalSpeed > myRapidSpeed)
	{
		myNormalSpeed = myRapidSpeed; // cap it at the rapid speed
	}

	auto *encoderEventData = new SingleValueEventData<int32_t>(myNormalSpeed);
	PublishEvent(SETTINGS_EVENT, Event::SaveNormalSpeed, encoderEventData);
	auto *uiEncoderEventData = new SingleValueEventData<int32_t>(aNormalSpeedDelta);
	PublishEvent(UI_EVENT, Event::UpdateSpeed, uiEncoderEventData);

	UpdateActiveSpeed();
}

void Stepper::MoveLeft()
{
	// stepper.moveleft()
	UpdateActiveSpeed();
	printf("Stepper", "Moving left");
}

void Stepper::MoveRight()
{
	// move_right()
	UpdateActiveSpeed();
	printf("Stepper", "Moving right");
}

void Stepper::Stop()
{

	printf("Stepper", "Stopping");
}

std::string Stepper::GetState()
{

#ifdef USE_DENDO_STEPPER
	switch (myStepper.getState())
	{

	case IDLE:
		return "IDLE";
		break;
	case ACC:
		return "ACCELERATING";
		break;
	case DEC:
		return "DECELERATING";
		break;
	case COAST:
		return "COASTING";
		break;
	case DISABLED:
		return "DISABLED";
		break;
	}

#elif USE_FASTACCELSTEPPER
	if (myStepper->isRunning())
	{
		return "RUNNING";
	}
	else if (myStepper->isStopping())
	{
		return "STOPPING";
	}
	else
	{
		return "STOPPED";
	}
#endif
	return "ERROR";
}

void Stepper::SetRapidSpeed()
{
	myUseRapidSpeed = true;
	UpdateActiveSpeed();
	printf("Stepper", "Rapid speed");
}

void Stepper::SetNormalSpeed()
{
	myUseRapidSpeed = false;
	UpdateActiveSpeed();
	printf("Stepper", "Normal speed");
}