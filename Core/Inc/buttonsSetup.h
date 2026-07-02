/*
 * buttonsSetup.h
 *
 *  Created on: Jan 29, 2024
 *      Author: wawer
 */
#pragma once

#ifndef INC_BUTTONSSETUP_H_
#define INC_BUTTONSSETUP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "main.h"
//#include "stm32f411xe.h"
//#include "stm32f4xx_hal_gpio.h"

#define DOUBLE_GAP_MILLIS_MAX 500
#define LONG_MILLIS_MIN 800
#define DEBOUNCE_MILLIS 30

typedef enum
{
    NO_PRESS,
    SINGLE_PRESS,
    LONG_PRESS,
    DOUBLE_PRESS
} eButtonEvent;

typedef struct
{
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
	uint32_t button_down_ts;
	uint32_t button_up_ts;
	uint32_t buttonstate_ts;
	bool double_pending ;
	bool long_press_pending ;
	bool button_down;
	uint32_t now;

} ButtonConfig;


eButtonEvent getButtonEvent(ButtonConfig *button);
#ifdef __cplusplus
}
#endif
#endif /* INC_BUTTONSSETUP_H_ */
