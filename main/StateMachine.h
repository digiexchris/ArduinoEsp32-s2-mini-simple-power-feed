
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
#include "Event.h"


class UI;
class StateMachine: public EventHandler, EventPublisher{
public:
	StateMachine(std::shared_ptr<Stepper> aStepper);
	void Start();
	State GetState();
private:
    void MoveLeftAction();
    void MoveRightAction();
    void RapidSpeedAction();
    void NormalSpeedAction();
    void StopLeftAction();
    void StopRightAction();

	static void CheckIfStoppedTask(void* params);

	void CreateStoppingTask();

	static void ProcessEventCallback(void *stateMachine, esp_event_base_t base, int32_t id, void *eventData);
    bool ProcessEvent(Event event, EventData* eventData);

    State currentState;
	UI* myUI;
	SpeedState currentSpeedState;
	std::shared_ptr<Stepper> myStepper;
	StateMachine* myRef;
};

#endif // STATE_H
