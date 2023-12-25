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

UI::UI(gpio_num_t sdaPin, 
	   gpio_num_t sclPin, 
	   i2c_port_t i2cPort, 
	   uint i2cClkFreq, 
	   gpio_num_t anEncAPin, 
	   gpio_num_t aEncBPin, 
	   gpio_num_t aButtonPin,
	   SettingData aSettings)
{
	myButtonPin = aButtonPin;
	myUIState = UIState::Stopped;
	myScreen = std::make_unique<Screen>(sdaPin, sclPin, i2cPort, i2cClkFreq, mySettings);
	myRef = this;

	auto led = configureLed(RGB_LED_PIN);

	// Clear LED strip (turn off all LEDs)
	ESP_ERROR_CHECK(led_strip_clear(led));
	// Flush RGB values to LEDs
	ESP_ERROR_CHECK(led_strip_refresh(led));

	gpio_pad_select_gpio(myButtonPin);
	gpio_set_direction(myButtonPin, GPIO_MODE_INPUT);
	gpio_set_pull_mode(myButtonPin, GPIO_PULLUP_ONLY);
	
	xTaskCreatePinnedToCore(ToggleUnitsButtonTask, "ToggleUnitsButtonTask", 2048*2, this, 1, nullptr, 0);
	mySettings = aSettings;

	myNormalSpeed = mySettings.someoffsetcalc;
	ESP_LOGI("UI", "UI constructor complete");
}

void UI::ProcessEventCallback(void *aUi, esp_event_base_t base, int32_t id, void *payload)
{
	UI *ui = (UI *)aUi;

	ASSERT_MSG(ui, "ProcessEventLoopIteration", "UI was null on start of task");

	Event event = static_cast<Event>(id);

	EventData *eventData = static_cast<EventData*>(payload);

	ui->ProcessEvent(event, eventData);
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
	case Event::UpdateNormalSpeed: 
		{
			UpdateSpeedEventData* evt = (UpdateSpeedEventData*)aEventData;
			myNormalSpeed = evt->mySpeed;
			if (!myIsRapid)
			{
				myScreen->SetSpeed(myNormalSpeed);
			}
		}
		break;
	case Event::UpdateRapidSpeed:
		{
			UpdateSpeedEventData* evt = (UpdateSpeedEventData*)aEventData;
			myRapidSpeed = evt->mySpeed;
			if (myIsRapid)
			{
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

	RegisterEventHandler(STATE_TRANSITION_EVENT, Event::Any, ProcessEventCallback);
	RegisterEventHandler(COMMAND_EVENT, Event::RapidSpeed, ProcessEventCallback);
	RegisterEventHandler(COMMAND_EVENT, Event::NormalSpeed, ProcessEventCallback);
	
	myScreen->Start();
	
	ESP_LOGI("UI", "UI init complete");
}

led_strip_handle_t UI::configureLed(gpio_num_t anLedPin)
{
	// LED strip general initialization, according to your led board design
	led_strip_config_t strip_config = {
		.strip_gpio_num = anLedPin,				  // The GPIO that connected to the LED strip's data line
		.max_leds = 1,							  // The number of LEDs in the strip,
		.led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
		.led_model = LED_MODEL_WS2812,			  // LED strip model
		.flags = {.invert_out = false}			  // whether to invert the output signal
	};

	// LED strip backend configuration: RMT
	led_strip_rmt_config_t rmt_config = {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
		.rmt_channel = 0,
#else
		.clk_src = RMT_CLK_SRC_DEFAULT,		   // different clock source can lead to different power consumption
		.resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
		.flags.with_dma = false,			   // DMA feature is available on ESP target like ESP32-S3
#endif
	};

	// LED Strip object handle
	led_strip_handle_t led_strip;
	ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
	ESP_LOGI("LED", "Created LED strip object with RMT backend");
	return led_strip;
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
                    ESP_ERROR_CHECK( esp_event_post_to(*myEventLoop, UI_QUEUE_EVENT, static_cast<int32_t>(Event::ToggleUnits), nullptr, sizeof(nullptr), portMAX_DELAY));
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
	if (mySettings.mySpeedUnits == SpeedUnit::MMPM)
	{
		mySettings.mySpeedUnits = SpeedUnit::IPM;
	}
	else
	{
		mySettings.mySpeedUnits = SpeedUnit::MMPM;
	}

	myScreen->SetUnit(mySettings.mySpeedUnits);
}