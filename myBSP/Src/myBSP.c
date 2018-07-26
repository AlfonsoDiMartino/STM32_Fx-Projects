/**
 * @file myBSP.c
 * @author 	Salvatore Barone <salvator.barone@gmail.com> ,
 * 			Alfonso Di Martino <alfonsodimartino160989@gmail.com> ,
 * 			Sossio Fiorillo <fsossio@gmail.com> ,
 * 		 	Pietro Liguori <pie.liguori@gmail.com> .
 *
 * @date 07 06 2017
 *
 * @copyright
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the License, or any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#include "myBSP.h"

/* ************************************************************************************************************ */
/* Definizione strutture private del modulo myBSP.c per indirizzare porto e pin degli 8 Led presenti su STM32F3 */
/* ************************************************************************************************************ */

#if defined(STM32F30) || defined(STM32F3DISCOVERY) || defined(STM32F3) || defined(STM32F303VCTx) || defined(STM32F303xC)

/* Vettore dei GPIO assegnati ai LED su STM32F3 */

GPIO_TypeDef *myGPIO_PORT[N_LED] = {myLED4_GPIO_PORT,
									myLED3_GPIO_PORT,
									myLED5_GPIO_PORT,
									myLED7_GPIO_PORT,
									myLED9_GPIO_PORT,
									myLED10_GPIO_PORT,
									myLED8_GPIO_PORT,
									myLED6_GPIO_PORT};


/* Vettore delle maschere di selezione LED su STM32F3 */

 uint16_t myPIN_LED[N_LED] = {myLED4_GPIO_PIN,
							  myLED3_GPIO_PIN,
							  myLED5_GPIO_PIN,
							  myLED7_GPIO_PIN,
							  myLED9_GPIO_PIN,
 	 	 	 	 	 	 	  myLED10_GPIO_PIN,
							  myLED8_GPIO_PIN,
							  myLED6_GPIO_PIN};

#endif



/* ********************************************************************************************************** */
/* Definizione strutture private del modulo myBSP.c per indirizzare porto e pin dei 4 Led presenti su STM32F4 */
/* ********************************************************************************************************** */

#if defined(DSTM32F407VGTx) || defined(STM32F4) || defined(STM32F4DISCOVERY) || defined(STM32F407xx)

/* Vettore dei GPIO assegnati ai LED su STM32F4 */

 GPIO_TypeDef *myGPIO_PORT[N_LED] = {myLED4_GPIO_PORT,
									myLED3_GPIO_PORT,
									myLED5_GPIO_PORT,
									myLED6_GPIO_PORT};

/* Vettore delle maschere di selezione LED su STM32F4 */

uint16_t myPIN_LED[N_LED] = {myLED4_GPIO_PIN,
							 myLED3_GPIO_PIN,
							 myLED5_GPIO_PIN,
							 myLED6_GPIO_PIN};

#endif



/* ********************************************************************************************************** */
/* Implementazione BSP_LED Function */
/* ********************************************************************************************************** */

#if defined(STM32F30) || defined(STM32F3DISCOVERY) || defined(STM32F3) || defined(STM32F303VCTx) || defined(STM32F303xC) || \
	defined(DSTM32F407VGTx) || defined(STM32F4) || defined(STM32F4DISCOVERY) || defined(STM32F407xx)

void myBSP_LED_Init(myLED_t Led){

	GPIO_InitTypeDef GPIO_InitStruct;

	/* Enable the GPIO_LED Clock */
	  myLED_GPIO_CLK_ENABLE();

	/* Configure the GPIO_LED pin */
	  GPIO_InitStruct.Pin = myPIN_LED[Led];
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(myGPIO_PORT[Led], &GPIO_InitStruct);
	HAL_GPIO_WritePin(myGPIO_PORT[Led], myPIN_LED[Led], GPIO_PIN_RESET);
}


void myBSP_LED_On(myLED_t Led){
	HAL_GPIO_WritePin(myGPIO_PORT[Led],myPIN_LED[Led],GPIO_PIN_SET);
}


void myBSP_LED_Off(myLED_t Led){
	HAL_GPIO_WritePin(myGPIO_PORT[Led],myPIN_LED[Led],GPIO_PIN_RESET);
}


void myBSP_LED_Toggle(myLED_t Led){
	HAL_GPIO_TogglePin(myGPIO_PORT[Led],myPIN_LED[Led]);
}



/* ******************************************************************************************************************* */
/* Definizione strutture private del modulo myBSP.c per indirizzare porto e pin del Button User presente su STM32F3/F4 */
/* ******************************************************************************************************************* */

/* GPIO assegnato al BUTTON */

GPIO_TypeDef *myBUTTON_PORT[N_BUTTON] = {myBUTTON_GPIO_PORT};

/* Maschere di selezione del BUTTON */

uint16_t myPIN_BUTTON[N_BUTTON] = {myBUTTON_GPIO_PIN};

/* Linea di interruzione assegnata al BUTTON */

uint8_t myBUTTON_IRQn[N_BUTTON] = {myBUTTON_EXTI_IRQn};


/* ********************************************************************************************************** */
/* Implementazione BSP_BUTTON Function */
/* ********************************************************************************************************** */

void myBSP_BUTTON_Init(myBUTTON_t Button, myBUTTON_MODE_Typedef Mode){
	GPIO_InitTypeDef GPIO_InitStruct;

	  /* Enable the BUTTON Clock */
		myBUTTON_GPIO_CLK_ENABLE();

	  if (Mode == myBUTTON_MODE_GPIO)
	  {
	    /* Configure Button pin as input */
	    GPIO_InitStruct.Pin = myPIN_BUTTON[Button];
	    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	    HAL_GPIO_Init(myBUTTON_PORT[Button], &GPIO_InitStruct);
	  }

	  if (Mode == myBUTTON_MODE_EXTI)
	  {
	    /* Configure Button pin as input with External interrupt */
	    GPIO_InitStruct.Pin = myPIN_BUTTON[Button];
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	    HAL_GPIO_Init(myBUTTON_PORT[Button], &GPIO_InitStruct);

	    /* Enable and set Button EXTI Interrupt to the lowest priority */
	    HAL_NVIC_SetPriority((IRQn_Type)(myBUTTON_IRQn[Button]), 0x0F, 0);
	    HAL_NVIC_EnableIRQ((IRQn_Type)(myBUTTON_IRQn[Button]));
	  }
}


uint32_t myBSP_BUTTON_GetStatus(myBUTTON_t Button){
	return HAL_GPIO_ReadPin(myBUTTON_PORT[Button], myPIN_BUTTON[Button]);
}

#endif
