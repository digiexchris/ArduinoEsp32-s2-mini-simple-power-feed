
#ifndef STATE_H
#define STATE_H

#include <memory>
#include "stepper.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "shared.h"

#include "esp_event_base.h"
#include "esp_event.h"
#include "ui.h"
#include "EventTypes.h"

ESP_EVENT_DECLARE_BASE(STATE_MACHINE_EVENT);

class StateMachine {
public:
  StateMachine(std::shared_ptr<Stepper> aStepper);
  void Start();

  State GetState() { return currentState; }
  //RingbufHandle_t GetEventRingBuf() { return myEventLoop; }
  std::shared_ptr<esp_event_loop_handle_t> GetEventLoop() { return myEventLoop; }

  //RingbufHandle_t GetUpdateSpeedQueue() { return myUpdateSpeedEventLoop; }

private:
    void MoveLeftAction();
    void MoveRightAction();
    void RapidSpeedAction();
    void NormalSpeedAction();
    void StopLeftAction();
    void StopRightAction();

	static void CheckIfStoppedTask(void* params);

	void CreateStoppingTask();

	static void EventLoopRunnerTask(void *args);
	static void ProcessEventLoopIteration(void *stateMachine, esp_event_base_t base, int32_t id, void *eventData);
	//static void ProcessUpdateSpeedQueueTask(void *params);
    bool ProcessEvent(Event event, EventData* eventData);

    State currentState;
	UI* myUI;
	SpeedState currentSpeedState; //TODO probably don't need this, if we can update using debouncer.Changed()
	std::shared_ptr<Stepper> myStepper;
    TaskHandle_t myProcessQueueTaskHandle;
	std::shared_ptr<esp_event_loop_handle_t> myEventLoop;
	std::shared_ptr <esp_event_loop_handle_t> myUiEventLoop;
	TaskHandle_t myEventLoopTaskHandle;
	StateMachine* myRef;
	//esp_event_loop_handle_t myUpdateSpeedEventLoop;
};

#endif // STATE_H
