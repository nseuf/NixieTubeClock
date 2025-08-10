#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "main.h"


uint32_t Flash_Write_Data (uint32_t StartPageAddress, uint32_t *Data, uint16_t numberofwords);

void Flash_Read_Data (uint32_t StartPageAddress, uint32_t *RxBuf, uint16_t numberofwords);



/********************  FLASH_Error_Codes   ***********************//*
HAL_FLASH_ERROR_NONE      0x00U  // No error
HAL_FLASH_ERROR_PROG      0x01U  // Programming error
HAL_FLASH_ERROR_WRP       0x02U  // Write protection error
HAL_FLASH_ERROR_OPTV      0x04U  // Option validity error
*/


#endif /* INC_FLASH_H_ */
