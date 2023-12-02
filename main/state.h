#pragma once
#include <memory>
#include "stepper.h"

enum class State {
    Moving,
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
    : myNormalSpeed(aNormalSpeed),
    myRapidSpeed(aRapidSpeed) {};
    int16_t myNormalSpeed;
    int16_t myRapidSpeed;
};

class StateMachine {
public:
    StateMachine(std::shared_ptr<Stepper> aStepper) : currentState(State::Stopped), myStepper(aStepper) {}

    void processEvent(Event event, EventData* data = nullptr) {
        switch (currentState) {
            case State::Stopped:
                if (event == Event::LeftPressed) {
                    currentState = State::Moving;
                    myStepper->MoveLeft();
                } else if (event == Event::RightPressed) {
                    currentState = State::Moving;
                    myStepper->MoveRight();
                }
                break;

            case State::Moving:
                if (event == Event::LeftReleased) {
                    currentState = State::Stopping;
                    myStepper->Stop();
                } else if (event == Event::RightReleased) {
                    currentState = State::Stopping;
                    myStepper->Stop();
                } else if (event == Event::RapidPressed) {
                    currentState = State::Moving;
                    myStepper->SetRapidSpeed();
                } else if (event == Event::RapidReleased) {
                    myStepper->SetNormalSpeed();
                } else if (event == Event::UpdateSpeed) {
                    myStepper->UpdateNormalSpeed(static_cast<UpdateSpeedEventData*>(data)->myNormalSpeed);
                    myStepper->UpdateRapidSpeed(static_cast<UpdateSpeedEventData*>(data)->myRapidSpeed);
                }
                break;

            //if we are stopping but we ask to resume in the same direction, we can move again immediately.
            //may need to force clear the queue or something.
            case State::Stopping:
                if (event == Event::LeftPressed) {
                    currentState = State::Moving;
                    myStepper->MoveLeft();
                } else if (event == Event::RightPressed) {
                    currentState = State::Moving;
                    myStepper->MoveRight();
                }
                break;
        }
    }

private:
    State currentState;
    std::shared_ptr<Stepper> myStepper;
};

