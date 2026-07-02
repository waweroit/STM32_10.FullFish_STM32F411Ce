/*
 * AlarmRTC.hpp
 *
 *  Created on: Feb 1, 2026
 *      Author: wawer
 */

#ifndef INC_ALARMRTC_HPP_
#define INC_ALARMRTC_HPP_

#include "rtc.h"
#include "EnviromentConfiguration.hpp"
extern volatile float SensorTemp;
extern RTC_HandleTypeDef hrtc;

class IAlarmRTC{
public:
	virtual ~IAlarmRTC() = default;
	virtual void SetConfiguration(AquaConfig *config) = 0;
	virtual void ActionAlarms() = 0;
};

class alarmRTC : public IAlarmRTC
{
public:
	void SetConfiguration(AquaConfig *config) override;
	void ActionAlarms() override;

private:
	//RTC_HandleTypeDef *_hrtc;
	AquaConfig *_config;
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
};


bool compareTime(RTC_TimeTypeDef *time, TimeConfig *alarmTime);

#endif /* INC_ALARMRTC_HPP_ */
