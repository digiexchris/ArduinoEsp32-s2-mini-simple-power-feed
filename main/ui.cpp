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

ESP_EVENT_DEFINE_BASE(UI_QUEUE_EVENT);

UI::UI(gpio_num_t sdaPin, 
	   gpio_num_t sclPin, 
	   i2c_port_t i2cPort, 
	   uint i2cClkFreq, 
	   gpio_num_t anEncAPin, 
	   gpio_num_t aEncBPin, 
	   gpio_num_t aButtonPin)
{
	myUIState = UIState::Stopped;
	myScreen = std::make_unique<Screen>(sdaPin, sclPin, i2cPort, i2cClkFreq);
	myRef = this;
	esp_event_loop_args_t loopArgs = {
		.queue_size = 16,
		.task_name = "UIEventLoop", // task will be created
		.task_priority = uxTaskPriorityGet(NULL),
		.task_stack_size = 3072,
		.task_core_id = tskNO_AFFINITY};
	
	myUIEventLoop = std::make_shared<esp_event_loop_handle_t>();
	ESP_ERROR_CHECK(esp_event_loop_create(&loopArgs, myUIEventLoop.get()));


	/* LED strip initialization with the GPIO and pixels number*/
	led_strip_config_t strip_config = {
		.strip_gpio_num = BLINK_GPIO,			  // The GPIO that connected to the LED strip's data line
		.max_leds = 1,							  // The number of LEDs in the strip,
		.led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
		.led_model = LED_MODEL_WS2812,			  // LED strip model
		.flags.invert_out = false,				  // whether to invert the output signal (useful when your hardware has a level inverter)
	};

	led_strip_rmt_config_t rmt_config = {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
		.rmt_channel = 0,
#else
		.clk_src = RMT_CLK_SRC_DEFAULT,	   // different clock source can lead to different power consumption
		.resolution_hz = 10 * 1000 * 1000, // 10MHz
		.flags.with_dma = false,		   // whether to enable the DMA feature
#endif
	};

	ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &myLed));
	
}

void UI::ProcessUIEventLoopTask(void *pvParameters)
{
	UI *ui = (UI *)pvParameters;
	while (true)
	{
		esp_event_loop_run(*ui->myUIEventLoop, 500);
		vTaskDelay(50);
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

	led_strip_handle_t led_strip;

	/* LED strip initialization with the GPIO and pixels number*/
	led_strip_config_t strip_config = {
		.strip_gpio_num = BLINK_GPIO,			  // The GPIO that connected to the LED strip's data line
		.max_leds = 1,							  // The number of LEDs in the strip,
		.led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
		.led_model = LED_MODEL_WS2812,			  // LED strip model
		.flags.invert_out = false,				  // whether to invert the output signal (useful when your hardware has a level inverter)
	};

	led_strip_rmt_config_t rmt_config = {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
		.rmt_channel = 0,
#else
		.clk_src = RMT_CLK_SRC_DEFAULT,	   // different clock source can lead to different power consumption
		.resolution_hz = 10 * 1000 * 1000, // 10MHz
		.flags.with_dma = false,		   // whether to enable the DMA feature
#endif
	};
	ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
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
	}
}

void UI::Start()
{
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(*myUIEventLoop, UI_QUEUE_EVENT, ESP_EVENT_ANY_ID, ProcessUIEventLoopIteration, myRef, nullptr));
	BaseType_t result = xTaskCreatePinnedToCore(ProcessUIEventLoopTask, "UIQueueUpdate", 2048, myRef, 1, NULL, 0);
	ASSERT_MSG(result == pdPASS, "Could not start UI event loop task, error: %d", result);
	
	myScreen->Start();
}
