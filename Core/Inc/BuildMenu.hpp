/*
 * BuildMenu.hpp
 *
 *  Created on: Jan 14, 2026
 *      Author: wawer
 */

#ifndef INC_BUILDMENU_HPP_
#define INC_BUILDMENU_HPP_

#define RTC_MAGIC 0xA5A5

#include "MainMenu.hpp"
#include "buttonsSetup.h"
#include "EncoderController.hpp"
#include "TemperatureSensors.hpp"
#include "EnviromentConfiguration.hpp"
#include "rtc.h"
#include "RTCController.hpp"
#include "AT24C32_ConfigStorage.hpp"

extern MenuState menu;

extern volatile float SensorTemp;
extern IEncoder *encoder;
extern ButtonConfig EncoderButtonConfig;
extern ITemperatureSensor *tempSensors;
extern IConfigStorage *AT24C32_Epprom;

extern IRtc *rtc;

extern RTC_HandleTypeDef hrtc;


void BuildMenu();
MenuItem* MenuRoot();
void BackToRootAndRender();
bool UserTimeout(uint16_t timeout);

void Action_PresentSystemTime();
void Action_SetHours();
void Action_SetMinutes();
void Action_SetSoundsOnOff();
void Action_DisplayBackLight();

void Action_ReadWaterTemp();
void Action_TemperatureCalibration();
void Action_SetWaterMinimalTempON();
void Action_SetWaterMaxTempOFF();

void Action_SetOnOff(uint8_t *flag, bool isManual);
void Action_SetEnvHours(uint8_t *hour);
void Action_SetEnvMinutes(uint8_t *minutes);

void ReadActualWaterLVL();

//void Action_SetEnvHours(AquaConfig *Enviroment, bool powerON);
//void Action_SetEnvMinutes(AquaConfig *Enviroment, bool powerON);


void SetFakeDate(RTC_DateTypeDef *date);
void RTC_InitOnce(void);
#endif /* INC_BUILDMENU_HPP_ */
