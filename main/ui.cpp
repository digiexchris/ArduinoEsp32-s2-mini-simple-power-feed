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
