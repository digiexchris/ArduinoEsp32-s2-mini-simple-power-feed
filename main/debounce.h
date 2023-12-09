#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "rom/gpio.h"
#include "freertos/queue.h"

#include <memory>
#include "state.h"
#include <esp_log.h>

class Debouncer {
public:
  Debouncer(RingbufHandle_t stateRingBuf, gpio_num_t switchPin, uint16_t aDelay, uint16_t aPollIntervalMs = 50)
	  : myStateRingBuf(stateRingBuf),
		mySwitchPin(switchPin),
		myDelay(aDelay),
		myPollIntervalMs(aPollIntervalMs){
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
		xTaskCreate(switchDebouncerTask, "switch_debouncer_task", 2048*18, this, 1, &myTaskHandle);
    }

private:
    RingbufHandle_t myStateRingBuf;
    gpio_num_t mySwitchPin;
    uint16_t myDelay;
    uint16_t myPollIntervalMs;
    Event mySwitchPressedEvent;
    Event mySwitchReleasedEvent;
    TickType_t myLastStateChangeTime = 0;
    bool myLastSwitchState;
	
	TaskHandle_t myTaskHandle;
    

    void mySwitchPressedCallback() {
        ESP_LOGI("debounce.h", "Switch pressed");
		xRingbufferSend(myStateRingBuf, &mySwitchPressedEvent, sizeof(mySwitchPressedEvent), pdMS_TO_TICKS(100));
        //xQueueSend(*myStateQueue, &mySwitchPressedEvent, pdMS_TO_TICKS(100));
    }

    void mySwitchReleasedCallback() {
        ESP_LOGI("debounce.h", "Switch released");
		xRingbufferSend(myStateRingBuf, &mySwitchReleasedEvent, sizeof(mySwitchReleasedEvent), pdMS_TO_TICKS(100));
    }

static void switchDebouncerTask(void* parameter) {
    Debouncer* debouncer = static_cast<Debouncer*>(parameter);

	bool callbackCalled = true; //don't need no stinkin callback on first startup
    while (true) {

		bool currentSwitchState = gpio_get_level(debouncer->mySwitchPin);

		if (currentSwitchState != debouncer->myLastSwitchState)
		{
			debouncer->myLastStateChangeTime = xTaskGetTickCount();
			debouncer->myLastSwitchState = currentSwitchState;
			callbackCalled = false;
		}

		if (!callbackCalled && (xTaskGetTickCount() - debouncer->myLastStateChangeTime) >= pdMS_TO_TICKS(debouncer->myDelay))
		{
			// Call the callback if the current state is stable for at least `myDelay` milliseconds
			if (currentSwitchState == 1)
			{
				debouncer->mySwitchPressedCallback();
			}
			else
			{
				debouncer->mySwitchReleasedCallback();
			}
			callbackCalled = true;
		}
			
        vTaskDelay(pdMS_TO_TICKS(debouncer->myPollIntervalMs));
    }
}


};
