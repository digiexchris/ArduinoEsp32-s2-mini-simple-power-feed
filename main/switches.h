#ifndef SWITCHES_H
#define SWITCHES_H

#include <Bounce2.h>
#include <memory>
#include "state.h"

enum class SwitchState {
    Pressed,
    Released
};

class Switches {
public:
    Switches(
        std::shared_ptr<StateMachine> aStateMachine, 
        int leftPin, 
        int rightPin, 
        int rapidPin
    );

    void ProcessLeftSwitchState();
    SwitchState GetLeftSwitchState();
    void ProcessRightSwitchState();
    SwitchState GetRightSwitchState();
    void ProcessRapidSwitchState();
    SwitchState GetRapidSwitchState();
    void UpdateSwitches();
    static void UpdateTask(void* params);
private:
    int myLeftPin;
    int myRightPin;
    int myRapidPin;
    SwitchState myLeftSwitchState;
    SwitchState myRightSwitchState;
    SwitchState myRapidSwitchState;

    TaskHandle_t switchesTaskHandle;

    std::shared_ptr<StateMachine> myStateMachine;

    Bounce2::Button leftDebouncer = Bounce2::Button();
    Bounce2::Button rightDebouncer = Bounce2::Button();
    Bounce2::Button rapidButtonDebouncer = Bounce2::Button();
};

#endif // SWITCHES_H
