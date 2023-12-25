#include "state.h"
#include "config.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>
#include "shared.h"
#include <esp_event_base.h>
#include <esp_event.h>

ESP_EVENT_DEFINE_BASE(COMMAND_EVENT);

StateMachine::StateMachine(std::shared_ptr<Stepper> aStepper, std::shared_ptr<esp_event_loop_handle_t> aUIEventLoop) : currentState(State::Stopped), currentSpeedState(SpeedState::Normal) {
    myStepper = aStepper;
	myRef = this;
	
    ESP_LOGI("state.cpp", "State Machine init complete");
}

void StateMachine::Start()
{
	RegisterEventHandler(COMMAND_EVENT, Event::Any, ProcessEventCallback);
}

void StateMachine::ProcessEventCallback(void *stateMachine, esp_event_base_t base, int32_t id, void *payload) 
{
	StateMachine *sm = static_cast<StateMachine *>(stateMachine);

	Event event = static_cast<Event>(id);
	EventData *eventData = static_cast<EventData *>(payload);
	sm->ProcessEvent(event, eventData);
}

void StateMachine::MoveLeftAction() {
    if(currentState == State::MovingLeft) {
        return;
    }

    ESP_LOGI("state.cpp", "Left pressed");
    currentState = State::MovingLeft;
    myStepper->MoveLeft();
}

void StateMachine::MoveRightAction() {
    if(currentState == State::MovingRight) {
        return;
    }

    ESP_LOGI("state.cpp", "Right pressed");
    currentState = State::MovingRight;
    myStepper->MoveRight();
}

void StateMachine::RapidSpeedAction() {
    if(currentSpeedState == SpeedState::Rapid) {
        return;
    }

    ESP_LOGI("state.cpp", "Rapid pressed");
    currentSpeedState = SpeedState::Rapid;
    myStepper->SetRapidSpeed();
}

void StateMachine::NormalSpeedAction() {
    if(currentSpeedState == SpeedState::Normal) {
        return;
    }

    ESP_LOGI("state.cpp", "Rapid released");
    currentSpeedState = SpeedState::Normal;
    myStepper->SetNormalSpeed();
}

void StateMachine::CheckIfStoppedTask(void* params) {
	StateMachine* sm = static_cast<StateMachine*>(params);
	ASSERT_MSG(sm, "CheckIfStoppedTask", "StateMachine was null on start of task");
	std::shared_ptr<esp_event_loop_handle_t> evht = sm->GetEventLoop();
	ASSERT_MSG(evht, "CheckIfStoppedTask", "Event ringbuf was null on start of task");
	bool isStopped = false;
	while(!isStopped) {
		vTaskDelay(pdMS_TO_TICKS(10));
		
		if(sm->myStepper->IsStopped()) {
			ESP_ERROR_CHECK(esp_event_post_to(*evht, MACHINE_EVENT, static_cast<int32_t>(Event::SetStopped), nullptr, sizeof(nullptr), pdMS_TO_TICKS(250)));
			isStopped = true;
			break;
		}
	}
	vTaskDelete(NULL);
}

void StateMachine::CreateStoppingTask() {
	xTaskCreatePinnedToCore(CheckIfStoppedTask, "processing-stopped", 24000, this, 11, nullptr, 1);
}

void StateMachine::StopLeftAction() {
    ESP_LOGI("state.cpp", "Stopping");
    currentState = State::StoppingLeft;
    myStepper->Stop();
	CreateStoppingTask();
}

void StateMachine::StopRightAction() {
    ESP_LOGI("state.cpp", "Stopping");
    currentState = State::StoppingRight;
    myStepper->Stop();
	CreateStoppingTask();
}

bool StateMachine::ProcessEvent(Event event, EventData* eventPayload) {
    switch (currentState) {
        case State::Stopped:
            //ESP_LOGI("state.cpp", "State is stopped");
            if (event == Event::MoveLeft) {
                MoveLeftAction();
				PublishEvent(STATE_TRANSITION_EVENT, Event::MovingLeft, nullptr);
				return true;
			} else if (event == Event::MoveRight) {
                MoveRightAction();
				PublishEvent(STATE_TRANSITION_EVENT,Event::MovingRight, nullptr);
				return true;
            }
            break;

        case State::MovingLeft:
            if (event == Event::StopMoveLeft) {
                StopLeftAction();
				PublishEvent(STATE_TRANSITION_EVENT, Event::Stopping, nullptr);
				return true;
            } 
            break;

        case State::MovingRight:
            if (event == Event::StopMoveRight) {
				StopRightAction();
				PublishEvent(STATE_TRANSITION_EVENT, Event::Stopping, nullptr);
				return true;
            } 
            break;

        //if we are stopping but we ask to resume in the same direction, we can move again immediately.
        //may need to force clear the queue or something.
        case State::StoppingLeft:
			if (event == Event::MoveLeft)
			{
				MoveLeftAction();
				PublishEvent(STATE_TRANSITION_EVENT, Event::MovingLeft, nullptr);
				return true;
			}
			else if (event == Event::MoveRight)
			{
				// ignore, gotta wait till we're stopped first.
				return false;
			}
			else if (event == Event::SetStopped)
			{
				currentState = State::Stopped;
				PublishEvent(STATE_TRANSITION_EVENT, Event::Stopped, nullptr);
				ESP_LOGI("state.cpp", "Stopped");
			} 
			break;

		case State::StoppingRight:
            if (event == Event::MoveRight) {
                MoveRightAction();
				PublishEvent(STATE_TRANSITION_EVENT, Event::MovingRight, nullptr);
            } 
            else if (event == Event::MoveLeft) {
                //Ignore, gotta wait till we're stopped first.
				return false;
			}
			else if (event == Event::SetStopped)
			{
				currentState = State::Stopped;
				PublishEvent(STATE_TRANSITION_EVENT, Event::Stopped, nullptr);
				ESP_LOGI("state.cpp", "Stopped");
			}
			break; 

        default:    
            break;
    }

	// UI handles these commands directly, so we don't need to publish them to it.
    switch(event) {
        case Event::RapidSpeed:
			RapidSpeedAction();
			break;
		case Event::NormalSpeed:
			NormalSpeedAction();
			break;
		case Event::UpdateRapidSpeed: 
		{
			UpdateSpeedEventData* eventData = dynamic_cast<UpdateSpeedEventData*>(eventPayload);
			ASSERT_MSG(eventData, "StateMachine", "Failed to cast event data to UpdateSpeedEventData");

			int16_t speed = eventData->mySpeed;
			ESP_LOGI("StateMachine", "Updating rapid speed to %d", speed);

			myStepper->UpdateRapidSpeed(speed);
			break;
		}
		case Event::UpdateNormalSpeed: 
		{
			UpdateSpeedEventData *eventData = dynamic_cast<UpdateSpeedEventData *>(eventPayload);
			ASSERT_MSG(eventData, "StateMachine", "Failed to cast event data to UpdateSpeedEventData");

			int16_t speed = eventData->mySpeed;
			ESP_LOGI("StateMachine", "Updating normal speed to %d", speed);

			myStepper->UpdateNormalSpeed(speed);
			break;
		}
		default:
			break;
    }

	return true;
}

