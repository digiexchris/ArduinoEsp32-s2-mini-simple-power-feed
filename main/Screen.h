#pragma once

#include "state.h"
#include <ssd1306.h>

class Screen
{

  public:
	enum class SpeedUnit
	{
		MMPM,
		IPM
	} mySpeedUnit;

	Screen(gpio_num_t sdaPin, gpio_num_t sclPin, i2c_port_t i2cPort, uint i2cClkFreq = 100000);
	void SetSpeed(uint8_t aSpeed);
	void SetState(UIState aState);
	void SetUnit(SpeedUnit aUnit);
	void Update();
	static void UpdateTask(void *pvParameters);
	void SetSpeedState(SpeedState aSpeedState);

  private:
	uint8_t mySpeed;
	uint8_t myPrevSpeed;
	UIState myState;
	UIState myPrevState;
	SpeedState mySpeedState;
	SpeedState myPrevSpeedState;
	SpeedUnit myPrevSpeedUnit;
	ssd1306_handle_t ssd1306_dev;
	i2c_config_t conf;
	void DrawString(int x, int y, const char *str, int fontHeight, int fontWidth);
	void DrawSpeed();
	void DrawSpeedUnit();
	void DrawState();
	void DrawUnit();

	double SpeedPerMinute();
};