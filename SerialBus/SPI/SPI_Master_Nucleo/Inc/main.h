/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_nucleo.h"
#include "stm32f4xx_hal.h"
#include "hd44780.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/**
 * @addtogroup busSeriali
 * @{
 * @addtogroup SPI
 * @{
 * @addtogroup SPI_Master
 * @{
 */

/* Private define ------------------------------------------------------------*/

/* BUTTON USER */
#define B1_Pin 				GPIO_PIN_13				//!< Pin associato al Button User
#define B1_GPIO_Port 		GPIOC					//!< Porto associato al Button User
#define B1_EXTI_IRQn 		EXTI15_10_IRQn			//!< External Line[15:10] Interrupts

/* LED 2 */
#define LD2_Pin 			GPIO_PIN_5				//!< Pin associato al Led2 (Led Verde)
#define LD2_GPIO_Port 		GPIOA					//!< Porto associato al Led2 (Led Verde)

/* LCD esterno */
	// Segnali di controllo //
#define LCD_RS_GPIO_Port 	GPIOB					//!< Porto associato al segnale di controllo RS
#define LCD_RS_Pin		 	GPIO_PIN_15				//!< Pin associato al segnale di controllo RS
#define LCD_RW_GPIO_Port 	GPIOB					//!< Porto associato al segnale di controllo RW
#define LCD_RW_Pin		 	GPIO_PIN_14				//!< Pin associato al segnale di controllo RW
#define LCD_E_GPIO_Port 	GPIOB					//!< Porto associato al segnale di controllo E
#define LCD_E_Pin		 	GPIO_PIN_13				//!< Pin associato al segnale di controllo E
	// Segnali dato //
#define LCD_D0_GPIO_Port 	GPIOB					//!< Porto associato al segnale Dato D0
#define LCD_D0_Pin		 	GPIO_PIN_1				//!< Pin associato al segnale Dato D0
#define LCD_D1_GPIO_Port 	GPIOB					//!< Porto associato al segnale Dato D1
#define LCD_D1_Pin		 	GPIO_PIN_2				//!< Pin associato al segnale Dato D1
#define LCD_D2_GPIO_Port 	GPIOB					//!< Porto associato al segnale Dato D2
#define LCD_D2_Pin		 	GPIO_PIN_12				//!< Pin associato al segnale Dato D2
#define LCD_D3_GPIO_Port 	GPIOA					//!< Porto associato al segnale Dato D3
#define LCD_D3_Pin		 	GPIO_PIN_11				//!< Pin associato al segnale Dato D3
#define LCD_D4_GPIO_Port 	GPIOA					//!< Porto associato al segnale Dato D4
#define LCD_D4_Pin		 	GPIO_PIN_12				//!< Pin associato al segnale Dato D4
#define LCD_D5_GPIO_Port 	GPIOC					//!< Porto associato al segnale Dato D5
#define LCD_D5_Pin		 	GPIO_PIN_5				//!< Pin associato al segnale Dato D5
#define LCD_D6_GPIO_Port 	GPIOC					//!< Porto associato al segnale Dato D6
#define LCD_D6_Pin		 	GPIO_PIN_6				//!< Pin associato al segnale Dato D6
#define LCD_D7_GPIO_Port 	GPIOC					//!< Porto associato al segnale Dato D7
#define LCD_D7_Pin		 	GPIO_PIN_8				//!< Pin associato al segnale Dato D7

/**
 * @brief Definisce i due possibili stati di esecuzione della macchina. <br>
 */
typedef enum{
	IDLE = 0,		//!< Lo stato IDLE è lo stato di reset e riposo, in attesa di una richiesta di misurazione. <br>
	RUNNING = 1		/*!< Lo stato RUNNING è lo stato di servizio di una richiesta. Quando si è in questo stato,
						non verranno accettate altre richieste di misurazione. <br> */
}StateTypeDef;

void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)


/**
 * @}
 * @}
 * @}
 */


/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
