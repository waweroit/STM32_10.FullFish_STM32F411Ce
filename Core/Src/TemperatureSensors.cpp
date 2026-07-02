/*
 * TemperatureSensors.cpp
 *
 *  Created on: Dec 17, 2025
 *      Author: wawer
 */
#include "TemperatureSensors.hpp"
//#include <cmath> // logf

//NTCSensor::NTCSensor(ADC_HandleTypeDef *hadc,  uint16_t devicePosInBuffer): hadc_(hadc), devicePosInBuffer_(devicePosInBuffer)
//{
//}
//
//float NTCSensor::GetTemperature()
//{
//	// cikawie kod wygenerowany przez AI :) hmmm :)
//    float retTemp;
//
//    // parametry NTC i dzielnika (typowe: NTC 10k B3950 + rezystor 10k)
//    float R_FIXED = 10000.0f;   // ohm
//    float R0      = 10000.0f;   // ohm @ 25°C
//    float BETA    = 3950.0f;    // K
//    float T0      = 25.0f;      // °C
//
//    float adcMax = (float)ADC_SCALE;
////    float adc    = (float)adc_Raw[devicePosInBuffer_];
//
//	float adc    = (float)GetADCDataChannel(devicePosInBuffer_);
//
//	// proste zabezpieczenie (żeby nie było dzielenia przez 0 / log(0))
//	if (adc < 1.0f)  return -1000.0f;
//	if (adc > adcMax - 1.0f) return -1000.0f;
//
//	// Rntc = R_FIXED * adc / (adcMax - adc)
//	float rNtc = R_FIXED * adc / (adcMax - adc);
//
//	// Beta:
//	// 1/T(K) = 1/T0(K) + (1/B) * ln(R/R0)
//	// na końcu konwersja do °C
//	float t0K = T0 + 273.15f;
//	float invT = (1.0f / t0K) + (1.0f / BETA) * logf(rNtc / R0);
//	float tK = 1.0f / invT;
//
//	retTemp = tK - 273.15f; // wynik w °C
//	return retTemp;
//
//}

// --------------------- DS18B20 ---------------------

DS18B20Sensor::DS18B20Sensor(GPIO_TypeDef* dqPort, uint16_t dqPin, TIM_HandleTypeDef* htim)
: dqPort_(dqPort), dqPin_(dqPin), htim_(htim)
{
    // Timer do µs powinien już działać (HAL_TIM_Base_Start)
    // Linia 1-Wire w spoczynku ma być "puszczona" (input) + podciągnięta rezystorem 4.7k do 3.3V.
    LineInput();
}

void DS18B20Sensor::DelayUs(uint16_t us)
{
    // Założenie: timer tyka 1 MHz => 1 tick = 1 µs
    __HAL_TIM_SET_COUNTER(htim_, 0);
    while (__HAL_TIM_GET_COUNTER(htim_) < us) { }
}

void DS18B20Sensor::LineInput()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin  = dqPin_;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // podciąganie masz zewnętrzne 4.7k
    HAL_GPIO_Init(dqPort_, &GPIO_InitStruct);
}

void DS18B20Sensor::LineOutputOD()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = dqPin_;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD; // open-drain
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(dqPort_, &GPIO_InitStruct);
}

void DS18B20Sensor::DriveLow()
{
    LineOutputOD();
    HAL_GPIO_WritePin(dqPort_, dqPin_, GPIO_PIN_RESET);
}

void DS18B20Sensor::ReleaseLine()
{
    // Zwalniamy magistralę: przejście w input (rezystor podciągnie do '1')
    LineInput();
}

uint8_t DS18B20Sensor::ReadLine()
{
    return (HAL_GPIO_ReadPin(dqPort_, dqPin_) == GPIO_PIN_SET) ? 1u : 0u;
}

bool DS18B20Sensor::ResetPulse()
{
    // Master reset: 480 µs low, potem presence detect
    DriveLow();
    DelayUs(480);

    ReleaseLine();
    DelayUs(70);

    // Presence: slave ściąga do 0
    uint8_t presence = (ReadLine() == 0u) ? 1u : 0u;

    DelayUs(410);
    return (presence == 1u);
}

void DS18B20Sensor::WriteBit(uint8_t bit)
{
    // Write 1: low ~6µs, release do końca slotu
    // Write 0: low ~60µs
    DriveLow();
    if (bit) {
        DelayUs(6);
        ReleaseLine();
        DelayUs(64);
    } else {
        DelayUs(60);
        ReleaseLine();
        DelayUs(10);
    }
}

uint8_t DS18B20Sensor::ReadBit()
{
    // Read: low ~6µs, release, sample po ~9µs
    uint8_t bit = 0;
    DriveLow();
    DelayUs(6);
    ReleaseLine();
    DelayUs(9);
    bit = ReadLine();
    DelayUs(55);
    return bit;
}

void DS18B20Sensor::WriteByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++) {
        WriteBit(data & 0x01u);
        data >>= 1;
    }
}

// Poprawna, czytelna wersja ReadByte:
 uint8_t DS18B20Sensor::ReadByte()
 {
     uint8_t data = 0;
     for (uint8_t i = 0; i < 8; i++) {
         data |= (ReadBit() << i);
     }
     return data;
 }

bool DS18B20Sensor::StartConversion()
{
    if (!ResetPulse()) return false;
    WriteByte(CMD_SKIP_ROM);
    WriteByte(CMD_CONVERT_T);
    return true;
}

bool DS18B20Sensor::ReadScratchpad(uint8_t sp[9])
{
    if (!ResetPulse()) return false;
    WriteByte(CMD_SKIP_ROM);
    WriteByte(CMD_READ_SCRATCH);

    for (uint8_t i = 0; i < 9; i++) {
        sp[i] = ReadByte();
    }

    // CRC (opcjonalnie, ale mocno polecam)
    uint8_t crc = Crc8(sp, 8);
    return (crc == sp[8]);
}

uint8_t DS18B20Sensor::Crc8(const uint8_t* data, uint8_t len)
{
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        uint8_t inbyte = data[i];
        for (uint8_t j = 0; j < 8; j++) {
            uint8_t mix = (crc ^ inbyte) & 0x01u;
            crc >>= 1;
            if (mix) crc ^= 0x8Cu;
            inbyte >>= 1;
        }
    }
    return crc;
}

float DS18B20Sensor::GetTemperature()
{
	if(ignoreReading)
    	return -999.0f;

    // Start konwersji
    if (!StartConversion())
    {
    	return -1000.0f;
    }

    // Czekamy na koniec konwersji:
    // DS18B20 sygnalizuje gotowość przez odczyt '1' w slocie read.
    // W najgorszym razie 12-bit ~750ms. Zrobimy polling co ~5ms.
    for (uint16_t i = 0; i < 200; i++) { // 200 * 5ms = 1000ms
        // ReadBit() w czasie konwersji zwraca 0, po konwersji 1 (dla zasilania normalnego)
        if (ReadBit() == 1u) break;
        HAL_Delay(5);
        if (i == 199)
        {
        	ignoreReading = true;
        	return -1000.0f;
        }
    }

    uint8_t sp[9] = {0};
    if (!ReadScratchpad(sp)) return -1000.0f;

    // Temperatura: 16-bit signed, LSB=1/16°C przy 12-bit
    int16_t raw = (int16_t)((sp[1] << 8) | sp[0]);
    return (float)raw / 16.0f;
}
