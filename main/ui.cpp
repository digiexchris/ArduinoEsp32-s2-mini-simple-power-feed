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

ESP_EVENT_DEFINE_BASE(UI_QUEUE_EVENT);

UI::UI(gpio_num_t sdaPin, 
	   gpio_num_t sclPin, 
	   i2c_port_t i2cPort, 
	   uint i2cClkFreq, 
	   gpio_num_t anEncAPin, 
	   gpio_num_t aEncBPin, 
	   gpio_num_t aButtonPin)
{
	
	mySettings = LoadSettings();
	myButtonPin = aButtonPin;
	myUIState = UIState::Stopped;
	myScreen = std::make_unique<Screen>(sdaPin, sclPin, i2cPort, i2cClkFreq, mySettings);
	myRef = this;
	esp_event_loop_args_t loopArgs = {
		.queue_size = 16,
		.task_name = "UIEventLoop", // task will be created
		.task_priority = uxTaskPriorityGet(NULL),
		.task_stack_size = 3072*2,
		.task_core_id = tskNO_AFFINITY};
	
	myUIEventLoop = std::make_shared<esp_event_loop_handle_t>();
	ESP_ERROR_CHECK(esp_event_loop_create(&loopArgs, myUIEventLoop.get()));


	auto led = configureLed(RGB_LED_PIN);

	// Clear LED strip (turn off all LEDs)
	ESP_ERROR_CHECK(led_strip_clear(led));
	// Flush RGB values to LEDs
	ESP_ERROR_CHECK(led_strip_refresh(led));

	gpio_pad_select_gpio(myButtonPin);
	gpio_set_direction(myButtonPin, GPIO_MODE_INPUT);
	gpio_set_pull_mode(myButtonPin, GPIO_PULLUP_ONLY);
	
	xTaskCreatePinnedToCore(ToggleUnitsButtonTask, "ToggleUnitsButtonTask", 2048*2, this, 1, nullptr, 0);

	const esp_timer_create_args_t timer_args = {
		.callback = &CheckAndSaveSettingsCallback,
		/* argument specified here will be passed to timer callback function */
		.arg = this,
		.name = "SaveSettings60S"};

	esp_timer_create(&timer_args, &myTimer);
	esp_timer_start_periodic(myTimer, 60000000);
	
	ESP_LOGI("UI", "UI constructor complete");
}

void UI::ProcessUIEventLoopTask(void *pvParameters)
{
	UI *ui = (UI *)pvParameters;
	while (true)
	{
		esp_event_loop_run(*ui->myUIEventLoop, 500);
		vTaskDelay(50 * portTICK_PERIOD_MS);
	}
}

std::shared_ptr<esp_event_loop_handle_t> UI::GetUiEventLoop() {
	return myUIEventLoop;
}

void UI::ProcessUIEventLoopIteration(void *aUi, esp_event_base_t base, int32_t id, void *payload)
{
	UI *ui = (UI *)aUi;

	ASSERT_MSG(ui, "ProcessUIEventLoopIteration", "UI was null on start of task");

	UIEvent event = static_cast<UIEvent>(id);

	UIEventData *eventData = static_cast<UIEventData*>(payload);

	ui->ProcessUIEvent(event, eventData);
}

void UI::ProcessUIEvent(UIEvent aEvent, UIEventData *aEventData)
{
	switch (aEvent)
	{
	case UIEvent::SetSpeed:
		myScreen->SetSpeed(aEventData->mySpeed);
		break;
	case UIEvent::MoveLeft:
		myScreen->SetState(UIState::MovingLeft);
		break;
	case UIEvent::MoveRight:
		myScreen->SetState(UIState::MovingRight);
		break;
	case UIEvent::RapidSpeed:
		myScreen->SetSpeedState(SpeedState::Rapid);
		break;
	case UIEvent::NormalSpeed:
		myScreen->SetSpeedState(SpeedState::Normal);
		break;
	case UIEvent::Stopping:
		myScreen->SetState(UIState::Stopping);
		break;
	case UIEvent::Stopped:
		myScreen->SetState(UIState::Stopped);
		break;
	case UIEvent::ToggleUnits:
		ToggleUnits();
		break;
	case UIEvent::SetEncoderOffset:
		mySettings->myEncoderOffset = ((UISetEncoderOffsetEventData*)aEventData)->myEncoderOffset;
		break;
	}
}

void UI::Start()
{
	
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(*myUIEventLoop, UI_QUEUE_EVENT, ESP_EVENT_ANY_ID, ProcessUIEventLoopIteration, myRef, nullptr));
	BaseType_t result = xTaskCreatePinnedToCore(ProcessUIEventLoopTask, "UIQueueUpdate", 2048*2, myRef, 1, NULL, 1);
	ASSERT_MSG(result == pdPASS, "Could not start UI event loop task, error: %d", result);
	
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
                    ESP_ERROR_CHECK( esp_event_post_to(*myUIEventLoop, UI_QUEUE_EVENT, static_cast<int32_t>(UIEvent::ToggleUnits), nullptr, sizeof(nullptr), portMAX_DELAY));
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

std::shared_ptr<UI::Settings> UI::LoadSettings()
{
	nvs_handle_t nvs_handle;
	esp_err_t err;

	std::shared_ptr<Settings> settings = std::make_shared<Settings>();

	// Initialize NVS
	err = nvs_flash_init();
	ASSERT_MSG(err == ESP_OK, "LoadSettings: nvs_flash_init", err);

	// Open NVS handle
	err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle);
	ASSERT_MSG(err == ESP_OK, "LoadSettings: nvs_open", err);

	// Read the offset value
	err = nvs_get_i32(nvs_handle, STORAGE_ENCODER_OFFSET_KEY, &settings->myEncoderOffset);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
	{
		if (err == ESP_ERR_NVS_NOT_FOUND)
		{
			settings->myEncoderOffset = 0;
		}
		else
		{
			nvs_close(nvs_handle);
			ASSERT_MSG(false, "SaveSettings: nvs_get_i32", err);
			return settings;
		}
	}

	// Read the offset value
	err = nvs_get_u8(nvs_handle, STORAGE_UI_UNITS_KEY, (uint8_t*)&settings->myUnits);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
	{
		if (err == ESP_ERR_NVS_NOT_FOUND)
		{
			settings->myUnits = SpeedUnit::MMPM;
		}
		else
		{
			nvs_close(nvs_handle);
			ASSERT_MSG(false, "SaveSettings: nvs_set_i8", err);
			return settings;
		}
	}

	// Close NVS handle
	nvs_close(nvs_handle);

	return settings;
}

void UI::SaveSettings(std::shared_ptr<UI::Settings> settings)
{
	nvs_handle_t nvs_handle;
	esp_err_t err;


	// Initialize NVS
	err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);
	ASSERT_MSG(err == ESP_OK, "SaveSettings: nvs_flash_init", err);

	// Open NVS handle
	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
	ASSERT_MSG(err == ESP_OK, "SaveSettings: nvs_open", err);

	// Write the offset value
	err = nvs_set_i32(nvs_handle, STORAGE_ENCODER_OFFSET_KEY, settings->myEncoderOffset);
	if (err != ESP_OK)
	{
		nvs_close(nvs_handle);
		ASSERT_MSG(false, "SaveSettings: nvs_set_i32", err);
		return;
	}

	err = nvs_set_u8(nvs_handle, STORAGE_ENCODER_OFFSET_KEY, (uint8_t)settings->myUnits);
	if (err != ESP_OK)
	{
		nvs_close(nvs_handle);
		ASSERT_MSG(false, "SaveSettings: nvs_set_i8", err);
		return;
	}
	// Commit changes
	err = nvs_commit(nvs_handle);
	if (err != ESP_OK)
	{
		nvs_close(nvs_handle);
		ASSERT_MSG(false, "SaveSettings: nvs_commit", err);
		return;
	}

	// Close NVS handle
	nvs_close(nvs_handle);

	return;
}

void UI::CheckAndSaveSettingsCallback(void *param)
{
	bool changed = false;
	UI *ui = (UI *)param;
	int32_t offset = ui->mySettings->myEncoderOffset;
	if (ui->myPrevSettings->myEncoderOffset != offset)
	{
		ui->myPrevSettings->myEncoderOffset = offset;
		changed = true;
	}

	SpeedUnit units = ui->mySettings->myUnits;
	if (ui->myPrevSettings->myUnits != units)
	{
		ui->myPrevSettings->myUnits = units;
		changed = true;
	}

	if (changed)
	{
		ui->SaveSettings(ui->mySettings);
	}
}

void UI::ToggleUnits() 
{
	if (mySettings->myUnits == SpeedUnit::MMPM)
	{
		mySettings->myUnits = SpeedUnit::IPM;
	}
	else
	{
		mySettings->myUnits = SpeedUnit::MMPM;
	}

	myScreen->SetUnit(mySettings->myUnits);
}