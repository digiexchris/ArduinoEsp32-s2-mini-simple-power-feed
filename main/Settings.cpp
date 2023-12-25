
#include "Settings.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "EventTypes.h"

ESP_EVENT_DEFINE_BASE(SETTINGS_EVENT);

Settings::Settings() : EventHandler(SETTINGS_EVENT)
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

	// Load settings from NVS
	Load();

	ESP_ERROR_CHECK(esp_event_handler_instance_register(SETTINGS_EVENT, (int32_t)Event::SetEncoderOffset, &UpdateSettingsEventCallback, this, NULL));

	const esp_timer_create_args_t timer_args = {
		.callback = &SaveSettingsTimerCallback,
		/* argument specified here will be passed to timer callback function */
		.arg = this,
		.name = "SaveSettings60S"};

	esp_timer_create(&timer_args, &myTimer);
	esp_timer_start_periodic(myTimer, 60000000);
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
	err = nvs_set_i32(my_handle, ENCODER_OFFSET_KEY, myData.myEncoderOffset);
	if (err != ESP_OK)
	{
		nvs_close(my_handle);
		return err;
	}

	// Write the units to NVS
	err = nvs_set_u8(my_handle, SPEED_UNITS_KEY, static_cast<uint8_t>(myData.myUnits));
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

	// Read the encoder offset from NVS
	err = nvs_get_i32(my_handle, ENCODER_OFFSET_KEY, &myData->myEncoderOffset);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
	{
//		nvs_close(my_handle);
//		return myData;
	}

	// Read the units from NVS
	uint8_t units;
	err = nvs_get_u8(my_handle, SPEED_UNITS_KEY, &(uint8_t)myData->myUnits);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
	{
//		nvs_close(my_handle);
//		return myData;
	}

	// Close NVS handle
	nvs_close(my_handle);
	return ESP_OK;
}

void Settings::SaveSettingsTimerCallback(void* param)
{
	bool changed = false;
	Settings *settings = (Settings *)param;
	int32_t offset = settings->myData->myEncoderOffset;
	if (settings->mySavedData->myEncoderOffset != offset)
	{
		settings->mySavedData->myEncoderOffset = offset;
		changed = true;
	}

	SpeedUnit units = settings->myData->myUnits;
	if (settings->mySavedData->myUnits != units)
	{
		settings->mySavedData->myUnits = units;
		changed = true;
	}

	if (changed)
	{
		settings->Save();
	}
}
