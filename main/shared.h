
#pragma once
#include <memory>
class StateMachine;
class SpeedUpdateHandler;
static DRAM_ATTR std::shared_ptr<SpeedUpdateHandler> mySpeedUpdateHandler;
static DRAM_ATTR std::shared_ptr<StateMachine> myState;
static DRAM_ATTR std::shared_ptr<Stepper> myStepper;

