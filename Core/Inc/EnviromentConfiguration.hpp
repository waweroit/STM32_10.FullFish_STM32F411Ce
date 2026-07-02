/*
 * EnviromentConfiguration.hpp
 *
 *  Created on: Jan 15, 2026
 *      Author: wawer
 */

#ifndef INC_ENVIROMENTCONFIGURATION_HPP_
#define INC_ENVIROMENTCONFIGURATION_HPP_

struct TimeConfig
{
	uint8_t hours = 0;
	uint8_t minutes = 0;
};

struct GPIOConfiguration
{
	uint8_t Enabled = 0;
	uint16_t pin;
	GPIO_TypeDef* port;
};

struct SoundsConfig
{
    uint8_t soundOnKeyPress = 0;
    uint8_t soundOnEncoder  = 0;
};

struct DisplayConfig
{
    uint16_t displayOnTime = 30;
};

struct DeviceConfig
{
    SoundsConfig  soundConfig;
    DisplayConfig displayConfig;
};

struct LightConfig
{
	GPIOConfiguration gpio;
	uint8_t PowerOnOffLithtManual = 0;
	uint8_t PowerOnOffLithtAuto = 1;
    TimeConfig PowerOnLight;
    TimeConfig PowerOffLight;
};

struct HeaterConfig
{
	GPIOConfiguration gpio;
	uint8_t PowerOnOffHeaterManual = 0;
	uint8_t PowerOnOffHeaterAuto = 1;
    uint16_t TempMinONHeater  = 26;
    uint16_t TempMaxOffHeater = 28;
    int16_t AdjTempSensor = 0;
};

struct WaterLvlConfig
{
	GPIOConfiguration gpio;
	uint8_t ControlWaterLvlAuto = 1;
	uint8_t WaterLevel = 1;
};

struct AquaConfig
{
	LightConfig lightConfig;
	HeaterConfig heaterConfig;
	WaterLvlConfig waterConfig;
};

// jeżeli chcesz globalne konfiguracje:
extern DeviceConfig devConfig;
extern AquaConfig   aquaConfig;


#endif /* INC_ENVIROMENTCONFIGURATION_HPP_ */
