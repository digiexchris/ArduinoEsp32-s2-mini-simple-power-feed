#include "switches.h"
#include <memory>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "state.h"

#error THIS IS BROKEN without bounce2, kill bounce2 and any arduino dependency.
https://github.com/craftmetrics/esp32-button

Switches::Switches(
    std::shared_ptr<StateMachine> aStateMachine, 
    int leftPin, 
    int rightPin, 
    int rapidPin) 
    : myLeftPin(leftPin), 
    myRightPin(rightPin), 
    myRapidPin(rapidPin),
    myStateMachine(aStateMachine)
{
    pinMode(leftPin, INPUT_PULLUP);
    pinMode(rightPin, INPUT_PULLUP);
    pinMode(rapidPin, INPUT_PULLUP);
    xTaskCreate(&UpdateTask, "UpdateTask", 2048, this, 1, &switchesTaskHandle);
};

void Switches::UpdateTask(void* params) {
    Switches* switches = (Switches*) params;
    while (true) {
        switches->UpdateSwitches();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
};

void Switches::UpdateSwitches() {
    leftDebouncer.update();
    rightDebouncer.update();
    rapidButtonDebouncer.update();

};


void Switches::ProcessLeftSwitchState() {
    if(leftDebouncer.changed()) {
        if (leftDebouncer.fell()) {
            myLeftSwitchState = SwitchState::Pressed;
            myStateMachine->AddEvent(Event::LeftPressed);
        }
        else if (leftDebouncer.rose()) {
            myLeftSwitchState = SwitchState::Released;
            myStateMachine->AddEvent(Event::LeftReleased);
        }
    }
};

SwitchState Switches::GetLeftSwitchState() {
    return myLeftSwitchState;
};

void Switches::ProcessRightSwitchState() {
    if(rightDebouncer.changed()) {
        if (rightDebouncer.fell()) {
            myRightSwitchState = SwitchState::Pressed;
            myStateMachine->AddEvent(Event::RightPressed);
        }
        else if (rightDebouncer.rose()) {
            myRightSwitchState = SwitchState::Released;
            myStateMachine->AddEvent(Event::RightReleased);
        }
    }
};

SwitchState Switches::GetRightSwitchState() {
    return myRightSwitchState;
};

void Switches::ProcessRapidSwitchState() {
    if(rapidButtonDebouncer.changed()) {
        if (rapidButtonDebouncer.fell()) {
            myRapidSwitchState = SwitchState::Pressed;
            myStateMachine->AddEvent(Event::RapidPressed);
        }
        else if (rapidButtonDebouncer.rose()) {
            myRapidSwitchState = SwitchState::Released;
            myStateMachine->AddEvent(Event::RapidReleased);
        }
    }
};

SwitchState Switches::GetRapidSwitchState() {
    return myRapidSwitchState;
};