
#include "Screen.h"
#include "icons.h"
#include "config.h"
#include "shared.h"
#include <sys/time.h>

extern "C"
{
#include <u8g2_esp32_hal.h>
};


Screen* Screen::myRef = nullptr;

Screen::Screen(gpio_num_t sdaPin, gpio_num_t sclPin, i2c_port_t , uint32_t )
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
		vTaskDelay(10 * portTICK_PERIOD_MS);
		u8x8_SetI2CAddress(&u8g2.u8x8, 0x78);

		u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in
								 // sleep mode after this,

		u8g2_SetPowerSave(&u8g2, 0); // wake up display
		u8g2_ClearBuffer(&u8g2);
		u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);

		const char *top = "POWER";
		uint8_t textWidth = u8g2_GetUTF8Width(&u8g2, top);
		uint8_t x = (128 - textWidth) / 2;
		u8g2_DrawStr(&u8g2, x, 16, top);

		const char *bottom = "FEED";
		textWidth = u8g2_GetUTF8Width(&u8g2, bottom);
		x = (128 - textWidth) / 2;
		u8g2_DrawStr(&u8g2, x, 32, bottom);
		
		u8g2_SendBuffer(&u8g2);
		vTaskDelay(1000 / portTICK_PERIOD_MS);

		BaseType_t result = xTaskCreatePinnedToCore(&Screen::UpdateTask, "update screen", 4048, this, 1, nullptr, 0);
		ASSERT_MSG(result == pdPASS, "Screen: Failed to create task, error: %d", result);
		#endif

		ESP_LOGI("Screen", "Screen init complete");
	}

void Screen::Update()
{
	#if ENABLE_SSD1306
	u8g2_ClearBuffer(&u8g2);
	if (mySpeed != myPrevSpeed)
	{
		myPrevSpeed = mySpeed;
		
	}
	
	if (myState != myPrevState)
	{
		myPrevState = myState;
	}
	
	DrawSpeed();
	DrawSpeedUnit();
	DrawState();

	
	
	u8g2_SendBuffer(&u8g2);

	#endif
}

void Screen::DrawState()
{
	 struct timeval tv;
	 gettimeofday(&tv, NULL);
	 const bool flashHigh = tv.tv_usec < 500000 ? true : false;
	 
	 
	 

	 switch (myState)
	 {
	 case UIState::MovingLeft:
	 	if (mySpeedState == SpeedState::Rapid)
	 	{
			//u8g2.drawXBM(0, 0, 32, 32, moveleft3232);
			u8g2_DrawXBM(&u8g2, leftX, bottomY, rapidleft32HeightPixels, rapidleft32WidthPixels, rapidleft32);
	 	}
	 	else
	 	{
			u8g2_DrawXBM(&u8g2, leftX, bottomY, moveleft32HeightPixels, moveleft32WidthPixels, moveleft32);
	 	}
	 	break;
	 case UIState::MovingRight:
	 	if (mySpeedState == SpeedState::Rapid)
	 	{
			u8g2_DrawXBM(&u8g2, rightX, bottomY, rapidright32HeightPixels, rapidright32WidthPixels, rapidright32);
		}
			else
		{
			u8g2_DrawXBM(&u8g2, rightX, bottomY, moveright32HeightPixels, moveright32WidthPixels, moveright32);
	 	}
	 	break;
	 case UIState::Stopping:
	 	if (flashHigh)
	 	{
			u8g2_DrawXBM(&u8g2, rightX, bottomY, stop32HeightPixels, stop32WidthPixels, stop32inverted);
			u8g2_DrawXBM(&u8g2, leftX, bottomY, stop32HeightPixels, stop32WidthPixels, stop32);
	 	}
	 	else
	 	{
			u8g2_DrawXBM(&u8g2, rightX, bottomY, stop32HeightPixels, stop32WidthPixels, stop32);
			u8g2_DrawXBM(&u8g2, leftX, bottomY, stop32HeightPixels, stop32WidthPixels, stop32inverted);
	 	}
	 	break;
	 case UIState::Stopped:
		u8g2_DrawXBM(&u8g2, leftX, bottomY, stop32HeightPixels, stop32WidthPixels, stop32);
		u8g2_DrawXBM(&u8g2, rightX, bottomY, stop32HeightPixels, stop32WidthPixels, stop32);
	 	break;
		//u8g2_DrawXBM(&u8g2, bottomY, leftX, height, width, rapidleft3232);
	 }

	// ssd1306_draw_bitmap(ssd1306_dev, 0, 0, stop3232, 32, 32);
}

void Screen::DrawSpeed()
{
	u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
	const float speedPerMinute = SpeedPerMinute();
	char buffer[20];
	sprintf(buffer, "%3.2f", speedPerMinute);

	const uint8_t textWidth = u8g2_GetUTF8Width(&u8g2,buffer);
	const uint8_t x = (128 - textWidth) / 2;
	u8g2_DrawStr(&u8g2, x, 16, buffer);
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
	u8g2_SetFont(&u8g2, u8g2_font_ncenB12_tr);
	switch (mySpeedUnit)
	{
		case SpeedUnit::MMPM: 
		{
			const char *buffer = "mm/m";
			const uint8_t textWidth = u8g2_GetUTF8Width(&u8g2, buffer);
			const uint8_t x = (128 - textWidth) / 2;
			u8g2_DrawStr(&u8g2, x, 32, "mm/m");
			break;
		}
		case SpeedUnit::IPM:
		{
			const char *buffer = "IPM";
			const uint8_t textWidth = u8g2_GetUTF8Width(&u8g2, buffer);
			const uint8_t x = (128 - textWidth) / 2;
			u8g2_DrawStr(&u8g2, x, 32, "IPM");
			break;
		
		}
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

void Screen::ToggleUnits() 
{
	if (mySpeedUnit == SpeedUnit::MMPM)
	{
		mySpeedUnit = SpeedUnit::IPM;
	}
	else
	{
		mySpeedUnit = SpeedUnit::MMPM;
	}
}