/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "LCD_Keypad.h"
#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
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
ADC_HandleTypeDef hadc1;

RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN PV */
uint16_t value;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// -------------------------------------------------------- Variables --------------------------------------------------------
int score = 0;
int highScore = 0;
char scoreText[16] = "";
RTC_TimeTypeDef newTime;
RTC_TimeTypeDef currentTime;
RTC_DateTypeDef date;
uint16_t Milliseconds;
// ---------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------- Menu ----------------------------------------------------------
void menu() {
	lcd_clear();

	sprintf(scoreText, "HScore = %d", highScore);
	int startCol = (16 - strlen(scoreText)) / 2 + 1;
	bool display = true;

	do {
		HAL_Delay(50);
		HAL_ADC_Start(&hadc1);
		if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
			value = HAL_ADC_GetValue(&hadc1);

			HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
			HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN);
			if (currentTime.Seconds > 2) {
				display = !display;
				newTime = currentTime;
				newTime.Seconds = 0;
				newTime.SecondFraction = 0;
				newTime.SubSeconds = 0;
				HAL_RTC_SetTime(&hrtc, &newTime, RTC_FORMAT_BIN);
				lcd_clear();
			}

			lcd_print(1, 4, "Timberman!");
			if (display) {
				lcd_print(2, startCol, scoreText);
			} else if (!display) {
				lcd_print(2, 2, "Start = PRESS");
			}
		}
	} while (value > 4000);
}
// ---------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------- Game ----------------------------------------------------------
void game() {
	lcd_clear();

	score = 0;
	int randomNumber;
	bool isAlive = true;
	int display[2][6];
	bool canGo = true;

	/* ====== GENERATE TREE AT THE START ====== */
	for (int i = 0; i < 5; i++) {
		randomNumber = rand() % 100;
		if (randomNumber % 2 == 0) {
			display[0][i] = 4; // set right part of tree on height i to branch
			display[1][i] = 7; // set left part of tree on height i to log
		} else {
			display[0][i] = 6; // set right part of tree on height i to log
			display[1][i] = 5; // set left part of tree on height i to branch
		}
	}
	display[0][5] = 2; // set right part of tree at the bottom to right man
	display[1][5] = 7; // set left part of tree at the bottom to log

	sprintf(scoreText, "%d", score);
	lcd_print(1, 1, "SCORE:");
	lcd_print(2, 1, scoreText);
	lcd_print(1, 16, "|");
	lcd_print(2, 16, "|");
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 6; j++) {
			lcd_gotoxy(i + 1, j + 10);
			lcd_char_cp(display[i][j]);
		}
	}



	/* ====== MAIN GAME LOOP ====== */
	while (isAlive) {

		HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
		HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN);
		Milliseconds = ((currentTime.SubSeconds)/((float)currentTime.SecondFraction+1) * 100);
		if (currentTime.Seconds > 1 && !canGo) {
			canGo = true;
			newTime = currentTime;
			newTime.Seconds = 0;
			newTime.SecondFraction = 0;
			newTime.SubSeconds = 0;
			HAL_RTC_SetTime(&hrtc, &newTime, RTC_FORMAT_BIN);
		}

		HAL_Delay(50);
		HAL_ADC_Start(&hadc1);
		if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
			value = HAL_ADC_GetValue(&hadc1);

			if (canGo) {
				canGo = false;
				if (value > 700 && value < 820) { // move player to right
					display[0][5] = 0;
					display[1][5] = 7;
					lcd_gotoxy(1, 15);
					lcd_char_cp(display[0][5]);
					lcd_gotoxy(2, 15);
					lcd_char_cp(display[1][5]);
				} else if (value > 1800 && value < 1920) { // move player to left
					display[0][5] = 6;
					display[1][5] = 1;
					lcd_gotoxy(1, 15);
					lcd_char_cp(display[0][5]);
					lcd_gotoxy(2, 15);
					lcd_char_cp(display[1][5]);
				}

				randomNumber = rand() % 100;

				/* ====== LOOSING CONDITION ====== */
				 if (display[0][5] == 0 && display[0][4] == 4) isAlive = false;
				 else if (display[1][5] == 1 && display[1][4] == 5) isAlive = false;
				 else score++;
				/* ====== MOVING BOTTOM PART OF TREE DOWN ====== */
				if (display[0][5] == 0) {
					display[0][5] = 2;
					display[1][5] = display[1][4];
				} else if (display[1][5] == 1) {
					display[1][5] = 3;
					display[0][5] = display[0][4];
				}

				/* ====== MOVING MIDDLE PART OF TREE DOWN ====== */
				for (int i = 4; i > 0; i--) {
					display[0][i] = display[0][i - 1]; // move right part of the tree down by 1
					display[1][i] = display[1][i - 1]; // move left part of the tree down by 1
				}

				/* ====== GENERATING TOP OF THE TREE ====== */
				if (randomNumber % 2 == 0) {
					display[0][0] = 4; // set right part of tree on height i to branch
					display[1][0] = 7; // set left part of tree on height i to log
				} else {
					display[0][0] = 6; // set right part of tree on height i to log
					display[1][0] = 5; // set left part of tree on height i to branch
				}

				/* ====== DISPLAYING EVERYTHING ====== */
				HAL_Delay(300);
				if ((value > 700 && value < 820)
						|| (value > 1800 && value < 1920)) {
					lcd_clear();
					sprintf(scoreText, "%d", score);
					lcd_print(1, 1, "SCORE:");
					lcd_print(2, 1, scoreText);
					lcd_print(1, 16, "|");
					lcd_print(2, 16, "|");
					for (int i = 0; i < 2; i++) {
						for (int j = 0; j < 6; j++) {
							lcd_gotoxy(i + 1, j + 10);
							lcd_char_cp(display[i][j]);
						}
					}
				}
			}
		}
	}
}
// ---------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------- Game over screen ----------------------------------------------------
void gameOver() {
	lcd_clear();

	sprintf(scoreText, "Score = %d", score);
	int startCol = (16 - strlen(scoreText)) / 2 + 1;
	bool display = true;

	do {
		HAL_Delay(50);
		HAL_ADC_Start(&hadc1);
		if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
			value = HAL_ADC_GetValue(&hadc1);

			HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
			HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN);
			if (currentTime.Seconds > 2) {
				display = !display;
				newTime = currentTime;
				newTime.Seconds = 0;
				newTime.SecondFraction = 0;
				newTime.SubSeconds = 0;
				HAL_RTC_SetTime(&hrtc, &newTime, RTC_FORMAT_BIN);
				lcd_clear();
			}

			if (display) {
				lcd_print(1, startCol, scoreText);
			} else if (!display) {
				lcd_print(1, 3, "Game Over :(");
			}
			lcd_print(2, 3, "Menu = PRESS");
		}
	} while (value > 4000);

	if (highScore < score) highScore = score;
}
// ----------------------------------------------------------------------------------------------------------------------------
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	// ----------------------------------------------------- Custom characters variables ----------------------------------------------------
	char rightManHit[] = { 0x00, 0x00, 0x1F, 0x1D, 0x04, 0x04, 0x1F, 0x1F };
	char leftManHit[] = { 0x1F, 0x1F, 0x04, 0x04, 0x1D, 0x1F, 0x00, 0x00 };

	char rightMan[] = { 0x00, 0x00, 0x1F, 0x1D, 0x04, 0x00, 0x1F, 0x1F };
	char leftMan[] = { 0x1F, 0x1F, 0x00, 0x04, 0x1D, 0x1F, 0x00, 0x00 };

	char branchRight[] = { 0x00, 0x02, 0x02, 0x02, 0x02, 0x03, 0x1F, 0x1F };
	char branchLeft[] = { 0x1F, 0x1F, 0x03, 0x02, 0x02, 0x02, 0x02, 0x00 };

	char logRight[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F };
	char logLeft[] = { 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	// ---------------------------------------------------------------------------------------------------------------------------------------

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	// ---------------------------------------------------- Initialization of LCD screen -----------------------------------------------------
	lcd_init(_LCD_4BIT, _LCD_FONT_5x8, _LCD_2LINE);
	// ---------------------------------------------------------------------------------------------------------------------------------------

	KPAD_init(&hadc1);
	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_ADC1_Init();
	MX_RTC_Init();
	/* USER CODE BEGIN 2 */
	// ------------------------------------------------ Assigning custom characters to memory ------------------------------------------------
	lcd_cmd(0x40);
	for (int i = 0; i < 8; i++)
		lcd_char_cp(rightManHit[i]);
	lcd_cmd(0x40 + 8);
	for (int i = 0; i < 8; i++)
		lcd_char_cp(leftManHit[i]);
	lcd_cmd(0x40 + 16);
	for (int i = 0; i < 8; i++)
		lcd_char_cp(rightMan[i]);
	lcd_cmd(0x40 + 24);
	for (int i = 0; i < 8; i++)
		lcd_char_cp(leftMan[i]);
	lcd_cmd(0x40 + 32);
	for (int i = 0; i < 8; i++)
		lcd_char_cp(branchRight[i]);
	lcd_cmd(0x40 + 40);
	for (int i = 0; i < 8; i++)
		lcd_char_cp(branchLeft[i]);
	lcd_cmd(0x40 + 48);
	for (int i = 0; i < 8; i++)
		lcd_char_cp(logRight[i]);
	lcd_cmd(0x40 + 56);
	for (int i = 0; i < 8; i++)
		lcd_char_cp(logLeft[i]);

	lcd_cmd(0x80);
	// ---------------------------------------------------------------------------------------------------------------------------------------

	/*lcd_gotoxy(1, 1);
	 lcd_char_cp(0); // right man hit

	 lcd_gotoxy(2, 1);
	 lcd_char_cp(1); // left  man hit

	 lcd_gotoxy(1, 2);
	 lcd_char_cp(2); // right man

	 lcd_gotoxy(2, 2);
	 lcd_char_cp(3); // left  man

	 lcd_gotoxy(1, 3);
	 lcd_char_cp(4); // branch right

	 lcd_gotoxy(2, 3);
	 lcd_char_cp(5); // branch left

	 lcd_gotoxy(1, 4);
	 lcd_char_cp(6); // log right

	 lcd_gotoxy(2, 4);
	 lcd_char_cp(7);*/// log left
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		menu();
		game();
		gameOver();
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI
			| RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = 0;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 16;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV8;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_MultiModeTypeDef multimode = { 0 };
	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Common config
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc1.Init.LowPowerAutoWait = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	hadc1.Init.OversamplingMode = DISABLE;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure the ADC multi-mode
	 */
	multimode.Mode = ADC_MODE_INDEPENDENT;
	if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_5;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void) {

	/* USER CODE BEGIN RTC_Init 0 */

	/* USER CODE END RTC_Init 0 */

	RTC_TimeTypeDef sTime = { 0 };
	RTC_DateTypeDef sDate = { 0 };
	RTC_AlarmTypeDef sAlarm = { 0 };

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */

	/** Initialize RTC Only
	 */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if (HAL_RTC_Init(&hrtc) != HAL_OK) {
		Error_Handler();
	}

	/* USER CODE BEGIN Check_RTC_BKUP */

	/* USER CODE END Check_RTC_BKUP */

	/** Initialize RTC and set the Time and Date
	 */
	sTime.Hours = 0x0;
	sTime.Minutes = 0x0;
	sTime.Seconds = 0x0;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK) {
		Error_Handler();
	}
	sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	sDate.Month = RTC_MONTH_JANUARY;
	sDate.Date = 0x1;
	sDate.Year = 0x0;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK) {
		Error_Handler();
	}

	/** Enable the Alarm A
	 */
	sAlarm.AlarmTime.Hours = 0x0;
	sAlarm.AlarmTime.Minutes = 0x0;
	sAlarm.AlarmTime.Seconds = 0x0;
	sAlarm.AlarmTime.SubSeconds = 0x0;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS
			| RTC_ALARMMASK_MINUTES;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	sAlarm.AlarmDateWeekDay = 0x1;
	sAlarm.Alarm = RTC_ALARM_A;
	if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN RTC_Init 2 */

	/* USER CODE END RTC_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LCD_D6_Pin | LCD_D5_Pin | LCD_D4_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, LCD_D7_Pin | LCD_RS_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : USER_BUTTON_Pin */
	GPIO_InitStruct.Pin = USER_BUTTON_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LCD_D6_Pin LCD_D5_Pin LCD_D4_Pin */
	GPIO_InitStruct.Pin = LCD_D6_Pin | LCD_D5_Pin | LCD_D4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : LCD_EN_Pin */
	GPIO_InitStruct.Pin = LCD_EN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LCD_EN_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LCD_D7_Pin LCD_RS_Pin */
	GPIO_InitStruct.Pin = LCD_D7_Pin | LCD_RS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
