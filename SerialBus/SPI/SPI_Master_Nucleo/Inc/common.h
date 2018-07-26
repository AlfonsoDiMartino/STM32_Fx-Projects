/**
 * @file common.h
 * @author Salvatore Barone
 * @email salvator.barone@gmail.com
 * @date 15 08 2016
 *
 * @copyright
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the License, or any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __STM32_DRIVER_PACK_COMMON
#define __STM32_DRIVER_PACK_COMMON

/**
 * @addtogroup Common
 * @{
 */

#include <inttypes.h>
#include <stdlib.h>

#if defined(STM32F30) || defined(STM32F3DISCOVERY) || defined(STM32F3) || defined(STM32F303VCTx) || defined(STM32F303xC)
#include "stm32f3xx_hal.h"
#endif

#if defined(DSTM32F407VGTx) || defined(STM32F4) || defined(STM32F4DISCOVERY) || defined(STM32F407xx)
#include "stm32f4xx_hal.h"
#endif

typedef struct
{
	GPIO_TypeDef* Port;
	uint32_t Pin;
} PortPinPair_t;

/**
 * @brief Frequenza del clock, in Hertz
 * @warning deve essere aggiustata a seconda del clock con cui viene fatto funzionare il processore,
 * altrimenti la funzione DelayUS() non funzionera' correttamente
 */
#define CLOCK_FREQUENCY_HZ 8000000

/**
 * @brief Consente di fermare l'esecuzione del programma per un certo periodo di tempo, in millisecondi
 *
 * @warning La macro va modificata solo se strettamente necessario
 *
 * @param ms millisecondi per cui fermare l'esecuzione del programma
 */
#define timer_wait_ms(ms)   HAL_Delay(ms)

/**
 * @brief Consente di fermare l'esecuzione del programma per un certo periodo di tempo, in nanosecondi
 *
 * @warning La macro va modificata solo se strettamente necessario
 *
 * @param us nanosecondi per cui fermare l'esecuzione del programma
 */
#define timer_wait_us(us)	DelayUS(us)

/**
 * Le seguenti due macro vanno modificate con il valore del tipo enumerativo IRQn_Type
 * specifico per il particolare processore.
 * Il tipo enumerativo IRQn_Type e' definito nel file specifico per uno specifico processore
 * all'interno del percorso
 * Driver/CMSIS/ST/STM32Fxxx/Include/stm32fxxxxxxx.h
 * Ad esempio, quello per il processore STM32F303VCT6 usato per il testing e' il file
 * Driver/CMSIS/ST/STM32F3xx/Include/stm32f3030xc.h
 */
#define EXTI_LINE_9_5	EXTI9_5_IRQn     	/*!< External Line[9:5] Interrupts per STM32F303VCT6   */
#define EXTI_LINE_15_10	EXTI15_10_IRQn 		/*!< External Line[15:10] Interrupts  per STM32F303VCT6*/

/**
 * @brief Consente di fermare l'esecuzione del programma per un certo periodo di tempo, in nanosecondi
 *
 * @warning La macro va modificata solo se strettamente necessario
 *
 * @param us nanosecondi per cui fermare l'esecuzione del programma
 */
void DelayUS(uint32_t us);

#endif

/** @} */
