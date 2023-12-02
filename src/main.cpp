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

volatile bool isLeft = false;
volatile bool isRight = false;
volatile bool isRapid = false;

int speed = 0;
int rapidSpeed = 0;

// int32_t leftStop = INT32_MIN;
// int32_t rightStop = INT32_MAX;
bool isLeftStopSet = false;
bool isRightStopSet = false;

bool requiresStop = false;

enum Direction {
  LEFT,
  RIGHT
};

Direction currentDirection = Direction::LEFT;

enum State {
  STOPPED,
  MOVING,
  RAPID,
  STOPPING
};

State currentState = State::STOPPED;

std::shared_ptr<Stepper> stepper;

// Create Bounce objects for debouncing interrupts
Bounce2::Button leftDebouncer = Bounce2::Button();
Bounce2::Button rightDebouncer = Bounce2::Button();
Bounce2::Button rapidButtonDebouncer = Bounce2::Button();
Bounce2::Button leftStopButtonDebouncer = Bounce2::Button();
Bounce2::Button rightStopButtonDebouncer = Bounce2::Button();

void setup() {
  Serial.begin(9600);

  stepper = std::make_shared<Stepper>(dirPinStepper, enablePinStepper, stepPinStepper);
  
  pinMode(leftPin, INPUT_PULLDOWN);
  pinMode(rightPin, INPUT_PULLDOWN);
  pinMode(rapidPin, INPUT_PULLDOWN);
  pinMode(stopLeftPin, INPUT_PULLDOWN);
  pinMode(stopRightPin, INPUT_PULLDOWN);

//Bounce2::Button leftButtonDebouncer = Bounce2::Button();
// Bounce2::Button rightButtonDebouncer = Bounce2::Button();
// Bounce2::Button rapidButtonDebouncer = Bounce2::Button();
// Bounce2::Button leftStopButtonDebouncer = Bounce2::Button();
// Bounce2::Button rightStopButtonDebouncer = Bounce2::Button();
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


  // attachInterrupt(digitalPinToInterrupt(stopLeftPin), leftStopInterrupt, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(stopRightPin), rightStopInterrupt, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(leftPin), leftInterrupt, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(rightPin), rightInterrupt, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(rapidPin), rapidInterrupt, CHANGE);
  Serial.println("Setup complete");
}

void loop() {
  
  //Update all switches
  leftDebouncer.update();
  rightDebouncer.update();
  rapidButtonDebouncer.update();
  leftStopButtonDebouncer.update();
  rightStopButtonDebouncer.update();

  speed = map(analogRead(speedPin), 0, 1023, 0, maxDriverFreq);
  rapidSpeed = 1023;//map(analogRead(maxSpeedPin), 0, 1023, 0, maxDriverFreq);
  //Serial.println("Speed: " + String(speed) + " Rapid: " + String(rapidSpeed));

  if (stepper) {

    //if we're changing direction, or if a direction was unset, a stop is required.
    //The Align disconnects the clutch at the same time, so due to deceleration, the position will get lost
    //TODO add a pendant with move to stop buttons that cause the isLeft and isRight to be ignored.
    //TODO this basically means the stops don't work properly and lose position when we stop.
    //perhaps the workflow is to leave the handle engaged, use a jog button or encoder on the pendant to move to the stop
    //using the non rapid speed, then hit a button to set the corresponding stop.
    //then we need a moveToLeft and moveToRight buttons.
    //the handle probably should engage a runContinuous mode instead of moveTo(stop).
    if(stepper->isRunning() && !leftDebouncer.isPressed() && !rightDebouncer.isPressed()) {
      Serial.println("Requires stop");
      currentState = STOPPING;
      stepper->stopMove();
      return;
    }

    //let it come to a complete stop before changing direction, but if the new direction is the same as the current direction,
    //we can cancel the stop and start the move again
    if(stepper->isStopping()) {
      Serial.println("Stopping");
      if(currentDirection == LEFT && leftDebouncer.isPressed() && currentState != MOVING) {
        currentState = MOVING;
        stepper->moveTo(leftStop);
        // delay(10);
      } else if(currentDirection == RIGHT && rightDebouncer.isPressed()  && currentState != MOVING) {
        currentState = MOVING;
        stepper->moveTo(rightStop);
        // delay(10);
      } else {
        return;
      }
    }
    
    //is stopped basically, but there are a few other cases like Running Continuously
    //that should still be handled by isRunning
    if(!stepper->isRunning() && !stepper->isStopping()) {
      if(leftDebouncer.isPressed() && currentState != MOVING) {
        Serial.println("Moving left");
        currentState = MOVING;
        stepper->moveTo(leftStop);
      } else if(rightDebouncer.isPressed() && currentState != MOVING) {
        Serial.println("Moving right");
        currentState = MOVING;
        stepper->moveTo(rightStop);
      } else {
        // if(stepper->currentState != State::STOPPED) {
        //   Serial.println("Stopped");
        //   currentState = State::STOPPED;
        // }
      }
    }

    //cranked the speed pot to zero. FastAccelStepper ignores zero speed, so stop instead.
    //the case above should restart it when we turn the knob back up again.
    if(speed <= 50 && stepper->isRunning()) {
      Serial.println("Speed is zero");
      stepper->stopMove();
      currentState = STOPPING;
      return;
    }

    //finally, if we're moving, update the speeds
    if(stepper->isRunning() && rapidButtonDebouncer.isPressed()) {
      if(currentState != RAPID)
      {
        Serial.println("Rapid speed");
      currentState = RAPID;
      updateSpeed(rapidSpeed);
      }
    } else if (stepper->isRunning() && !rapidButtonDebouncer.isPressed()) {
      if(currentState != MOVING)
      {
        Serial.println("Normal speed");
        currentState = MOVING;
        updateSpeed(speed);
      } else {
        updateSpeed(speed);
      }
    }
  }
}