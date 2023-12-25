#include "switches.h"
#include "shared.h"
#include "state.h"
#include "esp_event.h"
#include <memory>

std::vector<std::shared_ptr<Switch>> Debouncer::mySwitches;

Switch::Switch(gpio_num_t aSwitchPin, uint16_t aDelay, Event aPressedEvent, Event aReleasedEvent)
{
	mySwitchPin = aSwitchPin;
	myDelay = aDelay;
	mySwitchPressedEvent = aPressedEvent;
	mySwitchReleasedEvent = aReleasedEvent;
	myLastStateChangeTime = 0;
	myPullMode = GPIO_PULLDOWN_ONLY;
	myIntrType = GPIO_INTR_ANYEDGE;
	myMode = GPIO_MODE_INPUT;
	callbackCalled = true;
	myLastSwitchState = false;
	myHasPendingStateChange = false;
}

void Debouncer::Create(std::shared_ptr <esp_event_loop_handle_t> anEventLoop)
{
	myEventLoop = anEventLoop;
}

void Debouncer::AddSwitch(SwitchName aName, std::shared_ptr<Switch>aSwitch)
{
	mySwitches.push_back(aSwitch);
	gpio_pad_select_gpio(aSwitch->mySwitchPin);
	gpio_set_direction(aSwitch->mySwitchPin, aSwitch->myMode);
	gpio_set_pull_mode(aSwitch->mySwitchPin, aSwitch->myPullMode);
	//gpio_set_intr_type(aSwitch->mySwitchPin, aSwitch->myIntrType);
	aSwitch->myLastSwitchState = gpio_get_level(aSwitch->mySwitchPin);
}

void Debouncer::Start()
{
	// for (std::shared_ptr<Switch> aSwitch : mySwitches)
	// {
	// 	gpio_isr_handler_add(aSwitch->mySwitchPin, DebounceHandler, aSwitch.get());
	// }
	xTaskCreatePinnedToCore(DebounceTask, "DebounceTask", 2048, nullptr, 10, nullptr, 0);
}

void IRAM_ATTR Debouncer::DebounceHandler(void *arg)
{
	Switch *aSwitch = static_cast<Switch *>(arg);
	aSwitch->myLastStateChangeTime = xTaskGetTickCountFromISR();
	aSwitch->myHasPendingStateChange = true;
}

void Debouncer::DebounceTask(void *arg)
{
    while (true)
    {
        for (auto &aSwitch : Debouncer::mySwitches)
        {
            bool currentLevel = gpio_get_level(aSwitch->mySwitchPin);

            // Check if switch state has changed
            if (currentLevel != aSwitch->myLastSwitchState)
            {
                // Reset the last change time if state has changed
                aSwitch->myLastStateChangeTime = xTaskGetTickCount();
                aSwitch->myLastSwitchState = currentLevel;
				aSwitch->myHasPendingStateChange = true;
            }
            else if ((xTaskGetTickCount() - aSwitch->myLastStateChangeTime) >= pdMS_TO_TICKS(aSwitch->myDelay))
            {
                // If state is stable for the debounce period, process the state change
                if (aSwitch->myHasPendingStateChange)
                {
                    aSwitch->myHasPendingStateChange = false;
                    Event event = currentLevel ? aSwitch->mySwitchPressedEvent : aSwitch->mySwitchReleasedEvent;

                    ESP_ERROR_CHECK(PublishEvent(STATE_MACHINE_EVENT, static_cast<int32_t>(event), nullptr);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Polling interval, adjust as needed
    }
}
