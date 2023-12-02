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

class StateMachine {
public:
    StateMachine(std::shared_ptr<Stepper> aStepper) : currentState(State::Stopped), myStepper(aStepper) {}

    void processEvent(Event event) {
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
                    myStepper->UpdateSpeed(/**placeholder for the speed passed in the event*/);
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

