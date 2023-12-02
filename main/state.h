#pragma once
#include <memory>
#include "stepper.h"

enum class State {
    Moving,
    MovingRapid,
    Stopping,
    Stopped
};

enum class Event {
    LeftPressed,
    LeftReleased,
    RightPressed,
    RightReleased,
    RapidPressed,
    RapidReleased,
    UpdateSpeed
};

class EventData {

};

class UpdateSpeedEventData : public EventData {
public:
    UpdateSpeedEventData(int16_t aNormalSpeed, int16_t aRapidSpeed) 
        :   myNormalSpeed(aNormalSpeed),
            myRapidSpeed(aRapidSpeed) {};
    int16_t myNormalSpeed;
    int16_t myRapidSpeed;
};

class StateMachine {
public:
    StateMachine(Stepper* aStepper);

    void processEvent(Event event, std::unique_ptr<EventData> data = nullptr);

private:
    State currentState;
    Stepper* myStepper;
};
