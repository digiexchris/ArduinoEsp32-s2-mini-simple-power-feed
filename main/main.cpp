//Currently written for the esp32-s2 mini dev kit available on aliexpress, the one based on the esp32-mini board with two rows of pins on each side
//a 5v level shifter is recommended for the stepper driver inputs, but I have not had any issues without one
//the stepper driver is a TB6600, but any driver that accepts step/dir inputs should work
//the stepper is a 60BYGH Nema23, but any stepper should work.
//prioritize rpm over torque, since the align power feed has a relatively high gear ratio
//if you reuse most of the clutch mechanism
#include <Arduino.h>
#include <Bounce2.h> // Include the Bounce2 library for debounce

#include "FastAccelStepper.h"
#include <memory>
#include "stepper.h"
#include "state.h"
#include <esp_log.h>

#define dirPinStepper 4
#define enablePinStepper 5
#define stepPinStepper 6
#define speedPin 7 //front knob pot
//#define maxSpeedPin 7 //trimpot to select max speed
#define leftPin 35
#define rightPin 38
#define rapidPin 36
#define stopLeftPin 8
#define stopRightPin 17

//TODO add stop positions to oled display

const int maxDriverFreq = 20000000; // 20kHz max pulse freq in millihz at 25/70 duty cycle, 13kHz at 50/50
const int maxRpm = 160; // 160 rpm max as per Align power feed
const int stepsPerRev = 200;

int speed = 0;
int rapidSpeed = maxDriverFreq;

Stepper* stepper;
StateMachine* myState;

// Create Bounce objects for debouncing interrupts
Bounce2::Button leftDebouncer = Bounce2::Button();
Bounce2::Button rightDebouncer = Bounce2::Button();
Bounce2::Button rapidButtonDebouncer = Bounce2::Button();
Bounce2::Button leftStopButtonDebouncer = Bounce2::Button();
Bounce2::Button rightStopButtonDebouncer = Bounce2::Button();

void setup() {
  Serial.begin(9600);
  int size = uxTaskGetStackHighWaterMark(NULL);
  ESP_LOGI("Setup", "Stack high water mark: %d\n", size);  
  stepper = new Stepper();
  size = uxTaskGetStackHighWaterMark(NULL);
  ESP_LOGI("Setup", "Stack high water mark: %d\n", size); 
  stepper->Init(dirPinStepper, enablePinStepper, stepPinStepper, rapidSpeed);
  size = uxTaskGetStackHighWaterMark(NULL);
  ESP_LOGI("Setup", "Stack high water mark: %d\n", size);
  pinMode(rightPin, INPUT_PULLDOWN);
  pinMode(rapidPin, INPUT_PULLDOWN);
  pinMode(stopLeftPin, INPUT_PULLDOWN);
  pinMode(stopRightPin, INPUT_PULLDOWN);

  leftDebouncer.attach( leftPin, INPUT_PULLDOWN );
  leftDebouncer.interval(5);
  leftDebouncer.setPressedState(HIGH);
  rightDebouncer.attach( rightPin, INPUT_PULLDOWN );
  rightDebouncer.interval(5);
  rightDebouncer.setPressedState(HIGH);
  rapidButtonDebouncer.attach( rapidPin, INPUT_PULLDOWN );
  rapidButtonDebouncer.interval(5);
  rapidButtonDebouncer.setPressedState(HIGH);
  leftStopButtonDebouncer.attach( stopLeftPin, INPUT_PULLDOWN );
  leftStopButtonDebouncer.interval(5);
  leftStopButtonDebouncer.setPressedState(HIGH);
  rightStopButtonDebouncer.attach( stopRightPin, INPUT_PULLDOWN );
  rightStopButtonDebouncer.interval(5);
  rightStopButtonDebouncer.setPressedState(HIGH);

  myState = new StateMachine(stepper);
  Serial.println("Setup complete");
}

void UpdateTask(void * pvParameters) {
  while(true) {
    int size = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI("Setup", "Stack high water mark: %d\n", size);
    //Update all switches
    leftDebouncer.update();
    rightDebouncer.update();
    rapidButtonDebouncer.update();
    leftStopButtonDebouncer.update();
    rightStopButtonDebouncer.update();

    speed = map(analogRead(speedPin), 0, 1023, 0, maxDriverFreq);

    //if the new speed is within .5% of the current speed, don't bother updating it
    if(stepper->GetNormalSpeed() <= speed + (speed*0.05) && stepper->GetNormalSpeed() >= speed - (speed*0.05)) {
      //noop
    } else {
      // Assuming processEvent takes a unique_ptr
      myState->processEvent(Event::UpdateSpeed, std::make_unique<UpdateSpeedEventData>(speed, rapidSpeed));
    }

    if(leftDebouncer.rose()) {
      myState->processEvent(Event::LeftPressed);
    }

    if(leftDebouncer.fell()) {
      myState->processEvent(Event::LeftReleased);
    }

    if(rightDebouncer.rose()) {
      myState->processEvent(Event::RightPressed);
    }

    if(rightDebouncer.fell()) {
      myState->processEvent(Event::RightReleased);
    }

    if(rapidButtonDebouncer.rose()) {
      myState->processEvent(Event::RapidPressed);
    }

    if(rapidButtonDebouncer.fell()) {
      myState->processEvent(Event::RapidReleased);
    }
    
    //30fps :D
    vTaskDelay(33.33 / portTICK_PERIOD_MS);
  }
}

TaskHandle_t loopTaskHandle = NULL;

extern "C" void app_main()
{
  setup();

//xTaskCreateUniversal(loopTask, "loopTask", getArduinoLoopTaskStackSize(), NULL, 1, &loopTaskHandle, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(&UpdateTask,"main loop", 8192*4, NULL, 1, &loopTaskHandle, 0);

}