
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

	ESP_ERROR_CHECK(esp_event_handler_instance_register(SETTINGS_EVENT, (int32_t)Event::UpdateSettings, &UpdateSettingsEventCallback, this, NULL));
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

SettingsData Settings::Load()
{
	// Open NVS handle
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle);
	if (err != ESP_OK)
	{
		return err;
	}

	// Read the encoder offset from NVS
	err = nvs_get_i32(my_handle, ENCODER_OFFSET_KEY, &myData.myEncoderOffset);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
	{
		nvs_close(my_handle);
		return err;
	}

	// Read the units from NVS
	uint8_t units;
	err = nvs_get_u8(my_handle, UNITS_KEY, &units);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
	{
		nvs_close(my_handle);
		return err;
	}
	myData.myUnits = static_cast<SpeedUnit>(units);

	// Close NVS handle
	nvs_close(my_handle);
	return ESP_OK;
}
