#include "state.h"
#include "config.h"
#include <esp_log.h>

StateMachine::StateMachine(int dirPin, int enablePin, int stepPin, uint16_t rapidSpeed) : currentState(State::Stopped), currentSpeedState(SpeedState::Normal) {
    myStepper = new Stepper();
    myStepper->Init(dirPin, enablePin, stepPin, rapidSpeed);
    myEventQueue = xQueueCreate( 10, sizeof( Event ) );
    xTaskCreate(&StateMachine::ProcessEventQueueTask, "ProcessEventQueueTask", 2048*16, this, 5, NULL);
    myUpdateSpeedQueue = xQueueCreate( 10, sizeof( UpdateSpeedEventData ) );
    xTaskCreate(&StateMachine::ProcessUpdateSpeedQueueTask, "ProcessUpdateSpeedQueueTask", 2048*8, this, 5, NULL);
    ESP_LOGI("state.cpp", "Stepper init complete");
}

void StateMachine::AddEvent(Event event) {
    //ESP_LOGI("state.cpp", "Adding event to queue");
    if(xQueueSend( myEventQueue, &event, 0 ) != pdPASS) {
        ESP_LOGE("state.cpp", "Failed to send event to queue");
        //return false;
    }
    //ESP_LOGI("state.cpp", "Event added to queue");
    //return true;
}

void StateMachine::ProcessEventQueueTask(void* params) {
    StateMachine* stateMachine = static_cast<StateMachine*>(params);
    while(true) {
        
        Event event;
        xQueueReceive(stateMachine->myEventQueue, &event, portMAX_DELAY);
        stateMachine->processEvent(event);
    }
}

void StateMachine::ProcessUpdateSpeedQueueTask(void* params) {
    StateMachine* stateMachine = static_cast<StateMachine*>(params);
    while(true) {
        
        UpdateSpeedEventData eventData;
        xQueueReceive(stateMachine->myUpdateSpeedQueue, &eventData, portMAX_DELAY);
        stateMachine->processSpeedEvent(eventData);
    }
}

bool StateMachine::AddUpdateSpeedEvent(UpdateSpeedEventData* eventData) {
    //ESP_LOGI("state.cpp", "Adding update speed event to queue");
    if(xQueueSend( myUpdateSpeedQueue, eventData, 0 ) != pdPASS) {
        ESP_LOGE("state.cpp", "Failed to send update speed event to queue");
        return false;
    }
    //ESP_LOGI("state.cpp", "Update speed event added to queue");
    return true;
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

void StateMachine::UpdateSpeedAction(UpdateSpeedEventData eventData) {
    //ESP_LOGI("state.cpp", "Update speeds");
    if(myStepper->IsStopped()) {
        currentState = State::Stopped;
        //myStepper->UpdateNormalSpeed(0);
    }
    myStepper->UpdateNormalSpeed(eventData.myNormalSpeed);
    myStepper->UpdateRapidSpeed(eventData.myRapidSpeed);
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

void StateMachine::processEvent(Event event) {
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
            else if (event == Event::RightPressed) {
                //Requeue, gotta wait till we're stopped first.
                AddEvent(event);
            }
            break;

        case State::StoppingRight:
            if (event == Event::RightPressed) {
                MoveRightAction();
            } 
            else if (event == Event::LeftPressed) {
                //Requeue, gotta wait till we're stopped first.
                AddEvent(event);
            }
            break; 

        default:    
            break;
    }

    switch(event) {
        case Event::RapidPressed:
        RapidSpeedAction();
        break;
    case Event::RapidReleased:
        NormalSpeedAction();
        break;
    default:
        break;
    }
}

void StateMachine::processSpeedEvent(UpdateSpeedEventData eventData) {
            UpdateSpeedAction(std::move(eventData));
}
