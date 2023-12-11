#include "state.h"
#include "config.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>
#include "shared.h"
#include <esp_event_base.h>
#include <esp_event.h>

ESP_EVENT_DEFINE_BASE(STATE_MACHINE_EVENT);

StateMachine::StateMachine(std::shared_ptr<Stepper> aStepper) : currentState(State::Stopped), currentSpeedState(SpeedState::Normal) {
    myStepper = aStepper;
	myRef = this;

	esp_event_loop_args_t loopArgs = {
		.queue_size = 16,
		.task_name = "StateMachineEventLoop", // task will be created
		.task_priority = uxTaskPriorityGet(NULL),
		.task_stack_size = 3072,
		.task_core_id = tskNO_AFFINITY};

	ESP_ERROR_CHECK(esp_event_loop_create(&loopArgs, &myEventLoop));
	//myEventLoop = xRingbufferCreate(sizeof(Event)*1024, RINGBUF_TYPE_NOSPLIT);
	ASSERT_MSG(myEventLoop, "StateMachine", "Failed to create event ringbuf");

	//myUpdateSpeedEventLoop = xRingbufferCreate(sizeof(UpdateSpeedEventData)*1024*3, RINGBUF_TYPE_NOSPLIT);;
	//ASSERT_MSG(myUpdateSpeedEventLoop, "StateMachine", "Failed to create update speed ringbuf");
	
    ESP_LOGI("state.cpp", "State Machine init complete");
}

void StateMachine::Start()
{
//	esp_event_handler_register()
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(myEventLoop, STATE_MACHINE_EVENT, ESP_EVENT_ANY_ID, ProcessEventLoopIteration, myRef, nullptr));
//esp_event_handler_instance_t
	xTaskCreate(StateMachine::EventLoopRunnerTask, "StateMachineEventLoop", 3072*5, this, uxTaskPriorityGet(NULL) + 1, &myEventLoopTaskHandle);

	//	xTaskCreate(&StateMachine::ProcessEventQueueTask, "ProcessEventQueueTask", 2048 * 24, this, 5, NULL);
	//	xTaskCreate(&StateMachine::ProcessUpdateSpeedQueueTask, "ProcessUpdateSpeedQueueTask", 1024 * 24, this, 5, NULL);
}

void StateMachine::EventLoopRunnerTask(void *stateMachine)
{
	StateMachine *sm = static_cast<StateMachine *>(stateMachine);
	while (1)
	{
		esp_event_loop_run(sm->myEventLoop, 100);
		vTaskDelay(10);
	}
}

void StateMachine::ProcessEventLoopIteration(void *stateMachine, esp_event_base_t base, int32_t id, void *payload) {
	//ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	StateMachine *sm = static_cast<StateMachine *>(stateMachine);
	ASSERT_MSG(sm, "ProccessEventQueueTask", "StateMachine was null on start of task");
	
	Event event = static_cast<Event>(id);
	
	EventData *eventData = static_cast<EventData *>(payload);

	sm->ProcessEvent(event, eventData);

	//delete eventData;
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

void StateMachine::CheckIfStoppedTask(void* params) {
	StateMachine* sm = static_cast<StateMachine*>(params);
	ASSERT_MSG(sm, "CheckIfStoppedTask", "StateMachine was null on start of task");
	esp_event_loop_handle_t evht = sm->GetEventLoop();
	ASSERT_MSG(evht, "CheckIfStoppedTask", "Event ringbuf was null on start of task");
	bool isStopped = false;
	while(!isStopped) {
		vTaskDelay(pdMS_TO_TICKS(10));
		
		if(sm->myStepper->IsStopped()) {
			ESP_ERROR_CHECK(esp_event_post_to(evht, STATE_MACHINE_EVENT, static_cast<int32_t>(Event::SetStopped), nullptr, sizeof(nullptr), pdMS_TO_TICKS(250)));
			isStopped = true;
			break;
		}
	}
	vTaskDelete(NULL);
}

void StateMachine::CreateStoppingTask() {
	xTaskCreatePinnedToCore(CheckIfStoppedTask, "processing-stopped", 24000, this, 1, nullptr, 0);
}

void StateMachine::StopLeftAction() {
    ESP_LOGI("state.cpp", "Stopping");
    currentState = State::StoppingLeft;
    myStepper->Stop();
	CreateStoppingTask();
	//ESP_LOGI("state.cpp", "Done requesting stepper stop");
}

void StateMachine::StopRightAction() {
    ESP_LOGI("state.cpp", "Stopping");
    currentState = State::StoppingRight;
    myStepper->Stop();
	CreateStoppingTask();
    //ESP_LOGI("state.cpp", "Done requesting stepper stop");
}

bool StateMachine::ProcessEvent(Event event, EventData* eventPayload) {
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
			if (event == Event::LeftPressed)
			{
				MoveLeftAction();
				return true;
			}
			else if (event == Event::RightPressed)
			{
				// Requeue, gotta wait till we're stopped first.
				return false;
			}
			else if (event == Event::SetStopped)
			{
				currentState = State::Stopped;
				ESP_LOGI("state.cpp", "Stopped");
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
			else if (event == Event::SetStopped)
			{
				currentState = State::Stopped;
				ESP_LOGI("state.cpp", "Stopped");
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
		case Event::UpdateSpeed: 
		{
			UpdateSpeedEventData* eventData = dynamic_cast<UpdateSpeedEventData*>(eventPayload);
			ASSERT_MSG(eventData, "StateMachine", "Failed to cast event data to UpdateSpeedEventData");
			
			auto speed = eventData->myNormalSpeed;
			//auto rapidSpeed = eventData->myRapidSpeed;
			ESP_LOGI("StateMachine", "Updating speed to %d", speed);
			
			myStepper->UpdateSpeeds(eventData->myNormalSpeed, eventData->myRapidSpeed);
			break;
		}
		default:
			break;
    }

	return true;
}

