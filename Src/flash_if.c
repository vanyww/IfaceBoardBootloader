/**
 ******************************************************************************
 * @file    STM32F0xx_IAP/src/flash_if.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    29-May-2012
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"
#include "common.h"

/** @addtogroup STM32F0xx_IAP
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Unlocks Flash for write access
 * @param  None
 * @retval None
 */
void FLASH_If_Init(void) {
	/* Unlock the Program memory */
	HAL_FLASH_Unlock();

	/* Clear all FLASH flags */
	__HAL_FLASH_CLEAR_FLAG(
			FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
	/* Unlock the Program memory */
	HAL_FLASH_Lock();
}

/**
 * @brief  This function does an erase of all user flash area
 * @param  StartSector: start of user flash area
 * @retval 0: user flash area successfully erased
 *         1: error occurred
 */
uint32_t FLASH_If_Erase(uint32_t StartSector) {
	uint32_t NbrOfPages = 0;
	uint32_t PageError = 0;
	FLASH_EraseInitTypeDef pEraseInit;
	HAL_StatusTypeDef status = HAL_OK;

	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();

	/* Get the sector where start the user flash area */
	NbrOfPages = (USER_FLASH_END_ADDRESS - StartSector) / FLASH_PAGE_SIZE;

	pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	pEraseInit.PageAddress = StartSector;
	pEraseInit.NbPages = NbrOfPages;
	status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

	/* Lock the Flash to disable the flash control register access (recommended
	 to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock();

	if (status != HAL_OK) {
		/* Error occurred while page erase */
		return 1;
	}

	return 0;
}

/**
 * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
 * @note   After writing data buffer, the flash content is checked.
 * @param  FlashAddress: start address for writing data buffer
 * @param  Data: pointer on data buffer
 * @param  DataLength: length of data buffer (unit is 32-bit word)
 * @retval 0: Data successfully written to Flash memory
 *         1: Error occurred while writing data in Flash memory
 *         2: Written Data in flash memory is different from expected one
 */
uint32_t FLASH_If_Write(__IO uint32_t* FlashAddress, uint32_t* Data,
		uint16_t DataLength) {
	uint32_t i = 0;

	HAL_FLASH_Unlock();

	for (i = 0;
			(i < DataLength) && (*FlashAddress <= (USER_FLASH_END_ADDRESS - 4));
			i++) {
		/* the operation will be done by word */
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, *FlashAddress,
				*(uint32_t*) (Data + i)) == HAL_OK) {
			/* Check the written value */
			if (*(uint32_t*) *FlashAddress != *(uint32_t*) (Data + i)) {
				/* Flash content doesn't match SRAM content */
				return (2);
			}
			/* Increment FLASH destination address */
			*FlashAddress += 4;
		} else {
			/* Error occurred while writing data in Flash memory */
			return (1);
		}
	}

	HAL_FLASH_Lock();

	return (0);
}
/**
 * @brief  Disables the write protection of user desired pages
 * @param  None
 * @retval 0: Write Protection successfully disabled
 *         1: Error: Flash write unprotection failed
 *         2: Flash memory is not write protected
 */
uint32_t FLASH_If_DisableWriteProtection(void) {
	uint32_t ProtectedPAGE = 0x0;
	FLASH_OBProgramInitTypeDef config_new, config_old;
	HAL_StatusTypeDef result = HAL_OK;

	if (FLASH_If_GetWriteProtectionStatus() != 0x00) {
		/* Get pages write protection status ****************************************/
		HAL_FLASHEx_OBGetConfig(&config_old);

		/* The parameter says whether we turn the protection on or off */
		config_new.WRPState = OB_WRPSTATE_DISABLE;

		/* We want to modify only the Write protection */
		config_new.OptionType = OPTIONBYTE_WRP;

		/* No read protection, keep BOR and reset settings */
		config_new.RDPLevel = OB_RDP_LEVEL_0;
		config_new.USERConfig = config_old.USERConfig;
		/* Get pages already write protected ****************************************/
		ProtectedPAGE = config_old.WRPPage | FLASH_PROTECTED_PAGES;

		result = HAL_FLASHEx_OBErase();

		if (result == HAL_OK) {
			config_new.WRPPage = ProtectedPAGE;
			result = HAL_FLASHEx_OBProgram(&config_new);
		}

		if (result == HAL_OK) {
			/* Write Protection successfully disabled */
			return (0);
		} else {
			/* Error: Flash write unprotection failed */
			return (1);
		}
	}

	return 1;
}

/**
 * @brief  Returns the write protection status of user flash area.
 * @param  None
 * @retval If the sector is write-protected, the corresponding bit in returned
 *         value is set.
 *         If the sector isn't write-protected, the corresponding bit in returned
 *         value is reset.
 */
uint32_t FLASH_If_GetWriteProtectionStatus(void) {
	FLASH_OBProgramInitTypeDef OptionsBytesStruct;

	HAL_FLASH_Unlock();

	/* Check if there are write protected sectors inside the user flash area ****/
	HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);

	/* Lock the Flash to disable the flash control register access (recommended
	 to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock();
	return (~OptionsBytesStruct.WRPPage & FLASH_PROTECTED_PAGES);
}

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
