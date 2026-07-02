/*
 * AlarmRTC.cpp
 *
 *  Created on: Feb 1, 2026
 *      Author: wawer
 */


#include "AlarmRTC.hpp"
#include <cmath>


void alarmRTC::SetConfiguration(AquaConfig *config)
{
//	_hrtc = hrtc;
	_config = config;
}

void alarmRTC::ActionAlarms()
{
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	//HAL_StatusTypeDef status = _rtc->ReadTime(time);
//	if(status == HAL_OK)
//	{
		if(_config->lightConfig.PowerOnOffLithtAuto == 1 && _config->lightConfig.PowerOnOffLithtManual == 0)
		{
			if(compareTime(&time, &_config->lightConfig.PowerOnLight) && _config->lightConfig.gpio.Enabled == 0)
			{
				_config->lightConfig.gpio.Enabled = 1;
				HAL_GPIO_WritePin(_config->lightConfig.gpio.port,_config->lightConfig.gpio.pin, GPIO_PIN_RESET);
			}

			if(compareTime(&time, &_config->lightConfig.PowerOffLight) && _config->lightConfig.gpio.Enabled == 1)
			{
				_config->lightConfig.gpio.Enabled = 0;
				HAL_GPIO_WritePin(_config->lightConfig.gpio.port,_config->lightConfig.gpio.pin, GPIO_PIN_SET);
			}
		}

		if(_config->heaterConfig.PowerOnOffHeaterAuto == 1 && _config->heaterConfig.PowerOnOffHeaterManual == 0)
		{
			int16_t temp = (int16_t)lroundf(SensorTemp);

			if((temp < _config->heaterConfig.TempMinONHeater  ) && (_config->heaterConfig.gpio.Enabled == 0))
			{
				_config->heaterConfig.gpio.Enabled = 1;
				HAL_GPIO_WritePin(_config->heaterConfig.gpio.port,_config->heaterConfig.gpio.pin, GPIO_PIN_RESET);
			}

			if((temp >_config->heaterConfig.TempMaxOffHeater  && temp > _config->heaterConfig.TempMinONHeater) && (_config->heaterConfig.gpio.Enabled == 1))
			{
				_config->heaterConfig.gpio.Enabled = 0;
				HAL_GPIO_WritePin(_config->heaterConfig.gpio.port,_config->heaterConfig.gpio.pin, GPIO_PIN_SET);
			}
		}
//	}
}

bool compareTime(RTC_TimeTypeDef *time, TimeConfig *alarmTime)
{

	if(time->Hours == alarmTime->hours)
	{
		if(time->Minutes == alarmTime->minutes)
			return true;
	}
	return false;
}
