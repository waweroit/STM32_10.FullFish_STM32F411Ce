/*
 * TemperatureSensors.hpp
 *
 *  Created on: Dec 17, 2025
 *      Author: wawer
 */

#ifndef INC_TEMPERATURESENSORS_HPP_
#define INC_TEMPERATURESENSORS_HPP_

//#include "ADC_Configuration.hpp"
//#include "dma.h"
//#include "adc.h"
#include "tim.h"

class ITemperatureSensor
{
public:
	virtual ~ITemperatureSensor() = default;
	virtual	float GetTemperature() = 0;
};


// zalozenia
//	3.3V
//	 │
//	[10k]  ← rezystor stały
//	 │──── ADC
//	[NTC 10k]
//	 │
//	 GND
//class NTCSensor : public ITemperatureSensor {
//public:
//	NTCSensor(ADC_HandleTypeDef *hadc, uint16_t devicePosInBuffer);
//	float GetTemperature() override;
//
//private:
//	ADC_HandleTypeDef *hadc_;
//	uint8_t adc_buf_size_;
//	uint16_t devicePosInBuffer_;
//
//    static constexpr float R_FIXED_OHM = 10000.0f;  // rezystor stały 10k
//    static constexpr float R0_OHM      = 10000.0f;  // NTC 10k przy 25°C
//    static constexpr float T0_K        = 298.15f;   // 25°C w kelwinach
//    static constexpr float BETA        = 3950.0f;   // typowo 3950
//};

class DS18B20Sensor : public ITemperatureSensor{
	public:
    // DQ = pin 1-Wire, timer = źródło opóźnień w µs (najlepiej 1 MHz)
    DS18B20Sensor(GPIO_TypeDef* dqPort, uint16_t dqPin, TIM_HandleTypeDef* htim);
	float GetTemperature() override;

private:
    GPIO_TypeDef* dqPort_;
    uint16_t dqPin_;
    TIM_HandleTypeDef* htim_;

    void DelayUs(uint16_t us);

    void LineInput();
    void LineOutputOD();
    void DriveLow();
    void ReleaseLine();
    uint8_t ReadLine();

    bool ResetPulse();
    void WriteBit(uint8_t bit);
    uint8_t ReadBit();
    void WriteByte(uint8_t data);
    uint8_t ReadByte();

    bool StartConversion();
    bool ReadScratchpad(uint8_t sp[9]);
    static uint8_t Crc8(const uint8_t* data, uint8_t len);

    // Komendy
    static constexpr uint8_t CMD_SKIP_ROM     = 0xCC;
    static constexpr uint8_t CMD_CONVERT_T    = 0x44;
    static constexpr uint8_t CMD_READ_SCRATCH = 0xBE;

    bool ignoreReading = false;

};


#endif /* INC_TEMPERATURESENSORS_HPP_ */
