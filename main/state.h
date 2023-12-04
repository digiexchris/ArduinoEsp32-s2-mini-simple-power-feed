
#ifndef STATE_H
#define STATE_H

#include <memory>
#include "stepper.h"

enum class State {
    MovingLeft,
    MovingRight,
    StoppingLeft,
    StoppingRight,
    Stopped
};

enum class SpeedState {
    Normal,
    Rapid
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
    StateMachine(int dirPin, int enablePin, int stepPin, uint16_t rapidSpeed);

    void processEvent(Event event, std::unique_ptr<EventData> data = nullptr);

private:
    void MoveLeftAction();
    void MoveRightAction();
    void RapidSpeedAction();
    void NormalSpeedAction();
    void UpdateSpeedAction(std::unique_ptr<EventData> data);
    void StopLeftAction();
    void StopRightAction();

    State currentState;
    SpeedState currentSpeedState; //TODO probably don't need this, if we can update using debouncer.Changed()
    Stepper* myStepper;
};

#endif // STATE_H
