/*
 * RTCController.cpp
 *
 *  Created on: Jan 16, 2026
 *      Author: wawer
 */

#include "RTCController.hpp"

#include <cstdint>

// Rejestry DS3231
static constexpr uint8_t DS3231_REG_SECONDS = 0x00;
static constexpr uint8_t DS3231_REG_WEEKDAY = 0x03;
static constexpr uint8_t DS3231_REG_STATUS  = 0x0F;

// Status register bits
static constexpr uint8_t DS3231_STATUS_OSF  = 0x80; // Oscillator Stop Flag

static inline uint8_t Bin2Bcd(uint8_t v)
{
    return (uint8_t)(((v / 10u) << 4) | (v % 10u));
}

static inline uint8_t Bcd2Bin(uint8_t v)
{
    return (uint8_t)(((v >> 4) * 10u) + (v & 0x0Fu));
}

static inline bool ValidateTime(const RTC_TimeTypeDef *t)
{
    if (!t) return false;
    if (t->Hours > 23) return false;
    if (t->Minutes > 59) return false;
    if (t->Seconds > 59) return false;
    return true;
}

static inline bool ValidateDate(const RTC_DateTypeDef *d)
{
    if (!d) return false;
    if (d->Month < 1 || d->Month > 12) return false;
    if (d->Date  < 1 || d->Date  > 31) return false;
    if (d->Year  > 99) return false;
    if (d->WeekDay < 1 || d->WeekDay > 7) return false;
    return true;
}

RtcController::RtcController(I2C_HandleTypeDef *I2Ccomunication, uint8_t devAddress)
    : I2Ccomunication_(I2Ccomunication), devAddress_(devAddress)
{
    // 7-bit -> 8-bit dla HAL
    devAddress_ = (uint8_t)(devAddress << 1);
}

bool RtcController::IsTimeValid()
{
    uint8_t st = 0;
    if (HAL_I2C_Mem_Read(I2Ccomunication_, devAddress_,
                         DS3231_REG_STATUS, I2C_MEMADD_SIZE_8BIT,
                         &st, 1, 100) != HAL_OK)
    {
        return false; // brak komunikacji => nie ufamy
    }

    // OSF=1 => oscylator był zatrzymany (czas mógł być nieważny)
    return (st & DS3231_STATUS_OSF) == 0;
}

HAL_StatusTypeDef RtcController::SetDateTime(RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime)
{
    if (!ValidateTime(sTime) || !ValidateDate(sDate)) {
        return HAL_ERROR;
    }

    // DS3231: sec, min, hour, wday, date, month, year
    uint8_t b[7];

    b[0] = Bin2Bcd((uint8_t)sTime->Seconds) & 0x7F;
    b[1] = Bin2Bcd((uint8_t)sTime->Minutes) & 0x7F;

    // Godziny: zapis w trybie 24h (bit6=0)
    b[2] = Bin2Bcd((uint8_t)sTime->Hours) & 0x3F;

    b[3] = (uint8_t)(sDate->WeekDay & 0x07);                 // 1..7
    b[4] = Bin2Bcd((uint8_t)sDate->Date) & 0x3F;             // 1..31
    b[5] = Bin2Bcd((uint8_t)sDate->Month) & 0x1F;            // 1..12, century=0
    b[6] = Bin2Bcd((uint8_t)sDate->Year);                    // 0..99

    // Zapis od rejestru 0x00
    HAL_StatusTypeDef st = HAL_I2C_Mem_Write(I2Ccomunication_, devAddress_,
                                             DS3231_REG_SECONDS, I2C_MEMADD_SIZE_8BIT,
                                             b, sizeof(b), 100);
    if (st != HAL_OK) {
        return st;
    }

    // Po ustawieniu czasu skasuj OSF (od teraz czas uznajemy za ważny)
    uint8_t statusReg = 0;
    st = HAL_I2C_Mem_Read(I2Ccomunication_, devAddress_,
                          DS3231_REG_STATUS, I2C_MEMADD_SIZE_8BIT,
                          &statusReg, 1, 100);
    if (st != HAL_OK) {
        return st;
    }

    statusReg &= (uint8_t)~DS3231_STATUS_OSF;
    st = HAL_I2C_Mem_Write(I2Ccomunication_, devAddress_,
                           DS3231_REG_STATUS, I2C_MEMADD_SIZE_8BIT,
                           &statusReg, 1, 100);
    return st;
}

HAL_StatusTypeDef RtcController::ReadTime(RTC_TimeTypeDef *sTime)
{
    if (!sTime) return HAL_ERROR;

    uint8_t b[3] = {0};

    // Odczyt: sec, min, hour
    HAL_StatusTypeDef st = HAL_I2C_Mem_Read(I2Ccomunication_, devAddress_,
                                            DS3231_REG_SECONDS, I2C_MEMADD_SIZE_8BIT,
                                            b, sizeof(b), 100);
    if (st != HAL_OK) {
        return st;
    }

    uint8_t sec_bcd = b[0] & 0x7F;
    uint8_t min_bcd = b[1] & 0x7F;

    uint8_t hour_raw = b[2];
    uint8_t hours = 0;

    if (hour_raw & 0x40) {
        // 12h mode (na wszelki wypadek)
        uint8_t hour_bcd = hour_raw & 0x1F;       // 1..12
        uint8_t pm = (hour_raw & 0x20) ? 1 : 0;   // PM?
        uint8_t h12 = Bcd2Bin(hour_bcd);
        if (h12 == 12) h12 = 0;
        hours = (uint8_t)(h12 + (pm ? 12 : 0));
    } else {
        // 24h mode
        hours = Bcd2Bin(hour_raw & 0x3F);
    }

    sTime->Seconds = Bcd2Bin(sec_bcd);
    sTime->Minutes = Bcd2Bin(min_bcd);
    sTime->Hours   = hours;

    sTime->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime->StoreOperation = RTC_STOREOPERATION_RESET;
    sTime->TimeFormat     = RTC_HOURFORMAT12_AM; // bez znaczenia tutaj

    return ValidateTime(sTime) ? HAL_OK : HAL_ERROR;
}

HAL_StatusTypeDef RtcController::ReadDate(RTC_DateTypeDef *sDate)
{
    if (!sDate) return HAL_ERROR;

    // Odczyt: weekday, date, month, year (0x03..0x06)
    uint8_t b[4] = {0};

    HAL_StatusTypeDef st = HAL_I2C_Mem_Read(I2Ccomunication_, devAddress_,
                                            DS3231_REG_WEEKDAY, I2C_MEMADD_SIZE_8BIT,
                                            b, sizeof(b), 100);
    if (st != HAL_OK) {
        return st;
    }

    sDate->WeekDay = b[0] & 0x07;
    sDate->Date    = Bcd2Bin(b[1] & 0x3F);

    uint8_t month_raw = b[2];
    sDate->Month = Bcd2Bin(month_raw & 0x1F); // ignorujemy century
    sDate->Year  = Bcd2Bin(b[3]);             // 0..99

    return ValidateDate(sDate) ? HAL_OK : HAL_ERROR;
}

void RtcController::SynchronizeRTC_MCU(RTC_HandleTypeDef *hrtc)
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	ReadTime(&sTime);
	ReadDate(&sDate);

	HAL_RTC_SetDate(hrtc, &sDate, RTC_FORMAT_BIN);
	HAL_RTC_SetTime(hrtc, &sTime, RTC_FORMAT_BIN);
}
