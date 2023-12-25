#pragma once


#include "shared.h"
#include "Event.h"

struct SettingsData
{
	int32_t myEncoderOffset;
	SpeedUnit myUnits;
};

class Settings : public EventHandler
{
  public:
	Settings();
	
	std::shared_ptr<SettingsData> Get() { return myData; };
	std::shared_ptr<esp_event_loop_handle_t> GetEventLoop() { return myEventLoop; }

  private:
	
	static void RunSettingsQueue(void * param);
	static void UpdateSettingsEventCallback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
	void SaveSettingsTimerCallback();
	esp_err_t Save();
	SettingsData Load();
	std::shared_ptr <SettingsData> myData;
	
	
	//constants

	// Define the NVS namespace and keys for the settings
	static constexpr char const *NVS_NAMESPACE = "settings_namespace";
	static constexpr char const *ENCODER_OFFSET_KEY = "encoder_offset";
	static constexpr char const *SPEED_UNITS_KEY = "speed_units";
};