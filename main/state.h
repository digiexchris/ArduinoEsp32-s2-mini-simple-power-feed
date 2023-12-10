
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


enum class State {
    MovingLeft,
    MovingRight,
    StoppingLeft,
    StoppingRight,
    Stopped
};

enum class SpeedState {
    Normal,
    Rapid
};

ESP_EVENT_DECLARE_BASE(STATE_MACHINE_EVENT);

enum class Event {
    LeftPressed,
    LeftReleased,
    RightPressed,
    RightReleased,
    RapidPressed,
    RapidReleased,
    UpdateSpeed,
	SetStopped
};



class EventData {
  public:
		EventData() {};
		virtual ~EventData() {};
};

class UpdateSpeedEventData : public EventData {
public:
    UpdateSpeedEventData() {};
    UpdateSpeedEventData(int16_t aNormalSpeed, int16_t aRapidSpeed) 
        :   myNormalSpeed(aNormalSpeed),
            myRapidSpeed(aRapidSpeed) {};
    int16_t myNormalSpeed;
    int16_t myRapidSpeed;

    // Copy constructor
    UpdateSpeedEventData(const UpdateSpeedEventData& other) 
        :   myNormalSpeed(other.myNormalSpeed),
            myRapidSpeed(other.myRapidSpeed) {};

    // Copy assignment operator
    UpdateSpeedEventData& operator=(const UpdateSpeedEventData& other) {
        if (this != &other) {
            myNormalSpeed = other.myNormalSpeed;
            myRapidSpeed = other.myRapidSpeed;
        }
        return *this;
    }

    // Destructor
    ~UpdateSpeedEventData() override {}
};

class StateMachine {
public:
  StateMachine(std::shared_ptr<Stepper> aStepper);
  void Start();

  State GetState() { return currentState; }
  //RingbufHandle_t GetEventRingBuf() { return myEventLoop; }
  esp_event_loop_handle_t GetEventLoop() { return myEventLoop; }

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
    SpeedState currentSpeedState; //TODO probably don't need this, if we can update using debouncer.Changed()
	std::shared_ptr<Stepper> myStepper;
    TaskHandle_t myProcessQueueTaskHandle;
	esp_event_loop_handle_t myEventLoop;
	TaskHandle_t myEventLoopTaskHandle;
	StateMachine* myRef;
	//esp_event_loop_handle_t myUpdateSpeedEventLoop;
};

#endif // STATE_H
