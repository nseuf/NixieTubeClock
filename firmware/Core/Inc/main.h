/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "printf_lib/printf.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "FLASH.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void Button_Init(void);
void Show_Time(void);
void process_button_events(void);
void Update_Format(int format);
void Update_DayLightSaving(int dst);
void RTC_SetTime(uint8_t hour, uint8_t min, uint8_t sec);
void RTC_SetDate(uint8_t year, uint8_t month, uint8_t date, uint8_t day);
void DisplayDigits(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dig5, uint8_t dig6);


void Show_UTC(uint8_t utc_num);

void Read_GPS(char *buff, uint8_t size);
void Parse_GPGGA(char *sentence);
void Print_UTC_Time(uint8_t hour, uint8_t minute, uint8_t second);


/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
