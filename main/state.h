
#ifndef STATE_H
#define STATE_H

#include <memory>
#include "stepper.h"

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
};

class StateMachine {
public:
    StateMachine(int dirPin, int enablePin, int stepPin, uint16_t rapidSpeed);
    bool AddEvent(Event event);
    bool AddUpdateSpeedEvent(UpdateSpeedEventData* data);

private:
    void MoveLeftAction();
    void MoveRightAction();
    void RapidSpeedAction();
    void NormalSpeedAction();
    void UpdateSpeedAction(UpdateSpeedEventData data);
    void StopLeftAction();
    void StopRightAction();

    static void ProcessEventQueueTask(void* params);
    static void ProcessUpdateSpeedQueueTask(void* params);
    void processEvent(Event event);
    void processSpeedEvent(UpdateSpeedEventData data);

    State currentState;
    SpeedState currentSpeedState; //TODO probably don't need this, if we can update using debouncer.Changed()
    Stepper* myStepper;
    TaskHandle_t myProcessQueueTaskHandle;
    QueueHandle_t myEventQueue;
    QueueHandle_t myUpdateSpeedQueue;
};

#endif // STATE_H
