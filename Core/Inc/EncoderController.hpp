/*
 * EncoderController.hpp
 *
 *  Created on: Dec 4, 2025
 *      Author: wawer
 */

#ifndef INC_ENCODERCONTROLLER_HPP_
#define INC_ENCODERCONTROLLER_HPP_

#include "tim.h"

// odczytywanie zboczy kanalu A B to 4 -standardowo bo po 2 zbocza na kanal
#define CountsPerStep 2

enum encoderShift{
			NoChange =0,
			UpIncrement=1,
			DownIncrement=2};

class IEncoder{
public:
	virtual ~IEncoder() = default;

	virtual uint16_t GetEncoderPulseCount() = 0;
	virtual uint16_t GetEncoderPosition() = 0;
	virtual encoderShift GetEncoderShift() =0;
	virtual void ResetEncoderPosition() = 0;
	virtual void SetEncoderPosition(uint16_t set) = 0;
//	virtual void ChaingeEncoderCounterMode(CounterMode cm)=0;
};

class EncoderDev : public IEncoder {
public:
	EncoderDev();
	EncoderDev(TIM_HandleTypeDef* timer, uint32_t channel);
//	void Init(TIM_HandleTypeDef* timer, uint32_t channel);
	uint16_t GetEncoderPulseCount() override;
	uint16_t GetEncoderPosition() override;
	encoderShift GetEncoderShift() override;
	void ResetEncoderPosition() override;
	void SetEncoderPosition(uint16_t set) override;
//	void ChaingeEncoderCounterMode(CounterMode cm) override;

private:
    TIM_HandleTypeDef* timer_;
    uint32_t channel_;
    uint32_t CounterARR_;
    uint32_t encoderCountsPerRevolution;
    uint32_t lastPosition;
    uint16_t oldPosition = 0;
};



#endif /* INC_ENCODERCONTROLLER_HPP_ */
