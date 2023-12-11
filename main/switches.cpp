#include "switches.h"
#include "shared.h"
#include "state.h"
#include "esp_event.h"

esp_event_loop_handle_t Debouncer::myEventLoop;
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

void Debouncer::Create(esp_event_loop_handle_t anEventLoop)
{
	myEventLoop = anEventLoop;
}

void Debouncer::AddSwitch(SwitchName aName, std::shared_ptr<Switch>aSwitch)
{
	mySwitches.push_back(aSwitch);
	gpio_pad_select_gpio(aSwitch->mySwitchPin);
	gpio_set_direction(aSwitch->mySwitchPin, aSwitch->myMode);
	gpio_set_pull_mode(aSwitch->mySwitchPin, aSwitch->myPullMode);
	gpio_set_intr_type(aSwitch->mySwitchPin, aSwitch->myIntrType);
	aSwitch->myLastSwitchState = gpio_get_level(aSwitch->mySwitchPin);
}

void Debouncer::Start()
{
	for (std::shared_ptr<Switch> aSwitch : mySwitches)
	{
		gpio_isr_handler_add(aSwitch->mySwitchPin, DebounceHandler, aSwitch.get());
	}
	xTaskCreatePinnedToCore(DebounceTask, "DebounceTask", 2048, nullptr, 1, nullptr, 0);
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
			if (aSwitch->myHasPendingStateChange &&
				(xTaskGetTickCount() - aSwitch->myLastStateChangeTime) >= pdMS_TO_TICKS(aSwitch->myDelay))
			{
				// Process the stable state change
				bool currentSwitchState = gpio_get_level(aSwitch->mySwitchPin);
				Event event = currentSwitchState ? aSwitch->mySwitchPressedEvent : aSwitch->mySwitchReleasedEvent;
				//xRingbufferSend(Debouncer::myStateRingBuf, &event, sizeof(event), portMAX_DELAY);

				ESP_ERROR_CHECK(esp_event_post_to(myEventLoop, STATE_MACHINE_EVENT, static_cast<int32_t>(event), nullptr, sizeof(nullptr), portMAX_DELAY));
				aSwitch->myHasPendingStateChange = false;
			}
		}
		vTaskDelay(pdMS_TO_TICKS(10)); // Check every 10 ms, adjust as needed
	}
}