#pragma once


#include "shared.h"
#include "Event.h"
#include "state.h"

struct SettingsData
{
	int32_t myEncoderCount;
	SpeedUnit mySpeedUnits;
};

class Settings : public EventHandler
{
  public:
	Settings();
	
	std::shared_ptr<SettingsData> Get(bool reload = false) {
		if (reload)
		{
			Load();
		}
		
		return myData; 
	};

  private:

	static void UpdateSettingsEventCallback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
	static void SaveSettingsTimerCallback(void* param);
	static std::unique_ptr<Settings> myRef;
	esp_err_t Save();
	esp_err_t Load();
	std::shared_ptr <SettingsData> myData;
	std::shared_ptr<SettingsData> mySavedData;
	esp_timer_handle_t myTimer;
	
	//constants

	// Define the NVS namespace and keys for the settings
	static constexpr char const *NVS_NAMESPACE = "v0.1.0";
	static constexpr char const *ENCODER_COUNT_KEY = "0002";
	static constexpr char const *SPEED_UNITS_KEY = "0003";
};