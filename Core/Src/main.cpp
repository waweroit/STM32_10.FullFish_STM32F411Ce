/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#include "adc.h"
//#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"
#include "tim.h"
#include "rtc.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "EnviromentConfiguration.hpp"
#include "EncoderController.hpp"
#include "MainMenu.hpp"
#include "BuildMenu.hpp"
#include "liquidcrystal_i2c.h"
#include "buttonsSetup.h"
//#include "ADC_Configuration.hpp"
#include "TemperatureSensors.hpp"
#include "RTCController.hpp"
#include "AT24C32_ConfigStorage.hpp"
#include "AlarmRTC.hpp"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

MenuState menu;

IEncoder *encoder = nullptr;
ITemperatureSensor *tempSensors = nullptr;
IRtc *rtc = nullptr;
IConfigStorage *AT24C32_Epprom = nullptr;
IAlarmRTC *IalarmRTC = nullptr;

ButtonConfig EncoderButtonConfig;

volatile float SensorTemp = 0;

DeviceConfig devConfig;
AquaConfig   aquaConfig;

volatile BacklightStatus HD16x2Backlight = LCD_BACKLIGHT_OFF;
volatile uint32_t TickCount = 1;
volatile uint32_t checkActionEverySec = 10;
volatile uint32_t ActionTick = checkActionEverySec;
volatile uint32_t synchronizeRTCTime = 7200;// sec
volatile uint32_t RtcTick = synchronizeRTCTime;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */


/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
//  MX_DMA_Init();
  MX_TIM4_Init();
  MX_I2C2_Init();
//  MX_ADC1_Init();
  MX_RTC_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();

  /* USER CODE BEGIN 2 */

  //RTC_InitOnce();
  RtcController rtcTime(&hi2c2, 0x68);
  rtcTime.SynchronizeRTC_MCU(&hrtc);
  rtc = &rtcTime;

  AT24C32_ConfigStorage store(&hi2c2, 0x57);
  AT24C32_Epprom = &store;

  if (store.Load(&devConfig, &aquaConfig) != HAL_OK) {
      // brak poprawnych danych -> zostaw domyślne i zapisz
      store.Save(&devConfig, &aquaConfig);
  }
  aquaConfig.lightConfig.gpio.Enabled = 0; // reset
  aquaConfig.lightConfig.gpio.port = LED_01_GPIO_Port;
  aquaConfig.lightConfig.gpio.pin = LED_01_Pin;

  aquaConfig.heaterConfig.gpio.Enabled = 0; // reset
  aquaConfig.heaterConfig.gpio.port = LED_01_GPIO_Port;
  aquaConfig.heaterConfig.gpio.pin = LED_01_Pin;

//  aquaConfig.waterConfig.gpio.port = WaterSensor_GPIO_Port;
//  aquaConfig.waterConfig.gpio.pin = WaterSensor_GPIO_Pin;

  alarmRTC alarmData;
  alarmData.SetConfiguration(&aquaConfig);
  IalarmRTC = &alarmData;


  if (HAL_TIM_Base_Start_IT(&htim10) != HAL_OK)
  {
    /* Starting Error */
    Error_Handler();
  }



  EncoderDev encoder1(&htim4, TIM_CHANNEL_ALL);
  encoder = &encoder1;

  EncoderButtonConfig.GPIOx = EncoderButton_GPIO_Port;
  EncoderButtonConfig.GPIO_Pin = EncoderButton_Pin;

//  uint8_t list[16];
//  memset(list, 0, sizeof(list));
//  uint8_t n = I2C_FindDevices(&hi2c2, list, sizeof(list));

  Initialize_HD44780(&hi2c2);
  HD16x2Backlight =LCD_BACKLIGHT_ON;
  HD44780_NoCursor();


  encoderShift shift = encoder1.GetEncoderShift();


  menu.current  = MenuRoot();   // root z BuildMenu.cpp
  menu.selected = 0;
  menu.top      = 0;
  BuildMenu();
  Render(menu);

//  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_Raw, ADC_BUF_SIZE);
//  InitADC(&hadc1);
//  NTCSensor ntcSensor(&hadc1, 0);
//  tempSensors = &ntcSensor;


  // gdzieś w init po starcie timera:
  HAL_TIM_Base_Start(&htim11);
  DS18B20Sensor DS18B20TempSensor(DS18B20_Output_GPIO_Port, DS18B20_Output_Pin, &htim11);
  tempSensors = &DS18B20TempSensor;

  uint32_t checkTemperatureEveryMS = 10000;
  uint32_t checkTemperature_FromTime = 0;
  uint32_t TimeNOW = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  TimeNOW = HAL_GetTick();
	  shift = encoder1.GetEncoderShift();

		if ((TimeNOW - checkTemperature_FromTime) >= checkTemperatureEveryMS) {
			SensorTemp = tempSensors->GetTemperature() + aquaConfig.heaterConfig.AdjTempSensor;
			checkTemperature_FromTime = TimeNOW;
		}

	  if(shift != NoChange)
	  {
			if (HD16x2Backlight == LCD_BACKLIGHT_OFF) {
				HD16x2Backlight = LCD_BACKLIGHT_ON;
				HD44780_Backlight();
			}
			TickCount = 1;

			switch (shift) {
			case UpIncrement:
				MoveUp(menu);
				Render(menu);
				break;

			case DownIncrement:
				MoveDown(menu);
				Render(menu);
				break;
			}
	  }

	  switch(getButtonEvent(&EncoderButtonConfig))
		{
			case NO_PRESS :{} break ;
			case SINGLE_PRESS :
			{
				//HD44780_Clear();
				if(HD16x2Backlight == LCD_BACKLIGHT_OFF)
				{
					HD16x2Backlight = LCD_BACKLIGHT_ON;
					HD44780_Backlight();
				}
				TickCount = 1;
//				HD44780_SetCursor(0, 0);
//				HD44780_PrintStr("single Press");
				Enter(menu);
				Render(menu);


			} break ;
			case LONG_PRESS :
			{
				//HD44780_Clear();
				if(HD16x2Backlight == LCD_BACKLIGHT_OFF)
				{
					HD16x2Backlight = LCD_BACKLIGHT_ON;
					HD44780_Backlight();
				}
				TickCount = 1;

//				HD44780_SetCursor(0, 0);
//				HD44780_PrintStr("long Press");
				// do stuff

			} break ;
			case DOUBLE_PRESS :
			{
				//HD44780_Clear();
				if(HD16x2Backlight != 1)
				{
					HD16x2Backlight = LCD_BACKLIGHT_ON;
					HD44780_Backlight();
				}
				TickCount = 1;
//				HD44780_SetCursor(0, 0);
//				HD44780_PrintStr("double Press");
				Back(menu);
				Render(menu);


			} break ;
		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  if(ActionTick == 0)
	  {
//			if(IalarmRTC != nullptr)
//				IalarmRTC->ActionAlarms();
			   ActionTick = checkActionEverySec;
	  }

		if(TickCount >= devConfig.displayConfig.displayOnTime)
		{
			TickCount = 1;
			if(HD16x2Backlight == LCD_BACKLIGHT_ON)
			{
				HD16x2Backlight = LCD_BACKLIGHT_OFF;
				HD44780_NoBacklight();
			}
		}

		if (RtcTick == 0) {
			rtcTime.SynchronizeRTC_MCU(&hrtc);
			RtcTick = synchronizeRTCTime;
		}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM10)
	{
		if(ActionTick > 0)
			ActionTick--;

		TickCount++;

		if(RtcTick > 0)
			RtcTick--;

		if(IalarmRTC != nullptr)
			IalarmRTC->ActionAlarms();
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == WaterSensor_Pin)
	{
		HAL_GPIO_TogglePin(LED_01_GPIO_Port, LED_01_Pin);
	}

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
