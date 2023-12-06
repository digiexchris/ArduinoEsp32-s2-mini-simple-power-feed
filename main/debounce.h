#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "driver/gpio.h"

class Debouncer {
public:
    Debouncer(std::shared_ptr<StateMachine> stateMachine,  gpio_num_t switchPin, uint16_t aDelay, uint16_t aPollIntervalMs = 50) 
    : mySwitchPin(switchPin),
      myDelay(aDelay),
      myPollIntervalMs(aPollIntervalMs) {
        gpio_pad_select_gpio(switchPin);
        gpio_set_direction(switchPin, GPIO_MODE_INPUT);
        gpio_set_pull_mode(switchPin, GPIO_PULLDOWN_ONLY);
        myLastSwitchState = gpio_get_level(mySwitchPin);
    }

    void setSwitchPressedEvent(Event anEvent) {
        mySwitchPressedEvent = anEvent;
    }

    void setSwitchReleasedEvent(Event anEvent) {
        mySwitchReleasedEvent = anEvent;
    }

    void start() {
        xTaskCreate(switchDebouncerTask, "switch_debouncer_task", 2048, this, 10, NULL);
    }

private:
    std::shared_ptr<StateMachine> myStateMachine;
    gpio_num_t mySwitchPin;
    uint16_t myDelay;
    uint16_t myPollIntervalMs;
    Event mySwitchPressedEvent;
    Event mySwitchReleasedEvent;
    TickType_t myLastStateChangeTime = 0;
    bool myLastSwitchState;
    

    void mySwitchPressedCallback() {
        ESP_LOGI("debounce.h", "Switch pressed");
        myStateMachine->AddEvent(mySwitchPressedEvent);
    }

    void mySwitchReleasedCallback() {
        ESP_LOGI("debounce.h", "Switch released");
        myStateMachine->AddEvent(mySwitchReleasedEvent);
    }

    static void switchDebouncerTask(void* parameter) {
        Debouncer* debouncer = static_cast<Debouncer*>(parameter);

        while (true) {
            bool currentSwitchState = gpio_get_level(debouncer->mySwitchPin);

            if (currentSwitchState != debouncer->myLastSwitchState) {
                debouncer->myLastSwitchState = xTaskGetTickCount();
            }

            if (currentSwitchState != debouncer->myLastSwitchState && (xTaskGetTickCount() - debouncer->myLastSwitchState) >= pdMS_TO_TICKS(debouncer->myDelay)) {
                if (currentSwitchState == 0) {
                    debouncer->mySwitchPressedCallback();
                } else {
                    debouncer->mySwitchReleasedCallback();
                }
            }

            debouncer->myLastSwitchState = currentSwitchState;
            vTaskDelay(pdMS_TO_TICKS(debouncer->myPollIntervalMs));
        }
    }
};
