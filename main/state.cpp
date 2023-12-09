#include "state.h"
#include "config.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>
#include "shared.h"

StateMachine::StateMachine(std::shared_ptr<Stepper> aStepper) : currentState(State::Stopped), currentSpeedState(SpeedState::Normal) {
    myStepper = aStepper;
	myEventRingBuf = xRingbufferCreate(1024, RINGBUF_TYPE_NOSPLIT);
	if (!myEventRingBuf)
	{
		ESP_LOGE("state.cpp", "Failed to create event queue");
	}

	myUpdateSpeedRingbuf = xRingbufferCreate(1024, RINGBUF_TYPE_NOSPLIT);;
	if (!myUpdateSpeedRingbuf)
	{
		ESP_LOGE("state.cpp", "Failed to create update speed queue");
	}
//	myUpdateSpeedQueue->
	
    ESP_LOGI("state.cpp", "State Machine init complete");
}

void StateMachine::ProcessEventQueueTask(void* params) {
    StateMachine* sm = static_cast<StateMachine*>(params);
	if (!sm)
	{
		ESP_LOGE("state.cpp", "Failed to cast params to StateMachine while starting ProcessUpdateSpeedQueueTask");
	}

	RingbufHandle_t ringBuf = sm->GetEventRingBuf();
	if (!ringBuf)
	{
	    ESP_LOGE("state.cpp", "Failed to get event queue while starting ProcessEventQueueTask");
	}
	while(true) {
       

		size_t item_size;
		Event *event = (Event *)xRingbufferReceive(ringBuf, &item_size, portMAX_DELAY);

		// Check received item
		if (!item_size)
		{
			if (!sm->ProcessEvent(*event))
			{
				xRingbufferSend(ringBuf, (void *)new Event(*event), sizeof(event), pdMS_TO_TICKS(50));
			}

			vRingbufferReturnItem(ringBuf, (void *)event);
		}
    }
}

void StateMachine::Start() {
	xTaskCreate(&StateMachine::ProcessEventQueueTask, "ProcessEventQueueTask", 2048 * 16, this, 5, NULL);
	xTaskCreate(&StateMachine::ProcessUpdateSpeedQueueTask, "ProcessUpdateSpeedQueueTask", 2048 * 8, this, 5, NULL);
}

void StateMachine::ProcessUpdateSpeedQueueTask(void* params) {
	StateMachine* sm = static_cast<StateMachine*>(params);
	RingbufHandle_t ringBuf = sm->GetUpdateSpeedQueue();
	if(!sm) {
		ESP_LOGE("state.cpp", "Failed to cast params to StateMachine while starting ProcessUpdateSpeedQueueTask");
	}
	if (!ringBuf) {
		ESP_LOGE("state.cpp", "Failed to get update speed queue while starting ProcessUpdateSpeedQueueTask");
	}
    while(true) {
        size_t item_size;
		UpdateSpeedEventData *eventData = (UpdateSpeedEventData *)xRingbufferReceive(ringBuf, &item_size, portMAX_DELAY);
		if (!item_size)
		{
			// ignore speed changes while stopping
			if (sm->GetState() == State::StoppingLeft || sm->GetState() == State::StoppingRight)
			{
				xRingbufferSend(ringBuf,(void*) new UpdateSpeedEventData(*eventData), sizeof(eventData), pdMS_TO_TICKS(1000));
			}
			else
			{
				sm->myStepper->UpdateSpeeds(eventData->myNormalSpeed, eventData->myRapidSpeed);
			}

			vRingbufferReturnItem(ringBuf, (void *)eventData);
		}

		
	}
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

bool StateMachine::ProcessEvent(Event event) {
    switch (currentState) {
        case State::Stopped:
            //ESP_LOGI("state.cpp", "State is stopped");
            if (event == Event::LeftPressed) {
                MoveLeftAction();
				return true;
			} else if (event == Event::RightPressed) {
                MoveRightAction();
				return true;
            }
            break;

        case State::MovingLeft:
            if (event == Event::LeftReleased) {
                StopLeftAction();
				return true;
            } 
            break;

        case State::MovingRight:
            if (event == Event::RightReleased) {
                StopRightAction();
				return true;
            } 
            break;

        //if we are stopping but we ask to resume in the same direction, we can move again immediately.
        //may need to force clear the queue or something.
        case State::StoppingLeft:
            if (event == Event::LeftPressed) {
                MoveLeftAction();
				return true;
            } 
            else if (event == Event::RightPressed) {
                //Requeue, gotta wait till we're stopped first.
				return false;
			}
            break;

        case State::StoppingRight:
            if (event == Event::RightPressed) {
                MoveRightAction();
            } 
            else if (event == Event::LeftPressed) {
                //Requeue, gotta wait till we're stopped first.
				return false;
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

	return true;
}

