#include "switches.h"
#include "shared.h"

RingbufHandle_t Debouncer::myStateRingBuf;
std::vector<Switch *> Debouncer::mySwitches;

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
}

void Debouncer::Create(RingbufHandle_t stateRingBuf)
{
	myStateRingBuf = stateRingBuf;
}

void Debouncer::AddSwitch(SwitchName aName, Switch *aSwitch)
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
	for (Switch *aSwitch : mySwitches)
	{
		gpio_isr_handler_add(aSwitch->mySwitchPin, DebounceHandler, aSwitch);
	}
}

void IRAM_ATTR Debouncer::DebounceHandler(void *arg)
{
	Switch *aSwitch = static_cast<Switch *>(arg);
//	TickType_t now = xTaskGetTickCountFromISR();

	bool currentSwitchState = gpio_get_level(aSwitch->mySwitchPin);
	if (currentSwitchState != aSwitch->myLastSwitchState)
	{
		//aSwitch->myLastStateChangeTime = now;
		aSwitch->myLastSwitchState = currentSwitchState;
		aSwitch->callbackCalled = false;
	}
	if (!aSwitch->callbackCalled) //&& (xTaskGetTickCount() - aSwitch->myLastStateChangeTime) >= pdMS_TO_TICKS(aSwitch->myDelay))
	{
		// Call the callback if the current state is stable for at least `myDelay` milliseconds
		if (currentSwitchState == 1)
		{
			xRingbufferSendFromISR(myStateRingBuf, &aSwitch->mySwitchPressedEvent, sizeof(aSwitch->mySwitchPressedEvent), nullptr);
		}
		else
		{
			xRingbufferSendFromISR(myStateRingBuf, &aSwitch->mySwitchReleasedEvent, sizeof(aSwitch->mySwitchPressedEvent), nullptr);
		}
		aSwitch->callbackCalled = true;
	}
}