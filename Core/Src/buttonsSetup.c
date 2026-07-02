/*
 * buttonsSetup.c
 *
 *  Created on: Jan 29, 2024
 *      Author: wawer
 */

#include "buttonsSetup.h"

//ButtonConfig buttonList[2];
//
//void MakeListOfButtons(ButtonConfig *addButtonHandle)
//{
//	buttonList;
//}


uint8_t BUTTON_State(ButtonConfig *button)
{
	uint8_t buttonDownState = 0;
	//GPIO_PinState state1 = HAL_GPIO_ReadPin( GPIOx, GPIO_Pin );
	GPIO_PinState state1 = HAL_GPIO_ReadPin(button->GPIOx, button->GPIO_Pin );
	if(state1 == GPIO_PIN_RESET)
	{
		buttonDownState = 1;
	}

    button->now = HAL_GetTick();


    //    if( now - buttonstate_ts > DEBOUNCE_MILLIS )
        if( (button->now - button->buttonstate_ts) > DEBOUNCE_MILLIS )
        {
			button->buttonstate_ts = button->now;
			GPIO_PinState state2 = HAL_GPIO_ReadPin(button->GPIOx, button->GPIO_Pin );
			if(state2 == GPIO_PIN_RESET)
			{
				buttonDownState = 1;
			}
			else
				button->buttonstate_ts = 0;
        }
        else if( (button->now - button->buttonstate_ts) < DEBOUNCE_MILLIS )
        {
        	buttonDownState = 1;
        }


    return buttonDownState ;
}

//eButtonEvent getButtonEvent(ButtonConfig *button, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
eButtonEvent getButtonEvent(ButtonConfig *button)
{

    eButtonEvent button_event = NO_PRESS ;
    //uint32_t now = Get_TenthTick() ;
    button->now = HAL_GetTick() ;
    //now = HAL_GetTick() ;

    // If state changed...
    //if( button_down != BUTTON_State(GPIOx ,GPIO_Pin ) )
    if( button->button_down != BUTTON_State(button))
    {
    	button->button_down = !button->button_down ;
        if( button->button_down )
        {
            // Timestamp button-down
            //button_down_ts = now ;
        	button->button_down_ts = button->now;
        }
        else
        {
            // Timestamp button-up
        	button->button_up_ts = button->now;

            // If double decision pending...
            if( button->double_pending )
            {
                button_event = DOUBLE_PRESS ;
                button->double_pending = false ;


            }
            else
            {
            	button->double_pending = true ;
            }

            // Cancel any long press pending
            button->long_press_pending = false ;
        }
    }

    // If button-up and double-press gap time expired, it was a single press
    if( !button->button_down && button->double_pending && button->now - button->button_up_ts > DOUBLE_GAP_MILLIS_MAX )
    {
    	button->double_pending = false ;
        button_event = SINGLE_PRESS ;


    }
    // else if button-down for long-press...
    else if( !button->long_press_pending && button->button_down && button->now - button->button_down_ts > LONG_MILLIS_MIN )
    {
        button_event = LONG_PRESS ;
        button->long_press_pending = false ;
        button->double_pending = false ;
        button->button_down = false;
    }

    return button_event ;
}
