
#include "Screen.h"
#include "icons.h"
#include "config.h"
#include "shared.h"
#include <sys/time.h>

extern "C"
{
#include <u8g2_esp32_hal.h>
};
#define ENABLE_SSD1306 1

Screen* Screen::myRef = nullptr;

Screen::Screen(gpio_num_t sdaPin, gpio_num_t sclPin, i2c_port_t i2cPort, uint32_t i2cClkFreq)
{
	myRef = this;
#if ENABLE_SSD1306
	u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.bus.i2c.sda = sdaPin;
	u8g2_esp32_hal.bus.i2c.scl = sclPin;
	u8g2_esp32_hal_init(u8g2_esp32_hal);

	u8g2_Setup_ssd1306_i2c_128x32_univision_f(
		&u8g2, U8G2_R0,
		// u8x8_byte_sw_i2c,
		u8g2_esp32_i2c_byte_cb,
		u8g2_esp32_gpio_and_delay_cb); // init u8g2 structure


#endif

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
	while (true)
	{
		myRef->Update();
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void Screen::Start()
	{
		#if ENABLE_SSD1306
		
		u8x8_SetI2CAddress(&u8g2.u8x8, 0x78);

		u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in
								// sleep mode after this,

		u8g2_SetPowerSave(&u8g2, 0); // wake up display
		u8g2_ClearBuffer(&u8g2);
		u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
		u8g2_DrawStr(&u8g2, 2, 17, "POWER!");
		u8g2_SendBuffer(&u8g2);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	
		BaseType_t result = xTaskCreatePinnedToCore(&Screen::UpdateTask, "update screen", 4048, this, 1, nullptr, 0);
		ASSERT_MSG(result == pdPASS, "Screen: Failed to create task, error: %d", result);
		#endif

		ESP_LOGI("Screen", "Screen init complete");
	}

void Screen::Update()
{
	u8g2_ClearBuffer(&u8g2);
	if (mySpeed != myPrevSpeed)
	{
		myPrevSpeed = mySpeed;
		
	}
	
	DrawSpeed();
	DrawSpeedUnit();
	
	if (myState != myPrevState)
	{
		myPrevState = myState;
	}
	
	u8g2_SendBuffer(&u8g2);
}

void Screen::DrawState()
{
	// struct timeval tv;
	// gettimeofday(&tv, NULL);
	// const bool flashHigh = tv.tv_usec < 500000 ? true : false;

	// switch (myState)
	// {
	// case UIState::MovingLeft:
	// 	if (mySpeedState == SpeedState::Rapid)
	// 	{
	// 		ssd1306_draw_bitmap(ssd1306_dev, 0, 0, moveleft3232, 32, 32);
	// 	}
	// 	else
	// 	{

	// 		ssd1306_draw_bitmap(ssd1306_dev, 0, 0, rapidleft3232, 32, 32);
	// 	}
	// 	break;
	// case UIState::MovingRight:
	// 	if (mySpeedState == SpeedState::Rapid)
	// 	{
	// 		ssd1306_draw_bitmap(ssd1306_dev, 0, 0, moveright3232, 32, 32);
	// 	}
	// 	else
	// 	{
	// 		ssd1306_draw_bitmap(ssd1306_dev, 0, 0, rapidright3232, 32, 32);
	// 	}
	// 	break;
	// case UIState::Stopping:
	// 	if (flashHigh)
	// 	{
	// 		ssd1306_draw_bitmap(ssd1306_dev, 0, 0, stop3232inverted, 32, 32);
	// 	}
	// 	else
	// 	{
	// 		ssd1306_draw_bitmap(ssd1306_dev, 0, 0, stop3232, 32, 32);
	// 	}
	// 	break;
	// case UIState::Stopped:
	// 	ssd1306_draw_bitmap(ssd1306_dev, 0, 0, stop3232, 32, 32);
	// 	break;
	// }

	// ssd1306_draw_bitmap(ssd1306_dev, 0, 0, stop3232, 32, 32);
}

void Screen::DrawSpeed()
{
	const float speedPerMinute = SpeedPerMinute();
	char buffer[20];
	sprintf(buffer, "%3.2f", speedPerMinute);
	
	u8g2_DrawStr(&u8g2, 2, 17, buffer);
	//DrawString(128 - 40, 0, buffer, ArialMT_Plain_16);
}

void Screen::SetSpeedState(SpeedState aSpeedState)
{
	mySpeedState = aSpeedState;
}

void Screen::SetSpeed(uint32_t aSpeed)
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
		u8g2_DrawStr(&u8g2, 60, 17, " mm/m");
		//ssd1306_draw_string(ssd1306_dev, 100, 16, (const uint8_t*)"mm/m", 16, 1);
		break;
	case SpeedUnit::IPM:
		u8g2_DrawStr(&u8g2, 60, 17, " IPM");
		//ssd1306_draw_string(ssd1306_dev, 100, 16, (const uint8_t*)"ipm", 16, 1);
		break;
	}
}

void Screen::SetState(UIState aState)
{
	myState = aState;
}

float Screen::SpeedPerMinute()
{
	const float mmPerMinute = mySpeed / stepsPerMm * 60;

	if (mySpeedUnit == SpeedUnit::MMPM)
	{
		return mmPerMinute;
	}
	else
	{
		return mmPerMinute * 0.0393701;
	}
}