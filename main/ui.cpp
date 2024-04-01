#include "ui.h"
#include <chrono>
#include "config.h"
#include "state.h"
#include "icons.h"
#include <sys/time.h>
#include <esp_event.h>
#include <memory>
#include <shared.h>
#include "EventTypes.h"
#include <led_strip.h>
#include "nvs_flash.h"
#include "nvs.h"

std::shared_ptr<UI> UI::myRef = nullptr;

UI::UI(gpio_num_t sdaPin, 
	   gpio_num_t sclPin, 
	   i2c_port_t i2cPort, 
	   uint i2cClkFreq, 
	   gpio_num_t anEncAPin, 
	   gpio_num_t aEncBPin, 
	   gpio_num_t aButtonPin,
	   int32_t aSavedNormalSpeed,
	   int32_t aSavedRapidSpeed,
	   SpeedUnit aSavedSpeedUnits)
{
	myRapidSpeed = aSavedRapidSpeed;
	mySpeedUnits = aSavedSpeedUnits;
	myNormalSpeed = aSavedNormalSpeed;
	
	myButtonPin = aButtonPin;
	myUIState = UIState::Stopped;
	myScreen = std::make_unique<Screen>(sdaPin, sclPin, i2cPort, i2cClkFreq);
	myRef.reset(this);
	myIsRapid = false;

	gpio_pad_select_gpio(myButtonPin);
	gpio_set_direction(myButtonPin, GPIO_MODE_INPUT);
	gpio_set_pull_mode(myButtonPin, GPIO_PULLUP_ONLY);
	
	xTaskCreatePinnedToCore(ToggleUnitsButtonTask, "ToggleUnitsButtonTask", 2048*2, this, 1, nullptr, 0);

	ESP_LOGI("UI", "UI constructor complete");
}

void UI::ProcessEventCallback(void *aUi, esp_event_base_t base, int32_t id, void *payload)
{
	Event event = static_cast<Event>(id);

	EventData *eventData = static_cast<EventData*>(payload);

	myRef->ProcessEvent(event, eventData);
}

void UI::ProcessEvent(Event aEvent, EventData *aEventData)
{
	switch (aEvent)
	{
	case Event::MovingLeft:
		myScreen->SetState(UIState::MovingLeft);
		break;
	case Event::MovingRight:
		myScreen->SetState(UIState::MovingRight);
		break;
	case Event::RapidSpeed:
		myIsRapid = true;
		myScreen->SetSpeedState(SpeedState::Rapid);
		myScreen->SetSpeed(myRapidSpeed);
		break;
	case Event::NormalSpeed:
		myIsRapid = false;
		myScreen->SetSpeedState(SpeedState::Normal);
		myScreen->SetSpeed(myNormalSpeed);
		break;
	case Event::Stopping:
		myScreen->SetState(UIState::Stopping);
		break;
	case Event::Stopped:
		myScreen->SetState(UIState::Stopped);
		break;
	case Event::ToggleUnits:
		ToggleUnits();
		break;
	case Event::UpdateSpeed:
		{
			SingleValueEventData<int32_t> const *evt = (SingleValueEventData<int32_t>*)aEventData;
			int32_t speedDelta = evt->myValue;

			if(myIsRapid)
			{
				myRapidSpeed += speedDelta;
				myScreen->SetSpeed(myRapidSpeed);
			}
			else
			{
				myNormalSpeed += speedDelta;
				myScreen->SetSpeed(myNormalSpeed);
			}

		}
		break;
	default:
		break;
	}
}

void UI::Start()
{
	myScreen->Start();
	
	RegisterEventHandler(STATE_TRANSITION_EVENT, Event::Any, ProcessEventCallback);
	RegisterEventHandler(COMMAND_EVENT, Event::RapidSpeed, ProcessEventCallback);
	RegisterEventHandler(COMMAND_EVENT, Event::NormalSpeed, ProcessEventCallback);
	RegisterEventHandler(UI_EVENT , Event::UpdateSpeed, ProcessEventCallback);
	RegisterEventHandler(UI_EVENT, Event::UpdateSpeed, ProcessEventCallback);
	RegisterEventHandler(COMMAND_EVENT, Event::ToggleUnits, ProcessEventCallback);

	myScreen->SetUnit(mySpeedUnits);
	myScreen->SetSpeed(myNormalSpeed);

	ESP_LOGI("UI", "UI init complete");
}


void UI::ToggleUnitsButtonTask(void *params)
{
	UI *ui = (UI *)params;

	try
	{
		ui->ToggleUnitsButton();
	}
	catch (const std::exception &e)
	{
		ESP_LOGE("UI", "ToggleUnitsButtonTask: %s", e.what());
	}
	catch (...)
	{
		ESP_LOGE("UI", "ToggleUnitsButtonTask: Unknown exception");
	}
	
}
//TODO move the units button to the encoder or switches.
void UI::ToggleUnitsButton()
{
    bool buttonPressed = false;
    auto startTime = std::chrono::steady_clock::now();
	bool buttonPressedEventSent = false;

	while (true)
    {
		if (!gpio_get_level(myButtonPin))
        {
            if (!buttonPressed)
            {
                startTime = std::chrono::steady_clock::now();
                buttonPressed = true;
				buttonPressedEventSent = false;
            }
            else
            {
                auto currentTime = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);

                if (duration.count() >= 1 && !buttonPressedEventSent)
                {
                    ESP_ERROR_CHECK(PublishEvent(COMMAND_EVENT,Event::ToggleUnits));
                    buttonPressedEventSent = true;
                }
            }
        }
        else
        {
            buttonPressed = false;
        }

		vTaskDelay(250 * portTICK_PERIOD_MS);
    }
}

void UI::ToggleUnits() 
{
	if (mySpeedUnits == SpeedUnit::MMPM)
	{
		mySpeedUnits = SpeedUnit::IPM;
	}
	else
	{
		mySpeedUnits = SpeedUnit::MMPM;
	}

	myScreen->SetUnit(mySpeedUnits);

	PublishEvent(SETTINGS_EVENT, Event::SetSpeedUnit, new SingleValueEventData<SpeedUnit>(mySpeedUnits));
}