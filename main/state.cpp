#include "state.h"

StateMachine::StateMachine(Stepper* aStepper) : currentState(State::Stopped), myStepper(aStepper) {}

void StateMachine::processEvent(Event event, std::unique_ptr<EventData> data = nullptr) {
    switch (currentState) {
        case State::Stopped:
            if (event == Event::LeftPressed) {
                currentState = State::Moving;
                myStepper->MoveLeft();
            } else if (event == Event::RightPressed) {
                currentState = State::Moving;
                myStepper->MoveRight();
            } else if (event == Event::RapidReleased) {
                myStepper->SetNormalSpeed();
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
                currentState = State::MovingRapid;
                myStepper->SetRapidSpeed();
            } else if (event == Event::RapidReleased) {
                myStepper->SetNormalSpeed();
            } else if (event == Event::UpdateSpeed) {
                myStepper->UpdateNormalSpeed(static_cast<UpdateSpeedEventData*>(data)->myNormalSpeed);
                myStepper->UpdateRapidSpeed(static_cast<UpdateSpeedEventData*>(data)->myRapidSpeed);
            }
            break;

        case State::MovingRapid:
            if (event == Event::LeftReleased) {
                currentState = State::Stopping;
                myStepper->Stop();
            } else if (event == Event::RightReleased) {
                currentState = State::Stopping;
                myStepper->Stop();
            }  else if (event == Event::RapidReleased) {
                currentState = State::Moving;
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
            } else if (event == Event::RapidReleased) {
                myStepper->SetNormalSpeed();
            }
            break;
        
    }
}