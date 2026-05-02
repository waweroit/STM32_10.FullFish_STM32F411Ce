/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
volatile W25Q_HandleTypeDef w25q;

typedef struct {
	uint32_t magic;
	int KeyPressedCount;
	GPIO_PinState LowLevelWasReached;
} DataFlash;

volatile DataFlash dane;
volatile uint32_t flash_addr = 0x000100;


volatile GPIO_PinState LowLevelWasReachedPrev = GPIO_PIN_RESET; // stan niski (0)
volatile bool DisableACRelay = false;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

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

//	typedef struct {
//	    uint32_t addr;
//	    uint16_t len;
//	    char name[12];
//	} FlashMapEntry;
//
//	FlashMapEntry variables[] = {
//	    {0x000100, 8, "liczba1"},
//	    {0x000108, 50, "tekst1"},
//	    {0x000200, 8, "liczba2"},
//	};





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
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  DelayInit();


  uint32_t TimeNOW = 0;
  int GetReadMesurment_TimeLenghtDefault = 1000; // 5 sek
  uint32_t GetReadMesurment_FromTime = 0;

  Send("Welcome waweroIT\r\n");
  HAL_Delay(1000);


	// --- Inicjalizacja obsługi W25Q64 ---

	W25Q_Init((W25Q_HandleTypeDef*)&w25q, &hspi1, SPI1_CS_GPIO_Port, SPI1_CS_Pin); // Podaj port i pin CS

	Send("Odczyt z pamieci flash na starcie...\r\n");
	if (!W25Q_Read((W25Q_HandleTypeDef*)&w25q, flash_addr, (uint8_t*)&dane, sizeof(dane)))
	{
		Send("Blad odczytu na starcie!\r\n");
	}else
		Send("Odczyt na starcie ok.\r\n");

	if  (dane.magic != FLASH_MAGIC)
	{
	    // To pierwsze uruchomienie lub dane niewazne
	    dane.magic = FLASH_MAGIC;
	    dane.KeyPressedCount = 0;                // Ustaw wartosci domyslne!
	    dane.LowLevelWasReached = GPIO_PIN_RESET ; // Domyslna flaga!

	    // Skasuj i zapisz domyslne wartosci do flash
	    W25Q_EraseSector((W25Q_HandleTypeDef*)&w25q, flash_addr);
	    if (!W25Q_Write((W25Q_HandleTypeDef*)&w25q, flash_addr, (uint8_t*)&dane, sizeof(dane)))
	    {
	    	Send("Blad zapisu!\r\n");
	    }

	}
	else
	{
		Send("Magic data ok.\r\n");
	}

	LowLevelWasReachedPrev = dane.LowLevelWasReached;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	snprintf(txBuffer, sizeof(txBuffer), "Odczytane dane.KeyPressedCount: %d\r\n", dane.KeyPressedCount);
	Send(txBuffer);

	snprintf(txBuffer, sizeof(txBuffer), "Odczytany dane.LowLevelWasReached: %d\r\n", (int)dane.LowLevelWasReached);
	Send(txBuffer);

	if(dane.LowLevelWasReached == GPIO_PIN_SET)
	{
	  // Disable DC PUMP
		HAL_GPIO_WritePin(LED_01_GPIO_Port, LED_01_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ACRelay_GPIO_Port, ACRelay_Pin, GPIO_PIN_SET);

	  Send("AC Relay: Disable \r\n");
	  DisableACRelay = true;
	}
	else
	{
		  // Disable DC PUMP
			HAL_GPIO_WritePin(LED_01_GPIO_Port, LED_01_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(ACRelay_GPIO_Port, ACRelay_Pin, GPIO_PIN_RESET);

		  Send("AC Relay: Enable \r\n");
		  DisableACRelay = false;
	}

  while (1)
  {
	TimeNOW = HAL_GetTick();

	if(((uint32_t)TimeNOW - GetReadMesurment_FromTime) >= GetReadMesurment_TimeLenghtDefault)
	{
		GetReadMesurment_FromTime = TimeNOW;
		dane.LowLevelWasReached = HAL_GPIO_ReadPin(FloatSensor_GPIO_Port, FloatSensor_Pin);

		if(dane.LowLevelWasReached != LowLevelWasReachedPrev)
		{
			LowLevelWasReachedPrev = dane.LowLevelWasReached;

			if(LowLevelWasReachedPrev == GPIO_PIN_SET && DisableACRelay == false) // set czyli 1 - niski stan wody
			{
				HAL_GPIO_WritePin(LED_01_GPIO_Port, LED_01_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(ACRelay_GPIO_Port, ACRelay_Pin, GPIO_PIN_SET);
				Send("Stan wody: NISKI \r\n");
				Send("AC Relay: Disable \r\n");
				DisableACRelay = true;

			    if (!W25Q_Write((W25Q_HandleTypeDef*)&w25q, flash_addr, (uint8_t*)&dane, sizeof(dane)))
			    {
			    	Send("Blad zapisu!\r\n");
			    }
			}
			else if(LowLevelWasReachedPrev == GPIO_PIN_SET) // reset  czyli 1 - wysoki stan wody
			{
				Send("Stan wody: NISKI \r\n");
			}
			else if(LowLevelWasReachedPrev == GPIO_PIN_RESET) // reset  czyli 0 - wysoki stan wody
			{
				Send("Stan wody: WYSOKI \r\n");
			}
		}

	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 144;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {

    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) // Sprawdź, czy to Twój UART
    {
        isTransmissionComplete = true;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == UserKey_Pin)
	{
		dane.KeyPressedCount++;
		dane.LowLevelWasReached = GPIO_PIN_RESET;
		LowLevelWasReachedPrev = dane.LowLevelWasReached;
		Send("Key pressed\r\n");
		DisableACRelay = false;
		Send("FloatSensor Reset !\r\n");
		HAL_GPIO_WritePin(LED_01_GPIO_Port, LED_01_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ACRelay_GPIO_Port, ACRelay_Pin, GPIO_PIN_RESET);

	    if (!W25Q_Write((W25Q_HandleTypeDef*)&w25q, flash_addr, (uint8_t*)&dane, sizeof(dane)))
	    {
	    	Send("Blad zapisu!\r\n");
	    }
		//ToggleLedOnBlackPill();
	}

//	if(GPIO_Pin == FloatSensor_Pin)
//	{
//
//		Send("FloatSensor Low Level !\r\n");
//
//		if(dane.LowLevelWasReached == false)
//		{
//			dane.LowLevelWasReached = true;
//
//			if (!W25Q_Write((W25Q_HandleTypeDef*)&w25q, flash_addr, (uint8_t*)&dane, sizeof(dane)))
//			{
//				Send("Blad zapisu!\r\n");
//			}
//		}
//
//		ToggleLedOnBlackPill();
//	}
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
