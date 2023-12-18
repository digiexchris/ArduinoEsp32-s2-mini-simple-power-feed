#include "ui.h"
#include <ssd1306.h>
#include <chrono>
#include "config.h"
#include "state.h"
#include "icons.h"
#include <sys/time.h>
#include <esp_event_loop.h>
#include <memory>
#include <shared.h>

Screen::Screen(gpio_num_t sdaPin, gpio_num_t sclPin, i2c_port_t i2cPort, uint i2cClkFreq)
{
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = sdaPin;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = sclPin;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = i2cClkFreq;
	conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

	i2c_param_config(i2cPort, &conf);
	i2c_driver_install(i2cPort, conf.mode, 0, 0, 0);

	ssd1306_dev = ssd1306_create(i2cPort, SSD1306_I2C_ADDRESS);
	ssd1306_refresh_gram(ssd1306_dev);
	ssd1306_clear_screen(ssd1306_dev, 0x00);
	myPrevSpeedUnit = SpeedUnit::MMPM;
	mySpeedUnit = SpeedUnit::MMPM;
	mySpeed = 0;
	myPrevSpeed = 0;
	myState = UIState::Stopped;
	myPrevState = UIState::Stopped;
	mySpeedState = SpeedState::Normal;
	myPrevSpeedState = SpeedState::Normal;
}

void Screen::UpdateTask(void *pvParameters)
{
	Screen *screen = (Screen *)pvParameters;
	while (true)
	{
		screen->Update();
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void Screen::Update()
{
	bool dataChanged = false;
	if (mySpeed != myPrevSpeed)
	{
		myPrevSpeed = mySpeed;
		dataChanged = true;
		DrawSpeed();
		DrawSpeedUnit();
	}

	if (myState != myPrevState)
	{
		myPrevState = myState;
		dataChanged = true;
		
	}
	
	DrawState(); //every time, because it flashes
	
	ssd1306_refresh_gram(ssd1306_dev);
}

void Screen::DrawString(int x, int y, const char *str, int fontHeight, int fontWidth)
{
	ssd1306_draw_string(ssd1306_dev, x, y, (uint8_t*)str, fontHeight, fontWidth);
}

void Screen::DrawState()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	const bool flashHigh = tv.tv_usec < 500000 ? true : false;
	
	char buffer[20];
	switch (myState)
	{
	case UIState::MovingLeft:
		if (mySpeedState == SpeedState::Rapid)
		{
			ssd1306_draw_bitmap(ssd1306_dev, 0, 0, moveleft3232, 32, 32);
		}
		else
		{

			ssd1306_draw_bitmap(ssd1306_dev, 0, 0, rapidleft3232, 32, 32);
		}
		break;
	case UIState::MovingRight:
		if (mySpeedState == SpeedState::Rapid)
		{
			ssd1306_draw_bitmap(ssd1306_dev, 0, 0, moveright3232, 32, 32);
		}
		else
		{
			ssd1306_draw_bitmap(ssd1306_dev, 0, 0, rapidright3232, 32, 32);
		}
	case UIState::Stopping:
		if (flashHigh)
		{
			ssd1306_draw_bitmap(ssd1306_dev, 0, 0, stop3232inverted, 32, 32);
		}
		else
		{
			ssd1306_draw_bitmap(ssd1306_dev, 0, 0, stop3232, 32, 32);
		}
		break;
	case UIState::Stopped:
		ssd1306_draw_bitmap(ssd1306_dev, 0, 0, stop3232, 32, 32);
		break;
	}
}

void Screen::DrawSpeed()
{
	const double speedPerMinute = SpeedPerMinute();
	char buffer[20];
	sprintf(buffer, "%d", speedPerMinute);
	DrawString(128 - 40, 0, buffer, 16, 8);
}

void Screen::SetSpeedState(SpeedState aSpeedState)
{
	mySpeedState = aSpeedState;
}

void Screen::SetSpeed(uint8_t aSpeed)
{
	mySpeed = aSpeed;
}

void Screen::SetUnit(SpeedUnit aUnit)
{
	mySpeedUnit = aUnit;
}

void Screen::DrawSpeedUnit()
{
	switch (mySpeedUnit)
	{
	case SpeedUnit::MMPM:
		DrawString(100, 16, "mm/m", 16, 8);
		break;
	case SpeedUnit::IPM:
		DrawString(100, 16, "ipm", 16, 8);
		break;
	}
}

void Screen::SetState(UIState aState)
{
		myState = aState;
}

double Screen::SpeedPerMinute()
{
	const double mmPerMinute = mySpeed / stepsPerMm * 60;
	
	if(mySpeedUnit == SpeedUnit::MMPM)
	{
		return mmPerMinute;
	}
	else
	{
		return mmPerMinute * 0.0393701;
	}
}

//SpeedEncoder::SpeedEncoder(gpio_num_t anAPin, gpio_num_t aBPin, gpio_num_t aButtonPin)
//{
//	myEncoder = new espp::AbiEncoder<espp::EncoderType::LINEAR>({
//		.a_gpio = anAPin,
//		.b_gpio = aBPin,
//		.high_limit = 4092,
//		.low_limit = 0,
//		.counts_per_revolution = 20/4,
//	});
//
//	myEncoder->start();
//	
//}

UI::UI(gpio_num_t sdaPin, 
	   gpio_num_t sclPin, 
	   i2c_port_t i2cPort, 
	   uint i2cClkFreq, 
	   gpio_num_t anEncAPin, 
	   gpio_num_t aEncBPin, 
	   gpio_num_t aButtonPin)
{
	myUIState = UIState::Stopped;
	myScreen = new Screen(sdaPin, sclPin, i2cPort, i2cClkFreq);
	myRef = this;
	esp_event_loop_args_t loopArgs = {
		.queue_size = 16,
		.task_name = "UIEventLoop", // task will be created
		.task_priority = uxTaskPriorityGet(NULL),
		.task_stack_size = 3072,
		.task_core_id = tskNO_AFFINITY};

	esp_event_loop_handle_t* loop;
	ESP_ERROR_CHECK(esp_event_loop_create(&loopArgs,loop));
	myUIEventLoop = std::make_shared<esp_event_loop_handle_t>(loop);
}

void UI::ProcessUIEventLoopTask(void *pvParameters)
{
	UI *ui = (UI *)pvParameters;
	while (true)
	{
		esp_event_loop_run(*ui->myUIEventLoop.get(), 500);
		vTaskDelay(50);
	}
}

void ProcessUIEventLoopIteration(void *aUi, esp_event_base_t base, int32_t id, void *payload)
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
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(myUIEventLoop.get(), UI_EVENT, ESP_EVENT_ANY_ID, ProcessUIEventLoopIteration, myRef, nullptr));
	ESP_ERROR_CHECK(xTaskCreatePinnedToCore(myScreen->UpdateTask, "ScreenUpdate", 2048, myScreen, 1, NULL, 0));
}
