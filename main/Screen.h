#pragma once

#include "state.h"
#include <driver/i2c.h>
#include <u8g2.h>
#include <memory>
#include "shared.h"
#include "state.h"

extern "C"
{
#include <u8g2_esp32_hal.h>
};

class Screen
{

  public:
	

	Screen(gpio_num_t sdaPin, gpio_num_t sclPin, i2c_port_t i2cPort, uint32_t i2cClkFreq = 100000);
	void SetSpeed(uint32_t aSpeed);
	void SetState(UIState aState);
	void SetUnit(SpeedUnit aUnit);
	
	
	void SetSpeedState(SpeedState aSpeedState);
	void Start();
	void ToggleUnits();

	SpeedUnit mySpeedUnit = SpeedUnit::MMPM;
  private:
	uint32_t mySpeed = 0;
	uint32_t myPrevSpeed = 0;
	UIState myState = UIState::Stopped;
	UIState myPrevState = UIState::Stopped;
	SpeedState mySpeedState = SpeedState::Normal;
	SpeedState myPrevSpeedState = SpeedState::Normal;
	SpeedUnit myPrevSpeedUnit = SpeedUnit::MMPM;
	u8g2_t u8g2;
	u8g2_esp32_hal_t u8g2_esp32_hal;
	
	static Screen* myRef;
	
	static void UpdateTask(void *pvParameters);
	void Update();
	void DrawSpeed();
	void DrawSpeedUnit();
	void DrawState();
	void DrawUnit();

	float SpeedPerMinute();
	
	
	//icon placements
	uint8_t height = 32;
	uint8_t width = 32;
	uint8_t leftX = 0;
	uint8_t rightX = 128-32;
	uint8_t topY = 0;
	uint8_t bottomY = 0;
	uint8_t middleX = 64 - 16;
};