/*
 * BuildMenu.cpp
 *
 *  Created on: Jan 14, 2026
 *      Author: wawer
 */

#include "BuildMenu.hpp"

RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

uint32_t checkUserTimeout_FromTime = 0;
uint32_t userTimeout = 10000;

// 1) jeden root
MenuItem root {"", nullptr, {}};

// 2) menu główne
static MenuItem welcomeScr0		{"AQUA CONTROLLER", nullptr, {}};
static MenuItem welcomeScr1		{"Konfiguracja", nullptr, {}};

static MenuItem miEnvCfg      {"Konf srodowiska", nullptr, {}};
static MenuItem miDeviceCfg   {"Konf urzadzenia", nullptr, {}};

// 3) podmenu: konf urzadzenia
static MenuItem miCfgTime     {"Czas", nullptr, {}};
static MenuItem miCfgSound    {"Dzwiek", nullptr, {}};
static MenuItem miCfgDisplay  {"Podswietlenie", Action_DisplayBackLight, {}};

// 4) podmenu: konf czasu
static MenuItem miTimeShow    {"Wyswietl aktualny", Action_PresentSystemTime, {}};
static MenuItem miTimeSet     {"Ustaw czas", nullptr, {}};

// 5) podmenu: konf dzwieku
static MenuItem miKeySoundsToggle{"wl/wyl dzwieki", Action_SetSoundsOnOff, {}};

// 6) podmenu: konf srodowiska
static MenuItem miLight       {"Swiatla", nullptr, {}};
static MenuItem miHeaters     {"Grzalki", nullptr, {}};
static MenuItem miWaterLevel  {"Poziom wody", nullptr, {}};

// 7) podmenu: swiatlo
static MenuItem miLightManualOnOff  {"Manual  wl/wyl ",[](){
	Action_SetOnOff(&aquaConfig.lightConfig.PowerOnOffLithtManual,true);
	if(aquaConfig.lightConfig.PowerOnOffLithtManual == 1)
	{
		aquaConfig.lightConfig.gpio.Enabled = 1;
		HAL_GPIO_WritePin(aquaConfig.lightConfig.gpio.port,aquaConfig.lightConfig.gpio.pin, GPIO_PIN_RESET);
	}
	else
	{
		aquaConfig.lightConfig.gpio.Enabled = 0;
		HAL_GPIO_WritePin(aquaConfig.lightConfig.gpio.port,aquaConfig.lightConfig.gpio.pin, GPIO_PIN_SET);
	}
}, {}};
static MenuItem miLightAutoOnOff  	{"Automat wl/wyl ",[](){Action_SetOnOff(&aquaConfig.lightConfig.PowerOnOffLithtAuto,false);}, {}};
static MenuItem miLightOn     		{"Godz zalaczenia", nullptr, {}};
static MenuItem miLightOff    		{"Godz wylaczenia", nullptr, {}};

// menu miHeaters
static MenuItem miWaterTempRead		{"Aktualna temp",Action_ReadWaterTemp,{}};
static MenuItem miWaterHeaterAdjust			{"Kalibracja temp", Action_TemperatureCalibration,{}};
// podemnu dla miHeaters
static MenuItem miWaterTempManualOnOff  	{"Manual  wl/wyl ", [](){
	Action_SetOnOff(&aquaConfig.heaterConfig.PowerOnOffHeaterManual,true);
	if(aquaConfig.heaterConfig.PowerOnOffHeaterManual == 1)
	{
		aquaConfig.heaterConfig.gpio.Enabled = 1;
		HAL_GPIO_WritePin(aquaConfig.heaterConfig.gpio.port,aquaConfig.heaterConfig.gpio.pin, GPIO_PIN_RESET);
	}
	else
	{
		aquaConfig.heaterConfig.gpio.Enabled = 0;
		HAL_GPIO_WritePin(aquaConfig.heaterConfig.gpio.port,aquaConfig.heaterConfig.gpio.pin, GPIO_PIN_SET);
	}
}, {}};
static MenuItem miWaterTempAutoOnOff  		{"Automat wl/wyl ", [](){Action_SetOnOff(&aquaConfig.heaterConfig.PowerOnOffHeaterAuto,false);}, {}};
static MenuItem miWaterTempSetMin			{"Temp zalaczania",Action_SetWaterMinimalTempON,{}};
static MenuItem miWaterTempSetMax			{"Temp wylaczania",Action_SetWaterMaxTempOFF,{}};


static MenuItem miWaterLevelRead  		{"Aktualny poziom", ReadActualWaterLVL, {}};
static MenuItem miWaterLevelAutoOnOff  	{"Automat wl/wyl", [](){Action_SetOnOff(&aquaConfig.waterConfig.ControlWaterLvlAuto,false);}, {}};
static MenuItem miWaterLevelReset  		{"Reset", [](){aquaConfig.waterConfig.WaterLevel = 0;}, {}};

// podmenu dla konfiguracji czasu miTimeSet
static MenuItem miTimeSetHours		{"Ustaw godzine", Action_SetHours, {}};
static MenuItem miTimeSetMinutes	{"Ustaw minute", Action_SetMinutes, {}};


// podmenu dla konfiguracji czasu miLightOn (envoroment)
static MenuItem miEnvTimeSetHoursOn{"Ustaw godzine",[](){Action_SetEnvHours(&aquaConfig.lightConfig.PowerOnLight.hours);},{}};
static MenuItem miEnvTimeSetMinutesOn{"Ustaw minute",[](){Action_SetEnvMinutes(&aquaConfig.lightConfig.PowerOnLight.minutes);},{}};
static MenuItem miEnvTimeSetHoursOff{"Ustaw godzine",[](){Action_SetEnvHours(&aquaConfig.lightConfig.PowerOffLight.hours);},{}};
static MenuItem miEnvTimeSetMinutesOff{"Ustaw minute",[](){Action_SetEnvMinutes(&aquaConfig.lightConfig.PowerOffLight.minutes);},{}};

void SavedSuccessfulyMessage()
{
	char buf[17];
	snprintf(buf, sizeof(buf), "Zapisano..");
	HD44780_Clear();
	HD44780_SetCursor(0, 1);
	HD44780_PrintStr(buf);
	HAL_Delay(400);
}
void InterruptedMessage()
{
	char buf[17];
	snprintf(buf, sizeof(buf), "Przerwano.. ");
	HD44780_Clear();
	HD44780_SetCursor(0, 1);
	HD44780_PrintStr(buf);
	HAL_Delay(400);
}
void SaveErrorMessage()
{
	char buf[17];
	snprintf(buf, sizeof(buf), "Blad zapisu.. ");
	HD44780_Clear();
	HD44780_SetCursor(0, 1);
	HD44780_PrintStr(buf);
	HAL_Delay(1000);
}

void BackToRootAndRender(){
	GoRoot(menu, &root);
	Render(menu);
}


bool UserTimeout(uint16_t timeout) {
	uint32_t TimeNOW = HAL_GetTick();

	if ((TimeNOW - checkUserTimeout_FromTime) >= timeout) {
		checkUserTimeout_FromTime = TimeNOW;
		return true;
	}
	return false;
}

void BuildMenu()
{
    // root -> main items
    Link(root, welcomeScr0);
    Link(root, welcomeScr1);

    Link(welcomeScr1, miEnvCfg);
    Link(welcomeScr1, miDeviceCfg);

    // device cfg
    Link(miDeviceCfg, miCfgTime);
    Link(miDeviceCfg, miCfgSound);
    Link(miDeviceCfg, miCfgDisplay);

    // konfiguracja czasu urzadzenia
    Link(miCfgTime, miTimeShow);
    Link(miCfgTime, miTimeSet);
    Link(miTimeSet, miTimeSetHours);
    Link(miTimeSet, miTimeSetMinutes);

    // sound cfg (klawisze)
    Link(miCfgSound, miKeySoundsToggle);

    // env cfg
    Link(miEnvCfg, miLight);
    Link(miEnvCfg, miHeaters);
    Link(miEnvCfg, miWaterLevel);

    // light Enviroment cfg
    Link(miLight, miLightManualOnOff);
    Link(miLight, miLightAutoOnOff);
    Link(miLight, miLightOn);
    Link(miLight, miLightOff);
    Link(miLightOn, miEnvTimeSetHoursOn);
    Link(miLightOn, miEnvTimeSetMinutesOn);
    Link(miLightOff, miEnvTimeSetHoursOff);
    Link(miLightOff, miEnvTimeSetMinutesOff);

    // konfiguracja grzalkami
    Link(miHeaters, miWaterTempRead);
    Link(miHeaters, miWaterHeaterAdjust);
    Link(miHeaters, miWaterTempManualOnOff);
    Link(miHeaters, miWaterTempAutoOnOff);
    Link(miHeaters, miWaterTempSetMin);
    Link(miHeaters, miWaterTempSetMax);

    // kontrola czujnika plywaka
    Link(miWaterLevel, miWaterLevelRead);
    Link(miWaterLevel, miWaterLevelAutoOnOff);
    Link(miWaterLevel, miWaterLevelReset);
}

MenuItem* MenuRoot()
{
    return &root;
}

void Action_PresentSystemTime()
{
	char buf[17];
	snprintf(buf, sizeof(buf), "Aktualny czas:");
	HD44780_Clear();
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr(buf);

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

//	rtc->ReadTime(&sTime);

	snprintf(buf, sizeof(buf), "%02d:%02d:%02d  ", sTime.Hours, sTime.Minutes, sTime.Seconds);
	HD44780_SetCursor(0, 1);
	HD44780_PrintStr(buf);

    uint32_t refreshEveryMS = 1000;
    uint32_t refresh_FromTime = 0;
    uint32_t TimeNOW = 0;

	checkUserTimeout_FromTime = HAL_GetTick();
	while (1) {
    	eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);
    	TimeNOW = HAL_GetTick();
    	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

//    	rtc->ReadTime(&sTime);

  		if ((TimeNOW - refresh_FromTime) >= refreshEveryMS) {
  			rtc->ReadTime(&sTime);
  			snprintf(buf, sizeof(buf), "%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
  	        HD44780_SetCursor(0, 1);
  	        HD44780_PrintStr(buf);
  	      refresh_FromTime = TimeNOW;
  		}


		if(UserTimeout(userTimeout))
		{
			BackToRootAndRender();
			break;
		}


		if (buttonpressed == SINGLE_PRESS) {
			InterruptedMessage();
			break;
		}

	}
}

void Action_SetHours()
{
	char buf[17];
	snprintf(buf, sizeof(buf), "Godzina:");
	HD44780_Clear();
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr(buf);

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	snprintf(buf, sizeof(buf), "%02d ", sTime.Hours);
	HD44780_SetCursor(0, 1);
	HD44780_PrintStr(buf);
	uint16_t encPosition = 0;
	encoder->ResetEncoderPosition();
	encoder->SetEncoderPosition(sTime.Hours);

	uint16_t oldEncPos = sTime.Hours;

	checkUserTimeout_FromTime = HAL_GetTick();
	while (1) {
		eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);

		encPosition = encoder->GetEncoderPosition();

		if (oldEncPos != encPosition) {
			checkUserTimeout_FromTime = HAL_GetTick();
			if (encPosition >= 0 && encPosition <= 23) {
				oldEncPos = encPosition;
				snprintf(buf, sizeof(buf), "%02d ", encPosition);
				HD44780_SetCursor(0, 1);
				HD44780_PrintStr(buf);
			} else if (encPosition == 99) {
				encoder->SetEncoderPosition(23);
			} else if (encPosition > 23 && encPosition < 100) {
				encoder->SetEncoderPosition(0);
			}
		} else {
			if (UserTimeout(userTimeout)) {
				BackToRootAndRender();
				break;
			}
		}

		if (buttonpressed == SINGLE_PRESS) {
			sTime.Hours =(uint8_t)encPosition;
			HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

			SetFakeDate(&sDate);
			HAL_StatusTypeDef st = rtc->SetDateTime(&sDate, &sTime);

			if (st != HAL_OK)
			{
				SaveErrorMessage();
			}
			else
			{
				SavedSuccessfulyMessage();
			}

			HAL_PWR_EnableBkUpAccess();
			HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, RTC_MAGIC);
			break;
		}

		if (buttonpressed == DOUBLE_PRESS) {
			InterruptedMessage();
			break;
		}

	}
}

void Action_SetMinutes()
{
	char buf[17];
	snprintf(buf, sizeof(buf), "Minuta:");
	HD44780_Clear();
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr(buf);

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	snprintf(buf, sizeof(buf), "%02d ", sTime.Minutes);
	HD44780_SetCursor(0, 1);
	HD44780_PrintStr(buf);
	uint16_t encPosition = 0;
	encoder->ResetEncoderPosition();
	encoder->SetEncoderPosition(sTime.Minutes);

	uint16_t oldEncPos = sTime.Minutes;

	checkUserTimeout_FromTime = HAL_GetTick();

	while (1) {
		eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);

		encPosition = encoder->GetEncoderPosition();

		if (oldEncPos != encPosition) {
			checkUserTimeout_FromTime = HAL_GetTick();
			if (encPosition >= 0 && encPosition <= 59) {
				oldEncPos = encPosition;
				snprintf(buf, sizeof(buf), "%02d ", encPosition);
				HD44780_SetCursor(0, 1);
				HD44780_PrintStr(buf);
			} else if (encPosition == 99) {
				encoder->SetEncoderPosition(59);
			} else if (encPosition > 59 && encPosition < 100) {
				encoder->SetEncoderPosition(0);
			}
		} else {
			if (UserTimeout(userTimeout)) {
				BackToRootAndRender();
				break;
			}
		}

		if (buttonpressed == SINGLE_PRESS) {
			sTime.Minutes =(uint8_t)encPosition;
			sTime.Seconds=0;
			HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

			SetFakeDate(&sDate);
			HAL_StatusTypeDef st = rtc->SetDateTime(&sDate, &sTime);
			if (st != HAL_OK)
			{
				SaveErrorMessage();
			}
			else
			{
				SavedSuccessfulyMessage();
			}

			HAL_PWR_EnableBkUpAccess();
			HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, RTC_MAGIC);
			break;
		}

		if (buttonpressed == DOUBLE_PRESS) {
			InterruptedMessage();
			break;
		}

	}
}

void Action_SetSoundsOnOff()
{
	char buf[17];
	snprintf(buf, sizeof(buf), "Dzwieki:");
	HD44780_Clear();
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr(buf);
	uint16_t soundValEncoder = devConfig.soundConfig.soundOnEncoder;
	uint16_t soundValOnPress = devConfig.soundConfig.soundOnKeyPress;

	if(soundValEncoder == 1)
	{
		snprintf(buf, sizeof(buf), "wlaczone ");
		HD44780_SetCursor(0, 1);
		HD44780_PrintStr(buf);
	}
	else
	{
		snprintf(buf, sizeof(buf), "wylaczone");
		HD44780_SetCursor(0, 1);
		HD44780_PrintStr(buf);
	}

	encoderShift shift = NoChange;

	while (1) {
		eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);

		shift = encoder->GetEncoderShift();

		if (shift != NoChange) {
			checkUserTimeout_FromTime = HAL_GetTick();

			switch (shift) {
			case UpIncrement:
				memset(buf, 0, sizeof(buf));
				snprintf(buf, sizeof(buf), "wylaczone");
				HD44780_SetCursor(0, 1);
				HD44780_PrintStr(buf);
				soundValEncoder = 0;
				break;

			case DownIncrement:
				memset(buf, 0, sizeof(buf));
				snprintf(buf, sizeof(buf), "wlaczone ");
				HD44780_SetCursor(0, 1);
				HD44780_PrintStr(buf);
				soundValEncoder = 1;
				break;
			}
		} else {
			if (UserTimeout(userTimeout)) {
				BackToRootAndRender();
				break;
			}
		}

		if (buttonpressed == SINGLE_PRESS) {
			devConfig.soundConfig.soundOnEncoder = soundValEncoder;
			devConfig.soundConfig.soundOnKeyPress = soundValEncoder;

			HAL_StatusTypeDef saveStatus = AT24C32_Epprom->Save(&devConfig, &aquaConfig);
			if (saveStatus != HAL_OK)
			{
				SaveErrorMessage();
			}
			else
			{
				SavedSuccessfulyMessage();
			}
			break;
		}

		if (buttonpressed == DOUBLE_PRESS) {
			InterruptedMessage();
			break;
		}

	}
}

void Action_DisplayBackLight()
{
    char buf[17];
    snprintf(buf, sizeof(buf), "Czas:");

	HD44780_Clear();
    HD44780_SetCursor(0, 0);
    HD44780_PrintStr(buf);
    snprintf(buf, sizeof(buf), "%02d sek", devConfig.displayConfig.displayOnTime);
    HD44780_SetCursor(0, 1);
    HD44780_PrintStr(buf);
    uint16_t encPosition = 0;
	encoder->ResetEncoderPosition();
	encoder->SetEncoderPosition(devConfig.displayConfig.displayOnTime);

	uint16_t oldEncPos=devConfig.displayConfig.displayOnTime;

	checkUserTimeout_FromTime = HAL_GetTick();

    while(1)
    {
    	eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);

    	encPosition =  encoder->GetEncoderPosition();

		if (oldEncPos != encPosition) {
			checkUserTimeout_FromTime = HAL_GetTick();
			oldEncPos = encPosition;
			snprintf(buf, sizeof(buf), "%02d sek", encPosition);
			HD44780_SetCursor(0, 1);
			HD44780_PrintStr(buf);
		} else {
			if (UserTimeout(userTimeout)) {
				BackToRootAndRender();
				break;
			}
		}

        if(buttonpressed == SINGLE_PRESS)
        {
        	devConfig.displayConfig.displayOnTime = encPosition;
			HAL_StatusTypeDef saveStatus = AT24C32_Epprom->Save(&devConfig, &aquaConfig);
			if (saveStatus != HAL_OK)
			{
				SaveErrorMessage();
			}
			else
			{
				SavedSuccessfulyMessage();
			}
        	break;
        }

        if (buttonpressed == DOUBLE_PRESS)
        {
			InterruptedMessage();
            break;
        }
    }
}

void Action_ReadWaterTemp()
{
    char buf[17];
    snprintf(buf, sizeof(buf), "Temperatura:");
	HD44780_Clear();
    HD44780_SetCursor(0, 0);
    HD44780_PrintStr(buf);
    snprintf(buf, sizeof(buf), "%.1fC  ", SensorTemp);
    HD44780_SetCursor(0, 1);
    HD44780_PrintStr(buf);
    uint32_t checkTemperatureEveryMS = 5000;
    uint32_t checkTemperature_FromTime = 0;
    uint32_t TimeNOW = 0;

	checkUserTimeout_FromTime = HAL_GetTick();

    while(1)
    {
    	eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);
    	TimeNOW = HAL_GetTick();

  		if ((TimeNOW - checkTemperature_FromTime) >= checkTemperatureEveryMS) {
  			SensorTemp = tempSensors->GetTemperature() + aquaConfig.heaterConfig.AdjTempSensor;
  	        snprintf(buf, sizeof(buf), "%.1fC  ", SensorTemp);
  	        HD44780_SetCursor(0, 1);
  	        HD44780_PrintStr(buf);
  			checkTemperature_FromTime = TimeNOW;
  		}


		if(UserTimeout(userTimeout))
		{
			BackToRootAndRender();
			break;
		}

        if (buttonpressed == SINGLE_PRESS)
        {
			InterruptedMessage();
            break;
        }

    }
}

void Action_TemperatureCalibration()
{
    char buf[17];
    snprintf(buf, sizeof(buf), "Kalibracja:");

	HD44780_Clear();
    HD44780_SetCursor(0, 0);
    HD44780_PrintStr(buf);
    snprintf(buf, sizeof(buf), "%d C", aquaConfig.heaterConfig.AdjTempSensor);
    HD44780_SetCursor(0, 1);
    HD44780_PrintStr(buf);
	encoder->ResetEncoderPosition();
	int16_t adjTemp = aquaConfig.heaterConfig.AdjTempSensor;
	encoder->ResetEncoderPosition();
	encoder->SetEncoderPosition(0);

	encoderShift shift = NoChange;

	checkUserTimeout_FromTime = HAL_GetTick();

	while (1) {
		eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);

		shift = encoder->GetEncoderShift();

		if (shift != NoChange) {
			checkUserTimeout_FromTime = HAL_GetTick();
			switch (shift) {
			case UpIncrement:
				if (adjTemp < 5)
					adjTemp++;
				memset(buf, 0, sizeof(buf));
				snprintf(buf, sizeof(buf), "%d C ", adjTemp);
				HD44780_SetCursor(0, 1);
				HD44780_PrintStr(buf);

				break;

			case DownIncrement:
				memset(buf, 0, sizeof(buf));
				if (adjTemp > -5)
					adjTemp--;
				snprintf(buf, sizeof(buf), "%d C ", adjTemp);
				HD44780_SetCursor(0, 1);
				HD44780_PrintStr(buf);
				break;
			}
		} else {
			if(UserTimeout(userTimeout))
			{
				BackToRootAndRender();
				break;
			}
		}

		if (buttonpressed == SINGLE_PRESS) {
			aquaConfig.heaterConfig.AdjTempSensor = adjTemp;
			HAL_StatusTypeDef saveStatus = AT24C32_Epprom->Save(&devConfig,
					&aquaConfig);
			if (saveStatus != HAL_OK) {
				SaveErrorMessage();
			} else {
				SavedSuccessfulyMessage();
			}
			break;
		}

		if (buttonpressed == DOUBLE_PRESS) {
			InterruptedMessage();
			break;
		}
	}
}

void Action_SetWaterMinimalTempON()
{
    char buf[17];
    snprintf(buf, sizeof(buf), "Temp zal grzalki");

	HD44780_Clear();
    HD44780_SetCursor(0, 0);
    HD44780_PrintStr(buf);
    snprintf(buf, sizeof(buf), "%02d C", aquaConfig.heaterConfig.TempMinONHeater);
    HD44780_SetCursor(0, 1);
    HD44780_PrintStr(buf);
    uint16_t encPosition = 0;
	encoder->ResetEncoderPosition();
	encoder->SetEncoderPosition(aquaConfig.heaterConfig.TempMinONHeater);

	uint16_t oldEncPos=aquaConfig.heaterConfig.TempMinONHeater;

	checkUserTimeout_FromTime = HAL_GetTick();

    while(1)
    {
    	eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);

    	encPosition =  encoder->GetEncoderPosition();

    	if(oldEncPos != encPosition)
    	{
    		checkUserTimeout_FromTime = HAL_GetTick();
			oldEncPos = encPosition;
			snprintf(buf, sizeof(buf), "%02d C", encPosition);
			HD44780_SetCursor(0, 1);
			HD44780_PrintStr(buf);
		} else {
			if(UserTimeout(userTimeout))
			{
				BackToRootAndRender();
				break;
			}
		}

        if(buttonpressed == SINGLE_PRESS)
        {
        	aquaConfig.heaterConfig.TempMinONHeater = encPosition;
			HAL_StatusTypeDef saveStatus = AT24C32_Epprom->Save(&devConfig, &aquaConfig);
			if (saveStatus != HAL_OK)
			{
				SaveErrorMessage();
			}
			else
			{
				SavedSuccessfulyMessage();
			}
        	break;
        }

        if (buttonpressed == DOUBLE_PRESS)
        {
			InterruptedMessage();
            break;
        }
    }
}

void Action_SetWaterMaxTempOFF()
{
    char buf[17];
    snprintf(buf, sizeof(buf), "Temp wyl grzalki");

	HD44780_Clear();
    HD44780_SetCursor(0, 0);
    HD44780_PrintStr(buf);
    snprintf(buf, sizeof(buf), "%02d C", aquaConfig.heaterConfig.TempMaxOffHeater);
    HD44780_SetCursor(0, 1);
    HD44780_PrintStr(buf);
    uint16_t encPosition = 0;
	encoder->ResetEncoderPosition();
	encoder->SetEncoderPosition(aquaConfig.heaterConfig.TempMaxOffHeater);

	uint16_t oldEncPos=aquaConfig.heaterConfig.TempMaxOffHeater;

	checkUserTimeout_FromTime = HAL_GetTick();

    while(1)
    {
    	eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);

    	encPosition =  encoder->GetEncoderPosition();

    	if(oldEncPos != encPosition)
    	{
    		checkUserTimeout_FromTime = HAL_GetTick();
			oldEncPos = encPosition;
	        snprintf(buf, sizeof(buf), "%02d C  ", encPosition);
	        HD44780_SetCursor(0, 1);
	        HD44780_PrintStr(buf);
		} else {
			if(UserTimeout(userTimeout))
			{
				BackToRootAndRender();
				break;
			}
		}


        if(buttonpressed == SINGLE_PRESS)
        {
        	aquaConfig.heaterConfig.TempMaxOffHeater = encPosition;
			HAL_StatusTypeDef saveStatus = AT24C32_Epprom->Save(&devConfig, &aquaConfig);
			if (saveStatus != HAL_OK)
			{
				SaveErrorMessage();
			}
			else
			{
				SavedSuccessfulyMessage();
			}
        	break;
        }

        if (buttonpressed == DOUBLE_PRESS)
        {
			InterruptedMessage();
            break;
        }
    }
}

void Action_SetOnOff(uint8_t *flag, bool isManual)
{
	uint8_t* CurrentSet = flag;

	char buf[17];
	if(isManual == true )
	{
		snprintf(buf, sizeof(buf), "Manual:");
	}
	else if(isManual == false)
	{
		snprintf(buf, sizeof(buf), "Auto:");
	}

	HD44780_Clear();
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr(buf);


	if(*CurrentSet == 1)
	{
		snprintf(buf, sizeof(buf), "wlaczone ");
		HD44780_SetCursor(0, 1);
		HD44780_PrintStr(buf);
	}
	else
	{
		snprintf(buf, sizeof(buf), "wylaczone");
		HD44780_SetCursor(0, 1);
		HD44780_PrintStr(buf);
	}

	encoderShift shift = NoChange;

	checkUserTimeout_FromTime = HAL_GetTick();

	while (1) {
		eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);

		shift = encoder->GetEncoderShift();

		if (shift != NoChange) {
			checkUserTimeout_FromTime = HAL_GetTick();
			switch (shift) {
			case UpIncrement:
				memset(buf, 0, sizeof(buf));
				snprintf(buf, sizeof(buf), "wylaczone");
				HD44780_SetCursor(0, 1);
				HD44780_PrintStr(buf);
				*CurrentSet = 0;
				break;

			case DownIncrement:
				memset(buf, 0, sizeof(buf));
				snprintf(buf, sizeof(buf), "wlaczone ");
				HD44780_SetCursor(0, 1);
				HD44780_PrintStr(buf);
				*CurrentSet = 1;
				break;
			}
		} else {
			if (UserTimeout(userTimeout)) {
				BackToRootAndRender();
				break;
			}
		}

		if (buttonpressed == SINGLE_PRESS) {

			HAL_StatusTypeDef saveStatus = AT24C32_Epprom->Save(&devConfig, &aquaConfig);
			if (saveStatus != HAL_OK)
			{
				SaveErrorMessage();
			}
			else
			{
				SavedSuccessfulyMessage();
			}
			break;
		}

		if (buttonpressed == DOUBLE_PRESS) {
			InterruptedMessage();
			break;
		}
	}
}

void Action_SetEnvHours(uint8_t *hour)
{
	char buf[17];

	uint16_t oldEncPos = *hour;

	snprintf(buf, sizeof(buf), "Godzina:");
	HD44780_Clear();
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr(buf);
	encoder->ResetEncoderPosition();
	encoder->SetEncoderPosition(oldEncPos);

	snprintf(buf, sizeof(buf), "%02d ", oldEncPos);

	HD44780_SetCursor(0, 1);
	HD44780_PrintStr(buf);
	uint16_t encPosition = 0;

	checkUserTimeout_FromTime = HAL_GetTick();

	while (1) {
		eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);

		encPosition = encoder->GetEncoderPosition();

		if (oldEncPos != encPosition) {
			checkUserTimeout_FromTime = HAL_GetTick();
			if (encPosition >= 0 && encPosition <= 23) {
				oldEncPos = encPosition;
				snprintf(buf, sizeof(buf), "%02d ", encPosition);
				HD44780_SetCursor(0, 1);
				HD44780_PrintStr(buf);
			} else if (encPosition == 99) {
				encoder->SetEncoderPosition(23);
			} else if (encPosition > 23 && encPosition < 100) {
				encoder->SetEncoderPosition(0);
			}
		} else {
			if (UserTimeout(userTimeout)) {
				BackToRootAndRender();
				break;
			}
		}

		if (buttonpressed == SINGLE_PRESS) {
			*hour = (uint8_t)encPosition;

			HAL_StatusTypeDef saveStatus = AT24C32_Epprom->Save(&devConfig, &aquaConfig);
			if (saveStatus != HAL_OK)
			{
				SaveErrorMessage();
			}
			else
			{
				SavedSuccessfulyMessage();
			}
			break;
		}

		if (buttonpressed == DOUBLE_PRESS) {
			InterruptedMessage();
			break;
		}
	}
}

void Action_SetEnvMinutes(uint8_t *minutes)
{
	char buf[17];
	uint16_t oldEncPos = *minutes;

	snprintf(buf, sizeof(buf), "Minuta:");
	HD44780_Clear();
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr(buf);
	encoder->ResetEncoderPosition();
	encoder->SetEncoderPosition(oldEncPos);

	snprintf(buf, sizeof(buf), "%02d ", oldEncPos);

	HD44780_SetCursor(0, 1);
	HD44780_PrintStr(buf);
	uint16_t encPosition = 0;

	checkUserTimeout_FromTime = HAL_GetTick();

	while (1) {
		eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);

		encPosition = encoder->GetEncoderPosition();

		if (oldEncPos != encPosition) {
			checkUserTimeout_FromTime = HAL_GetTick();
			if (encPosition >= 0 && encPosition <= 59) {
				oldEncPos = encPosition;
				snprintf(buf, sizeof(buf), "%02d ", encPosition);
				HD44780_SetCursor(0, 1);
				HD44780_PrintStr(buf);
			} else if (encPosition == 99) {
				encoder->SetEncoderPosition(59);
			} else if (encPosition > 59 && encPosition < 100) {
				encoder->SetEncoderPosition(0);
			}
		} else {
			if (UserTimeout(userTimeout)) {
				BackToRootAndRender();
				break;
			}
		}

		if (buttonpressed == SINGLE_PRESS) {
			*minutes = (uint8_t)encPosition;


			HAL_StatusTypeDef saveStatus = AT24C32_Epprom->Save(&devConfig, &aquaConfig);
			if (saveStatus != HAL_OK)
			{
				SaveErrorMessage();
			}
			else
			{
				SavedSuccessfulyMessage();
			}
			break;
		}

		if (buttonpressed == DOUBLE_PRESS) {
			InterruptedMessage();
			break;
		}

	}
}

void ReadActualWaterLVL()
{
	char buf[17];

	uint8_t actualWaterLvl = aquaConfig.waterConfig.WaterLevel;

	snprintf(buf, sizeof(buf), "Poziom wody:");
	HD44780_Clear();
	HD44780_SetCursor(0, 0);
	HD44780_PrintStr(buf);

	if(actualWaterLvl == 0)
	{
		snprintf(buf, sizeof(buf), "Niski ");
	}
	else
	{
		snprintf(buf, sizeof(buf), "Wysoki ");
	}

	HD44780_SetCursor(0, 1);
	HD44780_PrintStr(buf);

    uint32_t checkLevelEveryMS = 1000;
    uint32_t checkLevel_FromTime = 0;
    uint32_t TimeNOW = 0;
    uint8_t oldWaterLvl = actualWaterLvl;

    checkUserTimeout_FromTime = HAL_GetTick();
	while (1) {
		eButtonEvent buttonpressed = getButtonEvent(&EncoderButtonConfig);

		TimeNOW = HAL_GetTick();

		if ((TimeNOW - checkLevel_FromTime) >= checkLevelEveryMS) {

			// tu czyjaj !

			if(oldWaterLvl != actualWaterLvl)
			{
				oldWaterLvl = actualWaterLvl;

				if(actualWaterLvl == 0)
				{
					snprintf(buf, sizeof(buf), "niski ");
				}
				else
				{
					snprintf(buf, sizeof(buf), "Wysoki ");
				}

				HD44780_SetCursor(0, 1);
				HD44780_PrintStr(buf);
				checkLevel_FromTime = TimeNOW;
			}
		}


		if(UserTimeout(userTimeout))
		{
			BackToRootAndRender();
			break;
		}


		if (buttonpressed == SINGLE_PRESS) {

			break;
		}

		if (buttonpressed == DOUBLE_PRESS) {
			InterruptedMessage();
			break;
		}

	}
}

void SetFakeDate(RTC_DateTypeDef *date)
{
	date->WeekDay = RTC_WEEKDAY_FRIDAY;
	date->Date    = 16;
	date->Month   = RTC_MONTH_JANUARY;
	date->Year    = 26;   // 2026 -> 26
}


void RTC_InitOnce(void)
{
    HAL_PWR_EnableBkUpAccess();

    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != RTC_MAGIC)
    {
        RTC_TimeTypeDef t = {0};
        RTC_DateTypeDef d = {0};

        t.Hours = 12;
        t.Minutes = 0;
        t.Seconds = 0;

        d.WeekDay = RTC_WEEKDAY_MONDAY;
        d.Month   = RTC_MONTH_JANUARY;
        d.Date    = 1;
        d.Year    = 25;

        HAL_RTC_SetTime(&hrtc, &t, RTC_FORMAT_BIN);
        HAL_RTC_SetDate(&hrtc, &d, RTC_FORMAT_BIN);

        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, RTC_MAGIC);
    }
}
