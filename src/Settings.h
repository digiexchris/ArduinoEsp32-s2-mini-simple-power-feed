#pragma once

#include "Event.h"
#include "config.h"
#include "shared.h"
#include "state.h"

struct SettingsData
{
	int32_t myNormalSpeed = 1;
	int32_t myRapidSpeed = maxOutputRPM;
	SpeedUnit mySpeedUnits = SpeedUnit::MMPM;
};

class Settings : public EventHandler
{
public:
	Settings();

	std::shared_ptr<SettingsData> Get(bool reload = false)
	{
		if (reload)
		{
			Load();
		}

		return myData;
	};

private:
	static void UpdateSettingsEventCallback(EventType bastypee, int32_t id, void *event_data);
	static void SaveSettingsTimerCallback(void *param);
	static std::unique_ptr<Settings> myRef;
	bool Save();
	bool Load();
	std::shared_ptr<SettingsData> myData;
	std::shared_ptr<SettingsData> mySavedData;

	// constants

	// Define the NVS namespace and keys for the settings
	static constexpr char const *NVS_NAMESPACE = "v0.1.4";
	static constexpr char const *NORMAL_SPEED_KEY = "0002";
	static constexpr char const *RAPID_SPEED_KEY = "0003";
	static constexpr char const *SPEED_UNITS_KEY = "0004";
};