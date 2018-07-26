/**
 * @file main.c
 * @author 	Salvatore Barone <salvator.barone@gmail.com> ,
 * 			Alfonso Di Martino <alfonsodimartino160989@gmail.com> ,
 * 			Sossio Fiorillo <fsossio@gmail.com> ,
 * 		 	Pietro Liguori <pie.liguori@gmail.com> .
 *
 * @date 07 07 2017
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
 
/* Includes ------------------------------------------------------------------*/

#include "config.h"
#include <stdlib.h>

/**
 * @addtogroup busSeriali
 * @{
 * @addtogroup I2C
 * @{
 * @addtogroup I2C_Master_B
 * @{
 *
 * @brief Implementazione di un dispositivo Master che effettua un conteggio, visualizzato attraverso i led, 
 * e invia il valore di tale conteggio a un dispositivo Slave nel momento in cui viene premuto il push button. <br>
 * @details
 * 			Il dispositivo incrementa il valore di conteggio ogni due secondi. Ad ogni incremento vengono accesi / spenti i led
 *			sulla board, realizzando di fatto un contatore esadecimale.
 * 			Quando viene premuto il push button sulla board, viene inviato il valore del conteggio nel momento
 *			in cui è stato premuto il button ad un dispositivo slave attraverso una comunicazione seriale I2C.
 */
 
 /**
 * @brief Funzione di inizializzazione.
 *
 * @details E' la prima funzione ad essere eseguita nel main program. Si occupa di:
 * 			 - Inizializzare la libreria HAL;
 * 			 - Configurare il system clock;
 * 			 - Inizializzare e configurare tutte le periferiche utilizzate;
 * 			 - Inizializzare le variabili ed eseguire le funzioni richieste al reset del sistema.
 */
void setup();
/**
 * @brief Funzione che implementa la logica del programma.
 *
 * @details Ogni 2 secondi si incrementa lo il conteggio effettuato dal dispositivo master e viene visualizzato
 * 			il corrispondente valore attraverso i led, realizzando un contatore esadecimale.
 *
 */
void loop();
/**
 * @brief System Clock Configuration
*/
static void SystemClock_Config(void);
/**
 * @brief Accensione / Spegnimento dei Led secondo uno schema ad anello.
 * @details
 * 			Questa funzione viene richiamata nella callback associata alla pressione del  button
 * 			quando la comunicazione col dispositivo slave attraverso I2C non va a buon fine.
 *			In tal caso, viene effettuato un toggle dei led secondo uno schema circolare (ad anello)
 *          per segnalare la mancata corretta comunicazione tra il master e lo slave.
*/
void ringOfTheDeath(void);
/**
 * @brief Callback associata alla pressione del BUTTON.
 *
 * @details Alla pressione del push button viene effettuato  l'invio ad un dispositivo Slave del valore associato al conteggio. <br>
 * @param[in] GPIO_Pin : specifica il pin connesso alla linea EXTI.
 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

int main(void){
	setup();
	for(;;)
		loop();
	return 1;
}

I2C_HandleTypeDef I2cHandle;


int counter, countDec;
short int led;
void setup(){
	HAL_Init();
	SystemClock_Config();
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

	I2cHandle.Instance = I2C1;
	I2cHandle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	I2cHandle.Init.ClockSpeed = 400000;
	I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	I2cHandle.Init.DutyCycle = I2C_DUTYCYCLE_16_9;
	I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	I2cHandle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	I2cHandle.Init.OwnAddress1 = I2C_ADDRESS_B;
	HAL_I2C_Init(&I2cHandle);
	/*MCU Support Package*/
	led=0;
	counter= 0x0000;
	countDec = 0;

}

void loop(){
	HAL_Delay(2000);
	counter+=0x1000;
	countDec++;
	HAL_GPIO_WritePin(GPIOD,counter,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,~counter,GPIO_PIN_RESET);
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	uint8_t* txBuffer = (uint8_t*) malloc(2);
	txBuffer[0] = 'B';
	txBuffer[1] = countDec%16; //F3

	//HAL_I2C_Mem_Write
    while(HAL_OK != HAL_I2C_Master_Transmit_IT(&I2cHandle, I2C_ADDRESS_C,txBuffer, 2)){
		if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF){
			ringOfTheDeath();
		}
    }
    while (HAL_I2C_STATE_READY != HAL_I2C_GetState(&I2cHandle));


    free(txBuffer);
}

void ringOfTheDeath(){
	for(;;){
		BSP_LED_On(LED3);
		HAL_Delay(400);
		BSP_LED_Off(LED3);
		BSP_LED_On(LED4);
		HAL_Delay(400);
		BSP_LED_Off(LED4);
		BSP_LED_On(LED5);
		HAL_Delay(400);
		BSP_LED_Off(LED5);
		BSP_LED_On(LED6);
		HAL_Delay(400);
		BSP_LED_Off(LED6);
	}
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

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
