
#include "Settings.h"
#include "EventTypes.h"

std::unique_ptr<Settings> Settings::myRef = nullptr;

Settings::Settings() : EventHandler()
{
	// Initialize the NVS
	// esp_err_t err = nvs_flash_init();
	// if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
	// {
	// 	// NVS partition was truncated and needs to be erased
	// 	// Retry nvs_flash_init
	// 	ESP_ERROR_CHECK(nvs_flash_erase());
	// 	err = nvs_flash_init();
	// 	myData->myNormalSpeed = 5;
	// 	myData->myRapidSpeed = maxStepsPerSecond;
	// 	myData->mySpeedUnits = SpeedUnit::MMPM;
	// 	Save();
	// }
	// ESP_ERROR_CHECK(err);

	myData = std::make_unique<SettingsData>();
	mySavedData = std::make_unique<SettingsData>();
	myRef.reset(this);

	// Load settings from NVS
	Load();

	RegisterEventHandler(EventType::SETTINGS_EVENT, Event::SetSpeedUnit, &UpdateSettingsEventCallback);
	RegisterEventHandler(EventType::SETTINGS_EVENT, Event::SaveNormalSpeed, &UpdateSettingsEventCallback);
	RegisterEventHandler(EventType::SETTINGS_EVENT, Event::SaveRapidSpeed, &UpdateSettingsEventCallback);

	// const esp_timer_create_args_t timer_args = {
	// 	.callback = SaveSettingsTimerCallback,
	// 	/* argument specified here will be passed to timer callback function */
	// 	.arg = this,
	// 	.dispatch_method = ESP_TIMER_TASK,
	// 	.name = "SaveSettings30S",
	// 	.skip_unhandled_events = true

	// };

	// esp_timer_create(&timer_args, &myTimer);
}

bool Settings::Save()
{
	// Open NVS handle
	// nvs_handle_t my_handle;
	// esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
	// if (err != ESP_OK)
	// {
	// 	return err;
	// }

	// // Write the encoder offset to NVS
	// err = nvs_set_i32(my_handle, NORMAL_SPEED_KEY, myData->myNormalSpeed);
	// if (err != ESP_OK)
	// {
	// 	nvs_close(my_handle);
	// 	return err;
	// }

	// // Write the encoder offset to NVS
	// err = nvs_set_i32(my_handle, RAPID_SPEED_KEY, myData->myRapidSpeed);
	// if (err != ESP_OK)
	// {
	// 	nvs_close(my_handle);
	// 	return err;
	// }

	// // Write the units to NVS
	// err = nvs_set_i32(my_handle, SPEED_UNITS_KEY, static_cast<uint32_t>(myData->mySpeedUnits));
	// if (err != ESP_OK)
	// {
	// 	nvs_close(my_handle);
	// 	return err;
	// }

	// // Commit written value to NVS
	// err = nvs_commit(my_handle);
	// if (err != ESP_OK)
	// {
	// 	nvs_close(my_handle);
	// 	return err;
	// }

	// // Close NVS handle
	// nvs_close(my_handle);

	printf("Settings", "Saved Settings");

	auto s = Get(true);
	printf("Settings", "Saved Values: %d, %d, $d", s->mySpeedUnits, s->myNormalSpeed, s->myRapidSpeed);

	return true;
}

bool Settings::Load()
{
	// // Open NVS handle
	// nvs_handle_t my_handle;
	// esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle);
	// if (err != ESP_OK)
	// {
	// 	return -1;
	// }

	// // Read the normal speed from NVS
	// err = nvs_get_i32(my_handle, NORMAL_SPEED_KEY, &myData->myNormalSpeed);
	// if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
	// {
	// 	//		nvs_close(my_handle);
	// 	//		return myData;
	// }

	// err = nvs_get_i32(my_handle, RAPID_SPEED_KEY, &myData->myRapidSpeed);
	// if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
	// {
	// 	//		nvs_close(my_handle);
	// 	//		return myData;
	// }

	// // Read the units from NVS
	int32_t units = 0; // static_cast<int32_t>(myData->mySpeedUnits);
	// err = nvs_get_i32(my_handle, SPEED_UNITS_KEY, &units);
	// if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
	// {
	// 	//		nvs_close(my_handle);
	// 	//		return myData;
	// }

	myData->mySpeedUnits = static_cast<SpeedUnit>(units);
	mySavedData->myNormalSpeed = myData->myNormalSpeed;
	mySavedData->myRapidSpeed = myData->myRapidSpeed;
	mySavedData->mySpeedUnits = static_cast<SpeedUnit>(units);

	// // Close NVS handle
	// nvs_close(my_handle);

	printf("Settings", "Saved Values: %d, %d, %d", mySavedData->mySpeedUnits, mySavedData->myNormalSpeed, mySavedData->myRapidSpeed);

	return true;
}

void Settings::SaveSettingsTimerCallback(void *param)
{
	bool changed = false;
	Settings *settings = (Settings *)param;

	if (int32_t normalSpeed = settings->myData->myNormalSpeed; settings->mySavedData->myNormalSpeed != normalSpeed)
	{
		settings->mySavedData->myNormalSpeed = normalSpeed;
		changed = true;
	}

	if (int32_t rapidSpeed = settings->myData->myRapidSpeed; settings->mySavedData->myRapidSpeed != rapidSpeed)
	{
		settings->mySavedData->myRapidSpeed = rapidSpeed;
		changed = true;
	}

	if (SpeedUnit units = settings->myData->mySpeedUnits; settings->mySavedData->mySpeedUnits != units)
	{
		settings->mySavedData->mySpeedUnits = units;
		changed = true;
	}

	if (changed)
	{
		settings->Save();
	}
}

void Settings::UpdateSettingsEventCallback(EventType type, int32_t id, void *event_data)
{
	// Settings *settings = (Settings *)param;

	auto evt = static_cast<Event>(id);

	switch (evt)
	{
	case Event::SetSpeedUnit:
	{
		auto const *evtData = static_cast<SingleValueEventData<SpeedUnit> *>(event_data);
		auto su = evtData->myValue;
		myRef->myData->mySpeedUnits = su;
	}

	break;
	case Event::SaveNormalSpeed:
	{
		auto const *evtData = (SingleValueEventData<uint32_t> *)event_data;
		myRef->myData->myNormalSpeed = evtData->myValue;
	}

	break;
	case Event::SaveRapidSpeed:
	{
		auto const *evtData = (SingleValueEventData<uint32_t> *)event_data;
		myRef->myData->myRapidSpeed = evtData->myValue;
	}
	break;
	default:
		break;
	}

	// save these values to NVS if they're unchanged in 30 seconds
	// if (esp_timer_is_active(myRef->myTimer))
	// {
	// 	esp_timer_stop(myRef->myTimer);
	// 	esp_timer_start_once(myRef->myTimer, 30000000);
	// }
	// else
	// {
	// 	esp_timer_start_once(myRef->myTimer, 30000000);
	// }
}