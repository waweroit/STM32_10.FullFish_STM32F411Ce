/*
 * RTCController.hpp
 *
 *  Created on: Jan 16, 2026
 *      Author: wawer
 */

#ifndef INC_RTCCONTROLLER_HPP_
#define INC_RTCCONTROLLER_HPP_

#include "rtc.h"
#include "i2c.h"
// i2c bedzie potrzebne do DS3231

class IRtc{
public:
	virtual ~IRtc() = default;
    virtual HAL_StatusTypeDef ReadTime(RTC_TimeTypeDef *sTime) = 0;
    virtual HAL_StatusTypeDef ReadDate(RTC_DateTypeDef *sDate) = 0;

    // zapis DS3231: czas + data
    virtual HAL_StatusTypeDef SetDateTime(RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime) = 0;
    virtual void SynchronizeRTC_MCU(RTC_HandleTypeDef *hrtc) = 0;
    // OSF=0 => oscylator nie był zatrzymany => czas można uznać za "ważny"
    virtual bool IsTimeValid() = 0;
};

class RtcController : public IRtc{
public:
	RtcController(I2C_HandleTypeDef *I2Ccomunication, uint8_t devAddress);
    HAL_StatusTypeDef ReadTime(RTC_TimeTypeDef *sTime) override;
    HAL_StatusTypeDef ReadDate(RTC_DateTypeDef *sDate) override;
    HAL_StatusTypeDef SetDateTime(RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime) override;
    void SynchronizeRTC_MCU(RTC_HandleTypeDef *hrtc) override;

    bool IsTimeValid() override;
private:
	I2C_HandleTypeDef *I2Ccomunication_;
	uint8_t devAddress_;
};

#endif /* INC_RTCCONTROLLER_HPP_ */
