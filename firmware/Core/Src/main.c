/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "UTC.h"
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
#define DEBUG 0
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
RTC_TimeTypeDef myTime;
RTC_DateTypeDef myDate;
bool Is12HourEnabled = false;
bool IsOneHourAdd = false;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define BTN_PIN			GPIO_PIN_0
#define BTN_PORT		GPIOC
typedef struct
{
    union {
        struct {
            uint16_t min_d0:10;
            uint16_t min_d1:10;

            uint16_t hour_d0:10;
            uint16_t hour_d1:10;

        } time_parts;
        uint8_t all_time[5];
    };
} TimeStruct;

TimeOffset utc_offset ={0};
uint8_t UTC_Constant = 0;

int8_t status = 0;
uint32_t ms_start;
volatile int NumPress = 0;
uint32_t button_press_time = 0;
uint32_t button_release_time = 0;
uint32_t last_press_time = 0;
uint8_t press_count = 0;
uint8_t button_state = 0;
uint8_t gpsCheck = 0;
uint8_t lastGpsCheck = 0;
uint8_t long_press_detected = 0;
bool Format24Flag = true;
bool DSTFlag = false;
bool isDMA = false;
bool inMenu = false;
int rtc_stage = -1;

#define BUFF_SIZE 100
char gps_buff[BUFF_SIZE];



void Update_Changes(int btn_status)
{
#if DEBUG==1
	if(btn_status == 1)			{	printf("Button Pressed: Once \r\n"); }
	else if(btn_status == 2)	{	printf("Button Pressed: Twice \r\n");}
	else if(btn_status == 3)	{	printf("Button Pressed: for 5 seconds\r\n");}
#endif

	if(btn_status == 1){
		Is12HourEnabled = !Is12HourEnabled;
	}
	else if(btn_status == 2){
		IsOneHourAdd = !IsOneHourAdd;
		if(IsOneHourAdd == true){
			HAL_RTC_DST_Add1Hour(&hrtc);
		}else{
			HAL_RTC_DST_Sub1Hour(&hrtc);
		}
	}
	else if(btn_status == 3){
		uint8_t utc_set_state = 0, button_state;
		uint32_t utc_ms_count = 0;
		uint8_t menuOpen = 0;
		inMenu = true;

		HAL_TIM_Base_Stop_IT(&htim6);

		while(utc_set_state < 10){
			switch(utc_set_state){
				case 0:
					HAL_TIM_Base_Stop_IT(&htim6);
					__HAL_TIM_SET_COUNTER(&htim6, 0);
					utc_ms_count = HAL_GetTick();
					utc_set_state = 1;
					Show_UTC(UTC_Constant);
					break;

				case 1:
					button_state = HAL_GPIO_ReadPin(BTN_PORT, BTN_PIN);
					if(button_state == GPIO_PIN_RESET){
						if(menuOpen == 1) {
							if(UTC_Constant >= 38)	UTC_Constant = 1;
							else					UTC_Constant++;
						}
						HAL_Delay(100);
						#if DEBUG == 1
						//printf("new utc= %d \r\n", UTC_Constant);
						#endif
						utc_ms_count = HAL_GetTick();
						Show_UTC(UTC_Constant);
						menuOpen = 1;
						while(button_state == HAL_GPIO_ReadPin(BTN_PORT, BTN_PIN));
					}

					if(HAL_GetTick()-utc_ms_count > 5000){
						utc_set_state = 2;
						#if DEBUG == 1
						printf("utc success: %d \r\n",UTC_Constant);
						#endif
						memset(gps_buff, 0, BUFF_SIZE);
					}
					break;
				case 2:
					Read_GPS(gps_buff, BUFF_SIZE);

					if(strstr(gps_buff, "$GNRMC") != NULL){
						Parse_GPGGA(gps_buff);
						utc_set_state = 20;
					}

				    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, UTC_Constant);

					HAL_TIM_Base_Start_IT(&htim6);
					inMenu = false;
					break;

				default: break;
			}

		}
	}
}


void Show_UTC(uint8_t utc_num)
{
	DisplayDigits(0, 0, 0, 0, utc_num/10, utc_num%10);

#if DEBUG == 1
	//printf("%d, %d, %d, %d, %d\r\n", test[4], test[3],test[2],test[1],test[0]);
#endif
}

void Show_Time(void)
{
	uint8_t new_hour=0;
	HAL_RTC_GetTime(&hrtc, &myTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &myDate, RTC_FORMAT_BIN);

	//myTime.Seconds = 12; myTime.Minutes = 59; myTime.Hours = 06;
	Print_UTC_Time(myTime.Hours, myTime.Minutes, myTime.Seconds);

	new_hour = myTime.Hours;
	if(Is12HourEnabled){
		if(new_hour > 12)
			new_hour = new_hour - 12;
		 if(new_hour == 0)
			new_hour = 12;
	}

	DisplayDigits(new_hour/10, new_hour%10, myTime.Minutes/10, myTime.Minutes%10, myTime.Seconds/10, myTime.Seconds%10);
}

void DisplayDigits(uint8_t hour1, uint8_t hour2, uint8_t min3, uint8_t min4, uint8_t sec5, uint8_t sec6) {
	static uint8_t shiftRegisters[8] = {0};
	uint16_t dig1, dig2, dig3, dig4, dig5, dig6;

	dig1 = 1U << sec6;
	dig2 = 1U << sec5;
	dig3 = 1U << min4;
	dig4 = 1U << min3;
	dig5 = 1U << hour2;
	dig6 = 1U << hour1;

	shiftRegisters[0] = dig1 & 0xFF;
	shiftRegisters[1] = ((dig2 & 0x3F) << 2) | ((dig1 >> 8) & 0x03);
	shiftRegisters[2] = ((dig3 & 0x0F) << 4) | ((dig2 >> 6) & 0x0F);
	shiftRegisters[3] = ((dig4 & 0x03) << 6) | ((dig3 >> 4) & 0x3F);
	shiftRegisters[4] = ((dig4 >> 2) & 0xFF);
	shiftRegisters[5] = dig5 & 0xFF;
	shiftRegisters[6] = ((dig6 & 0x3F) << 2) | ((dig5 >> 8) & 0x03);
	shiftRegisters[7] = ((dig6 >> 6) & 0x0F);

	HAL_SPI_Transmit_DMA(&hspi1, shiftRegisters, 8);
}

uint32_t f_write = 0xBABADEAD;
uint32_t f_read = 0;

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
  MX_DMA_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_TIM6_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
#if DEBUG == 1
   printf(">> Program starts... \r\n");
#endif
   printf(">> Program starts... \r\n");

   if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x00) {
	   UTC_Constant = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
   }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  switch(rtc_stage){
		  case -1:
			 // Update_Changes(3);
			  Button_Init();
			  rtc_stage = 0;
			  break;
	  	  case 0:
	  		  Read_GPS(gps_buff, BUFF_SIZE);

			  if(strstr(gps_buff, "$GNRMC") != NULL){
				  printf(gps_buff);
				  Parse_GPGGA(gps_buff);
			  }

		  	  rtc_stage = 1;
	  		  break;

	  	  case 1:
	  		  HAL_TIM_Base_Start_IT(&htim6);
	  		  HAL_TIM_Base_Start_IT(&htim7);

	  		  rtc_stage = 2;
	  		  break;

	  	  case 2:
#if DEBUG == 1
	  		  Read_GPS(gps_buff, BUFF_SIZE);

			  if(strstr(gps_buff, "$GNRMC") != NULL){
				  printf(gps_buff);
				  Parse_GPGGA(gps_buff);
			  }
#endif

			  /*if(myTime.Seconds == 00) {
				  lastGpsCheck = gpsCheck;

				  uint8_t tubeNum[10] = {1, 2, 5, 4, 8, 3, 9, 0, 7, 6};

				  for(int i = 0; i < 10; i++) {
					  DisplayDigits(tubeNum[i], tubeNum[i], tubeNum[i], tubeNum[i], tubeNum[i], tubeNum[i]);
					  HAL_Delay(100);
				  }
			  }*/

	  		  process_button_events();
	  		  Update_Changes(status);
	  		  status = -1;

	  		  break;

	  	  default: break;
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void Read_GPS(char *buff, uint8_t size)
{
	uint8_t idx = 0;

	MX_USART1_UART_Init();
	USART1->CR1 |= USART_CR1_RE;
	while(idx < size)
	{
		if(USART1->ISR & USART_ISR_RXNE_RXFNE){
			buff[idx] = USART1->RDR;
			if(buff[idx] == '\n'){
				buff[++idx] = '\0';
				return;
			}
			idx++;
		}
	}

	USART1->CR1 &= ~USART_CR1_RE;

}

void Parse_GPGGA(char *sentence)
{
	uint8_t sec = 0, min = 0, hour = 0;
	char utc_time[10] = "";
	char gpsParsed[15][10] = { NULL };
	uint8_t i = 0;

	char* token = strtok(sentence, ",");

	do {
		strcpy(gpsParsed[i], token);
		token = strtok(NULL, ",");
		i++;
	} while(token != NULL);

	//token = strtok(NULL, ",");

	snprintf(utc_time, sizeof(utc_time), "%s", gpsParsed[1]);  // Copy the time string

	// Extract hours, minutes, and seconds
	hour = (utc_time[0] - '0') * 10 + (utc_time[1] - '0');
	min = (utc_time[2] - '0') * 10 + (utc_time[3] - '0');
	sec = (utc_time[4] - '0') * 10 + (utc_time[5] - '0');

#if DEBUG == 1
	//Print_UTC_Time(hour, min, sec);
#endif
	//RTC_SetTime(hour, min, sec);

	if(gpsParsed[6][0] == '1' || gpsParsed[6][0] == '2' || gpsParsed[2][0] == 'A' || gpsParsed[1][6] == '.'){
		RTC_SetTime(hour, min, sec);
		RTC_SetDate(24, RTC_MONTH_DECEMBER, 20, RTC_WEEKDAY_SUNDAY);

		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, 1);
	} else {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, 0);
	}
	//Update_Changes(0);*/
}

void Print_UTC_Time(uint8_t hour, uint8_t minute, uint8_t second) {
	char time_str[25];
    snprintf(time_str, sizeof(time_str), "UTC Time: %02d:%02d:%02d \r\n", hour, minute, second);
    HAL_UART_Transmit(&huart2, (uint8_t*)time_str, strlen(time_str), 100);
}

void RTC_SetTime(uint8_t hour, uint8_t min, uint8_t sec)
{
	RTC_TimeTypeDef sTime;
	uint8_t new_hour=0, new_min=0;

	utc_offset = calculate_utc_offset(UTC_Constant);

	if(min + utc_offset.minutes < 0){
		new_min = 60 + min + utc_offset.minutes;
		hour -= 1;
		//printf("min <0: %d \r\n", new_min);
	}
	else if(min + utc_offset.minutes > 59){
		new_min =  min + utc_offset.minutes - 60;
		hour += 1;
		//printf("min >59: %d \r\n", new_min);
	}
	else{
		new_min = min + utc_offset.minutes;
		//printf("min 0-59: %d \r\n", new_min);
	}

	if(hour + utc_offset.hours < 0){
		new_hour = 24 + hour + utc_offset.hours;
		//printf("hour <0: %d \r\n", new_hour);
	}
	else if(hour + utc_offset.hours > 23){
		new_hour = hour + utc_offset.hours - 24;
		//printf("hour >23: %d \r\n", new_hour);
	}
	else{
		new_hour = hour + utc_offset.hours;
		//printf("hour 0-23: %d \r\n", new_hour);
	}

	if(IsOneHourAdd) {
		new_hour += 1;
	}

	sTime.Hours = new_hour;
	sTime.Minutes = new_min;
	sTime.Seconds = sec;
	sTime.SubSeconds = 0;
	sTime.TimeFormat = RTC_HOURFORMAT12_AM;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}

	//Print_UTC_Time(new_hour, new_min, sec);
}

void RTC_SetDate(uint8_t year, uint8_t month, uint8_t date, uint8_t day)
{
	RTC_DateTypeDef sDate = {0};

	sDate.WeekDay = day;
	sDate.Month = month;
	sDate.Date = date;
	sDate.Year = year;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
	//HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, PROTECTION_KEY);  // backup register
}

void Button_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(BTN_PORT, BTN_PIN, GPIO_PIN_RESET);


	GPIO_InitStruct.Pin = BTN_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(BTN_PORT, &GPIO_InitStruct);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM6){
	}

	if(htim->Instance == TIM7) {
		if(gpsCheck == 30) {
			Read_GPS(gps_buff, BUFF_SIZE);

			if(strstr(gps_buff, "$GNRMC") != NULL){
			  printf(gps_buff);
			  Parse_GPGGA(gps_buff);
			}

			gpsCheck = 0;
		} else {
			gpsCheck++;
		}
	}
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
	if(!inMenu) {
		Show_Time();
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
	//HAL_Delay(1);
	for(int i=0; i < 2000; i++);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
}

void process_button_events(void)
{
    //static uint32_t debounce_time = 0;
    static uint8_t pending_single_press = 0;

    // Read the button state
    uint8_t current_state = HAL_GPIO_ReadPin(BTN_PORT, BTN_PIN);

    if (current_state == GPIO_PIN_RESET && button_state == 0) {
        // Button Press Detected
        button_press_time = HAL_GetTick();
        button_state = 1;
        if ((button_press_time - last_press_time) < 1000) {
            press_count++;
        } else {
            press_count = 1;
        }
        last_press_time = button_press_time;
    }

    if (current_state == GPIO_PIN_SET && button_state == 1) {
        // Button Released
        button_release_time = HAL_GetTick();
        button_state = 0;

        if (!long_press_detected) {
            if (press_count == 1) {
                // Mark a pending single press
                pending_single_press = 1;
            } else if (press_count == 2) {
                // Handle double press immediately
                //double_press_event();
            	status = 2;
                press_count = 0;
                pending_single_press = 0; // Clear pending single press
            }
        }
        long_press_detected = 0;
    }

    // Check for long press
    if (button_state == 1 && (HAL_GetTick() - button_press_time) > 3000 && !long_press_detected) {
        long_press_detected = 1;
        //long_press_event();
        status = 3;
        pending_single_press = 0; // Cancel pending single press
    }

    // Handle single press after a delay to confirm no double press
    if (pending_single_press && (HAL_GetTick() - last_press_time) > 300) {
        //single_press_event();
    	status = 1;
        pending_single_press = 0; // Clear pending single press
    }
}


void _putchar(char character)
{
	while(!(USART2->ISR & UART_FLAG_TXE));
	USART2->TDR = character & 0xFF;
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
