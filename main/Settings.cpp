
#include "Settings.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "EventTypes.h"
#include "esp_timer.h"

std::unique_ptr<Settings> Settings::myRef = nullptr;

Settings::Settings() : EventHandler()
{
	// Initialize the NVS
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		// NVS partition was truncated and needs to be erased
		// Retry nvs_flash_init
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);
	
	myData = std::make_unique<SettingsData>();
	mySavedData = std::make_unique<SettingsData>();
	myRef.reset(this);

	// Load settings from NVS
	Load();

	RegisterEventHandler(SETTINGS_EVENT, Event::SetSpeedUnit, &UpdateSettingsEventCallback);
	RegisterEventHandler(SETTINGS_EVENT, Event::UpdateEncoderCount, &UpdateSettingsEventCallback);

	const esp_timer_create_args_t timer_args = {
		.callback = SaveSettingsTimerCallback,
		/* argument specified here will be passed to timer callback function */
		.arg = this,
		.name = "SaveSettings30S",
//		.skip_unhandled_events = true

	};

	esp_timer_create(&timer_args, &myTimer);
}

esp_err_t Settings::Save()
{
	// Open NVS handle
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
	if (err != ESP_OK)
	{
		return err;
	}

	// Write the encoder offset to NVS
	err = nvs_set_i32(my_handle, ENCODER_COUNT_KEY, myData->myEncoderCount);
	if (err != ESP_OK)
	{
		nvs_close(my_handle);
		return err;
	}

	// Write the units to NVS
	err = nvs_set_i32(my_handle, SPEED_UNITS_KEY, static_cast<uint32_t>(myData->mySpeedUnits));
	if (err != ESP_OK)
	{
		nvs_close(my_handle);
		return err;
	}

	// Commit written value to NVS
	err = nvs_commit(my_handle);
	if (err != ESP_OK)
	{
		nvs_close(my_handle);
		return err;
	}

	// Close NVS handle
	nvs_close(my_handle);

	ESP_LOGI("Settings", "Saved Settings");

	auto s = Get(true);
	ESP_LOGI("Settings", "Saved Values: %d, %d", s->mySpeedUnits, s->myEncoderCount);

	return ESP_OK;
}

esp_err_t Settings::Load()
{
	// Open NVS handle
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle);
	if (err != ESP_OK)
	{
		return -1;
	}
	
	// Read the normal speed from NVS
	err = nvs_get_i32(my_handle, ENCODER_COUNT_KEY, &myData->myEncoderCount);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
	{
		//		nvs_close(my_handle);
		//		return myData;
	}

	// Read the units from NVS
	int32_t units = 0;//static_cast<int32_t>(myData->mySpeedUnits);
	err = nvs_get_i32(my_handle, SPEED_UNITS_KEY, &units);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
	{
//		nvs_close(my_handle);
//		return myData;
	}
	
	myData->mySpeedUnits = static_cast<SpeedUnit>(units);
	mySavedData->myEncoderCount = myData->myEncoderCount;
	mySavedData->mySpeedUnits = static_cast<SpeedUnit>(units);

	// Close NVS handle
	nvs_close(my_handle);

	ESP_LOGI("Settings", "Saved Values: %d, %d", mySavedData->mySpeedUnits, mySavedData->myEncoderCount);
	
	return ESP_OK;
}

void Settings::SaveSettingsTimerCallback(void* param)
{
	bool changed = false;
	Settings *settings = (Settings *)param;
	
	int32_t count = settings->myData->myEncoderCount;
	if (settings->mySavedData->myEncoderCount != count)
	{
		settings->mySavedData->myEncoderCount = count;
		changed = true;
	}

	SpeedUnit units = settings->myData->mySpeedUnits;
	if (settings->mySavedData->mySpeedUnits != units)
	{
		settings->mySavedData->mySpeedUnits = units;
		changed = true;
	}

	if (changed)
	{
		settings->Save();
	}
}

void Settings::UpdateSettingsEventCallback(void* param, esp_event_base_t base, int32_t id, void* event_data)
{
	//Settings *settings = (Settings *)param;
	
	Event evt = static_cast<Event>(id);
	
	switch (evt)
	{
	case Event::SetSpeedUnit:
		{
			SingleValueEventData<SpeedUnit> *evtData = static_cast<SingleValueEventData<SpeedUnit> *>(event_data);
			auto su = evtData->myValue;
			myRef->myData->mySpeedUnits = su;
		}
		
		break;
	case Event::UpdateEncoderCount:
		{
			SingleValueEventData<uint32_t> *evtData = (SingleValueEventData<uint32_t> *)event_data;
			myRef->myData->myEncoderCount = evtData->myValue;
		}
		
		break;
	default:
		break;
	}
	
	//save these values to NVS if they're unchanged in 30 seconds
	if (esp_timer_is_active(myRef->myTimer)) 
	{
		esp_timer_stop(myRef->myTimer);
		esp_timer_start_once(myRef->myTimer, 30000000);
	}
	else 
	{
		esp_timer_start_once(myRef->myTimer, 30000000);
	}
}