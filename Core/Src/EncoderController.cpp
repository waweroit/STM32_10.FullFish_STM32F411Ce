/*
 * EncoderController.cpp
 *
 *  Created on: Dec 4, 2025
 *      Author: wawer
 */

#include "EncoderController.hpp"

EncoderDev::EncoderDev(){}

EncoderDev::EncoderDev(TIM_HandleTypeDef* timer, uint32_t channel): timer_(timer),channel_(channel)
{
	HAL_TIM_Encoder_Start(timer_, channel_);
	CounterARR_= timer_->Instance->ARR +1;
	encoderCountsPerRevolution = CounterARR_ / CountsPerStep;
	lastPosition = encoderCountsPerRevolution -1;
}

//void EncoderDev::Init(TIM_HandleTypeDef* timer, uint32_t channel)
//{
//	HAL_TIM_Encoder_Start(timer_, channel_);
//	CounterARR_= timer_->Instance->ARR +1;
//	encoderCountsPerRevolution = CounterARR_ / CountsPerStep;
//	lastPosition = encoderCountsPerRevolution -1;
//	timer_= timer;
//	channel_ = channel;
//}

uint16_t EncoderDev::GetEncoderPulseCount()
{
	// return static_cast<uint16_t>(timer_->Instance->CNT);
	uint16_t ret = static_cast<uint16_t>(__HAL_TIM_GET_COUNTER(timer_));
	return ret;
}

uint16_t EncoderDev::GetEncoderPosition() {
	// UWAGA dla przypadku ARR 403 / 4 zwraca 100
	return EncoderDev::GetEncoderPulseCount() / CountsPerStep;
}

encoderShift EncoderDev::GetEncoderShift(){

	//static uint16_t oldPosition = 0;
	uint16_t nowPosition = GetEncoderPosition();

	encoderShift result = NoChange;

	if(oldPosition != nowPosition)
	{
        if (oldPosition == lastPosition && nowPosition == 0)
            result = UpIncrement;       // 100 -> 0
        else if (oldPosition == 0 && nowPosition == lastPosition)
            result = DownIncrement;     // 0 -> 100
        else if (nowPosition > oldPosition)
            result = UpIncrement;
        else
            result = DownIncrement;
	}

	oldPosition = nowPosition;
	return result;
}

void EncoderDev::ResetEncoderPosition(){
	__HAL_TIM_SET_COUNTER(timer_, 0);
}

void EncoderDev::SetEncoderPosition(uint16_t set){
	oldPosition = set;
	__HAL_TIM_SET_COUNTER(timer_, (uint32_t)(set * CountsPerStep));
}

//void EncoderDev::ChaingeEncoderCounterMode(CounterMode cm) {
//	if (cm == up)
//		timer_->Init.CounterMode = TIM_COUNTERMODE_UP;
//	else if (cm == down)
//		timer_->Init.CounterMode = TIM_COUNTERMODE_DOWN;
//}
