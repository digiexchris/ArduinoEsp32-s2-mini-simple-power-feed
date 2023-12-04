#include "state.h"

StateMachine::StateMachine(int dirPin, int enablePin, int stepPin, uint16_t rapidSpeed) : currentState(State::Stopped), currentSpeedState(SpeedState::Normal) {
    myStepper = new Stepper();
    myStepper->Init(dirPin, enablePin, stepPin, rapidSpeed);

    ESP_LOGI("state.cpp", "Stepper init complete");
}

void StateMachine::MoveLeftAction() {
    if(currentState == State::MovingLeft) {
        return;
    }

    ESP_LOGI("state.cpp", "Left pressed");
    currentState = State::MovingLeft;
    myStepper->MoveLeft();
    //ESP_LOGI("state.cpp", "Done requesting stepper move left");
}

void StateMachine::MoveRightAction() {
    if(currentState == State::MovingRight) {
        return;
    }

    ESP_LOGI("state.cpp", "Right pressed");
    currentState = State::MovingRight;
    myStepper->MoveRight();
    //ESP_LOGI("state.cpp", "Done requesting stepper move right");
}

void StateMachine::RapidSpeedAction() {
    if(currentSpeedState == SpeedState::Rapid) {
        return;
    }

    ESP_LOGI("state.cpp", "Rapid pressed");
    currentSpeedState = SpeedState::Rapid;
    myStepper->SetRapidSpeed();
    //ESP_LOGI("state.cpp", "Done requesting stepper set rapid speed");
}

void StateMachine::NormalSpeedAction() {
    if(currentSpeedState == SpeedState::Normal) {
        return;
    }

    ESP_LOGI("state.cpp", "Rapid released");
    currentSpeedState = SpeedState::Normal;
    myStepper->SetNormalSpeed();
    //ESP_LOGI("state.cpp", "Done requesting stepper set normal speed");
}

void StateMachine::UpdateSpeedAction(std::unique_ptr<EventData> eventData) {
    //ESP_LOGI("state.cpp", "Update speeds");
    if(myStepper->IsStopped()) {
        currentState = State::Stopped;
        myStepper->UpdateNormalSpeed(0);
    }
    UpdateSpeedEventData data = *static_cast<UpdateSpeedEventData*>(eventData.get());
    myStepper->UpdateNormalSpeed(data.myNormalSpeed);
    myStepper->UpdateRapidSpeed(data.myRapidSpeed);
    //ESP_LOGI("state.cpp", "Done requesting stepper update speeds");
}

void StateMachine::StopLeftAction() {
    ESP_LOGI("state.cpp", "Stopping");
    currentState = State::StoppingLeft;
    myStepper->Stop();
    //ESP_LOGI("state.cpp", "Done requesting stepper stop");
}

void StateMachine::StopRightAction() {
    ESP_LOGI("state.cpp", "Stopping");
    currentState = State::StoppingRight;
    myStepper->Stop();
    //ESP_LOGI("state.cpp", "Done requesting stepper stop");
}

void StateMachine::processEvent(Event event, std::unique_ptr<EventData> eventData) {
    switch (currentState) {
        case State::Stopped:
            //ESP_LOGI("state.cpp", "State is stopped");
            if (event == Event::LeftPressed) {
                MoveLeftAction();
            } else if (event == Event::RightPressed) {
                MoveRightAction();
            }
            break;

        case State::MovingLeft:
            if (event == Event::LeftReleased) {
                StopLeftAction();
            } 
            break;

        case State::MovingRight:
            if (event == Event::RightReleased) {
                StopRightAction();
            } 
            break;

        //if we are stopping but we ask to resume in the same direction, we can move again immediately.
        //may need to force clear the queue or something.
        case State::StoppingLeft:
            if (event == Event::LeftPressed) {
                MoveLeftAction();
            } 
            break;

        case State::StoppingRight:
            if (event == Event::RightPressed) {
                MoveRightAction();
            } 
            break; 
    }

    switch(event) {
        case Event::RapidPressed:
            RapidSpeedAction();
            break;
        case Event::RapidReleased:
            NormalSpeedAction();
            break;
        case Event::UpdateSpeed:
            UpdateSpeedAction(std::move(eventData));
            break;
        default:
            break;
    }
}
