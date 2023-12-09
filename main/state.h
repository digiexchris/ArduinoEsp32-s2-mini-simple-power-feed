
#ifndef STATE_H
#define STATE_H

#include <memory>
#include "stepper.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "shared.h"

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

enum class Event {
    LeftPressed,
    LeftReleased,
    RightPressed,
    RightReleased,
    RapidPressed,
    RapidReleased,
    UpdateSpeed
};

class EventData {

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
    ~UpdateSpeedEventData() {}
};

class StateMachine {
public:
  StateMachine(std::shared_ptr<Stepper> aStepper);
  void Start();

  State GetState() { return currentState; }
  RingbufHandle_t GetEventRingBuf() { return myEventRingBuf; }

  RingbufHandle_t GetUpdateSpeedQueue() { return myUpdateSpeedRingbuf; }

private:
    void MoveLeftAction();
    void MoveRightAction();
    void RapidSpeedAction();
    void NormalSpeedAction();
    void StopLeftAction();
    void StopRightAction();

    static void ProcessEventQueueTask(void* params);
	static void ProcessUpdateSpeedQueueTask(void *params);
    bool ProcessEvent(Event event);

    State currentState;
    SpeedState currentSpeedState; //TODO probably don't need this, if we can update using debouncer.Changed()
	std::shared_ptr<Stepper> myStepper;
    TaskHandle_t myProcessQueueTaskHandle;
	RingbufHandle_t myEventRingBuf;
	RingbufHandle_t myUpdateSpeedRingbuf;
};

#endif // STATE_H
