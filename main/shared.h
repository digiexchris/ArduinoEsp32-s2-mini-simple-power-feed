
#pragma once
#include <memory>
class StateMachine;
class SpeedUpdateHandler;
static std::shared_ptr<SpeedUpdateHandler> mySpeedUpdateHandler;
static std::shared_ptr<StateMachine> myState;
static std::shared_ptr<Stepper> myStepper;

