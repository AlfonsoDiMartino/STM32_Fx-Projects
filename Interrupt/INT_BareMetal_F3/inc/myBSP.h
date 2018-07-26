/**
 * @file myBSP.h
 * @author
 * 			Salvatore Barone <salvator.barone@gmail.com> ,
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

#ifndef MYBSP_H_
#define MYBSP_H_


/**
 * @addtogroup BSP_STM32Fx
 * @{
 *	@brief Modulo BSP per Led e Button su STM32f3/F4 Discovery. <br>
 *
 *	@details Questo modulo definisce un Board Support Package (BSP) minimale, Led e Button User, per STM32F3/F4 Discovery.<br>
 *	Il modulo myBSP utilizza l'HAL della libreria STMCube per astrarre l'hardware specifico utilizzato.<br>
 *	A seconda della board selezionata per il progetto, automaticamente verranno messe a disposizione le corrispondenti strutture che astraggono le periferiche
 *	supportate per la board selezionata e definite per il BSP in uso. <br>
 * 	Per i dettagli si rimanda alla documentazione delle specifiche funzioni ed alla documentazione esterna che accompagna
 * 	il modulo, reperibile nella cartella Doc.
 *
 */

/**
 * @defgroup BSP_STM32Fx_LED
 * @ingroup BSP_STM32Fx
 * @{
 *
 * @brief Modulo di definizione delle funzioni per l'utilizzo dei Led.
 */

#if defined(STM32F30) || defined(STM32F3DISCOVERY) || defined(STM32F3) || defined(STM32F303VCTx) || defined(STM32F303xC)
/**
 * @defgroup BSP_STM32F3_LED
 * @ingroup BSP_STM32Fx_LED
 * @{
 *
 *	@brief Sezione relativa alla definizione di macro e strutture per i LED presenti su STM32F3
 */

#include "stm32f3xx_hal.h"

/*
 * ******** LED STM32F3 MACRO ********
 */

#define N_LED				8				//!< Numero di Led presenti su STM32F3

#define myLED4_GPIO_PORT	GPIOE			//!< LED4 collegato sul GPIOE
#define myLED4_GPIO_PIN		GPIO_PIN_8		//!< LED4 sul Pin 8

#define myLED3_GPIO_PORT	GPIOE			//!< LED3 collegato sul GPIOE
#define myLED3_GPIO_PIN		GPIO_PIN_9		//!< LED3 sul Pin 9

#define myLED5_GPIO_PORT	GPIOE			//!< LED5 collegato sul GPIOE
#define myLED5_GPIO_PIN		GPIO_PIN_10		//!< LED5 sul Pin 10

#define myLED7_GPIO_PORT	GPIOE			//!< LED7 collegato sul GPIOE
#define myLED7_GPIO_PIN		GPIO_PIN_11		//!< LED7 sul Pin 11

#define myLED9_GPIO_PORT	GPIOE			//!< LED9 collegato sul GPIOE
#define myLED9_GPIO_PIN		GPIO_PIN_12		//!< LED9 sul Pin 12

#define myLED10_GPIO_PORT	GPIOE			//!< LED10 collegato sul GPIOE
#define myLED10_GPIO_PIN	GPIO_PIN_13		//!< LED10 sul Pin 13

#define myLED8_GPIO_PORT	GPIOE			//!< LED8 collegato sul GPIOE
#define myLED8_GPIO_PIN		GPIO_PIN_14		//!< LED8 sul Pin 14

#define myLED6_GPIO_PORT	GPIOE			//!< LED6 collegato sul GPIOE
#define myLED6_GPIO_PIN		GPIO_PIN_15		//!< LED6 sul Pin 15

#define myLED_GPIO_CLK_ENABLE()	__HAL_RCC_GPIOE_CLK_ENABLE()	//!< Ablitita il clock su GPIOE

/**
 * @brief Seleziona uno degli 8 led presenti sulla board STM32F3
 */
typedef enum{
	myLED4,						//!< Indice 0 - Seleziona il LED4
	myLED3,						//!< Indice 1 - Seleziona il LED3
	myLED5,						//!< Indice 2 - Seleziona il LED5
	myLED7,						//!< Indice 3 - Seleziona il LED7
	myLED9,						//!< Indice 4 - Seleziona il LED9
	myLED10,					//!< Indice 5 - Seleziona il LED10
	myLED8,						//!< Indice 6 - Seleziona il LED8
	myLED6						//!< Indice 7 - Seleziona il LED6
}myLED_t;

/** @} */
#endif

#if defined(DSTM32F407VGTx) || defined(STM32F4) || defined(STM32F4DISCOVERY) || defined(STM32F407xx)
/**
 * @defgroup BSP_STM32F4_LED
 * @ingroup BSP_STM32Fx_LED
 * @{
 *
 *	@brief Sezione relativa alla definizione di macro e strutture per i LED presenti su STM32F4
 *
 */

#include "stm32f4xx_hal.h"

/*
 * ******** LED STM32F4 MACRO ********
 */

#define N_LED 				4				//!< Numero Led presenti su STM32F4

#define myLED4_GPIO_PORT	GPIOD			//!< LED4 collegato sul GPIOD
#define myLED4_GPIO_PIN		GPIO_PIN_12		//!< LED4 sul Pin 12

#define myLED3_GPIO_PORT	GPIOD			//!< LED3 collegato sul GPIOD
#define myLED3_GPIO_PIN		GPIO_PIN_13		//!< LED3 sul Pin 13

#define myLED5_GPIO_PORT	GPIOD			//!< LED5 collegato sul GPIOD
#define myLED5_GPIO_PIN		GPIO_PIN_14		//!< LED5 sul Pin 14

#define myLED6_GPIO_PORT	GPIOD			//!< LED6 collegato sul GPIOD
#define myLED6_GPIO_PIN		GPIO_PIN_15		//!< LED6 sul Pin 15

#define myLED_GPIO_CLK_ENABLE()	__HAL_RCC_GPIOD_CLK_ENABLE()	//!< Ablitita il clock su GPIOD

/**
 * @brief Seleziona uno dei 4 led presenti sulla board STM32F4
 */
typedef enum{
	myLED4,						//!< Indice 0 - Seleziona il LED4
	myLED3,						//!< Indice 1 - Seleziona il LED3
	myLED5,						//!< Indice 2 - Seleziona il LED5
	myLED6						//!< Indice 3 - Seleziona il LED6
}myLED_t;

/** @} */
#endif

/*
 *  ******** LED STM32Fx FUNCTIONS ********
 */

/**
 * @brief Funzione che permette di inizializzare un LED specificato.
 * @param[in] 	Led: specifica il Led da inizializzare.
 *
 * @code
 * myBSP_LED_Init(LED4);
 * @endcode
 */
void myBSP_LED_Init(myLED_t Led);

/**
 * @brief Funzione che permette di accendere un LED specificato.
 * @param[in] 	Led: specifica il Led da accendere.
 *
 * @code
 * myBSP_LED_Init(LED4);
 * myBSP_LED_On(LED4);
 * HAL_Delay(100);
 * myBSP_LED_Off(LED4);
 * @endcode
 *
 */
void myBSP_LED_On(myLED_t Led);

/**
 * @brief Funzione che permette di spegnere un LED specificato.
 * @param[in] 	Led: specifica il Led da spegnere.
 *
 * @code
 * myBSP_LED_Init(LED4);
 * myBSP_LED_On(LED4);
 * HAL_Delay(100);
 * myBSP_LED_Off(LED4);
 * @endcode
 */
void myBSP_LED_Off(myLED_t Led);

/**
 * @brief Funzione che permette di effettuare il toogle di un LED specificato.
 * @param[in] 	Led: specifica il Led su cui effetturare il toogle.
 *
 * @code
 * myBSP_LED_Init(LED4);
 * myBSP_LED_Toggle(LED4);
 * @endcode
 */
void myBSP_LED_Toggle(myLED_t);

/** @} */

/**
 * @defgroup BSP_STM32Fx_BUTTON
 * @ingroup BSP_STM32Fx
 * @{
 *
 * @brief Modulo di definizione delle funzioni per l'utilizzo del Button User.
 */

/*
 * ******** BUTTON STM32F3/F4 MACRO ********
 */

#define N_BUTTON					1				//!< Numero Button User presenti su STM32Fx

#define myBUTTON_GPIO_PORT			GPIOA							//!< BUTTON User collegato sul GPIOA
#define myBUTTON_GPIO_PIN			GPIO_PIN_0						//!< BUTTON User sul Pin 0
#define myBUTTON_EXTI_IRQn  		EXTI0_IRQn						//!< BUTTON User sul EXTI0_IRQn vector
#define myBUTTON_GPIO_CLK_ENABLE()	__HAL_RCC_GPIOA_CLK_ENABLE() 	//!< Abilita il clock su GPIOA


/**
 * @brief Seleziona il button user presente sulla board STM32F3/F4
 */
typedef enum{
	myBUTTON								//!< Indice 0 - BUTTON User
}myBUTTON_t;

/**
 * @brief Seleziona la modalita' del button User.
 */
typedef enum{
	myBUTTON_MODE_GPIO,						//!< Indice 0 - Seleziona la modalita' IO
	myBUTTON_MODE_EXTI 						//!< Indice 1 - Seleziona la modalita' Interrupt esterno
}myBUTTON_MODE_Typedef;


/*
 * ******** BUTTON STM32Fx FUNCTIONS********
 */

/**
 * @brief Funzione che permette di inizializzare un Button specificato.
 * @param[in] 		Button: specifica il Button da inizializzare.
 * @param[in]		Mode: specifica se il Button è usato come device IO o collegato a una linea di interruzione esterna.
 *
 * @code
 * myBSP_BUTTON_Init(myBUTTON,myBUTTON_MODE_EXTI);
 * @endcode
 */
void myBSP_BUTTON_Init(myBUTTON_t Button, myBUTTON_MODE_Typedef Mode);

/**
 * @brief Funzione che permette di leggere lo stato del Button selezionato.
 * @param[in] Button: specifica il Button selezionato.
 *
 * @return 	Restituisce lo stato del pin letto
 * @retval 	GPIO_PIN_SET 	se il pin letto e' GPIO_PIN_SET;
 * @retval	GPIO_PIN_RESET 	se il pin letto e' GPIO_PIN_RESET;
 *
 * @code
 * myBSP_LED_Init(LED4);
 * myBSP_BUTTON_Init(myBUTTON,myBUTTON_MODE_GPIO);
 * (myBSP_BUTTON_GetStatus(myBUTTON)==1 ? myBSP_LED_On(LED4) : myBSP_LED_Off(LED4));
 * @endcode
 */

uint32_t myBSP_BUTTON_GetStatus(myBUTTON_t Button);

/** @} */

/** @} */
#endif /* MYBSP_H_ */



