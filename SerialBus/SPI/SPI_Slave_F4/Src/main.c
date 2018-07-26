/**
 * @file main.c
 * @author 	Salvatore Barone <salvator.barone@gmail.com> ,
 * 			Alfonso Di Martino <alfonsodimartino160989@gmail.com> ,
 * 			Sossio Fiorillo <fsossio@gmail.com> ,
 * 		 	Pietro Liguori <pie.liguori@gmail.com> .
 *
 * @date 08 07 2017
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
#include "main.h"

/**
 * @addtogroup busSeriali
 * @{
 * @addtogroup SPI
 * @{
 * @addtogroup SPI_Slave
 * @{
 * @brief Implementazione del dispositivo Slave che effettua la misura di temperatura con un sensore di temperatura esterno. <br>
 *
 * @details Il dispositivo utilizza un sensore esterno per la misurazione, in particolare il sensore LM35 - Precision Centigrade Temperature Sensors,
 * 			 che invia il valore di tensione corrispondente alla misura effettuata al Pin0 di ingresso dell' ADC1 presente a bordo della board. Terminata la
 * 			 conversione analogico-digitale, il valore di temperatura, espresso in °C e rappresentato su 8 bit, viene trasferito alla board Master tramite
 * 			 comunicazione realizzata su bus seriale SPI.
 *
 *
 */

/**
 * @brief System Clock Configuration
*/
void SystemClock_Config(void);

/**
 * @brief Funzione di configurazione ed inizializzazione delle periferiche GPIO utilizzate.
 *
 * @details Abilità il clock di GPIOA, di cui utilizzo il Pin0 come ingresso all'ADC1
 */
static void MX_GPIO_Init(void);

/**
 * @brief Funzione di configurazione ed inizializzazione del modulo ADC.
 *
 * @details Vengono configurati tutti i parametri dell'ADC, per esempio la risoluzione, impostata a 12 bit.
 * 		 	Vengono configurati anche i parametri per l'ADC regular channel selezioanto, nel nostro caso il canale è ADC_CHANNEL_0 su PA0.
*/
static void MX_ADC1_Init(void);

/**
 * @brief Funzione di configurazione ed inizializzazione del modulo SPI.
 *
 * @details Il modulo è configurato come Slave, la comunicazione è bidirezionale con dimensione dei blocchi trasferiti di 8 bit. La coppia CPOL-CPHA è 0-0.
 * 		 	La modalità di selezione dello slave è impostata come SOFT => software, quindi gestita automaticamente dal modulo, senza necessità di avere una linea
 * 		 	fisica per l'abilitazione dello slave.
*/
static void MX_SPI1_Init(void);

/**
 * @brief Regular conversion complete callback in non blocking mode.
 *
 *	@details Terminata la conversione, prelevo il valore dal DR della struttura hadc. Il risultato generato dall'ADC indica uno dei 4096 possibili livelli
 *			(dato che la risoluzione è di 12 bit => 4096 livelli) su cui è discretizzato un range di tensioni da 0 a 3V.
 *			Per associare al livello il corrispondente valore di tensione, moltiplico per 3000 (portando il risultato in mV) e divido per (2^Risoluzione)-1.
 *			Dato che il sensore genera una tensione che cresce linearmente con la temperatura, con una relazione 10mV = 1 °C, per esprimere il risultato  in °C
 *			divido tutto per 10.
 *
 * @param[in] hadc : puntatore alla struttura ADC_HandleTypeDef.
 * @param[out] txBuffer : buffer di trasmissione usato per la comunicazione SPI.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

/**
 * @brief Funzione di inizializzazione.
 *
 * @details E' la prima funzione ad essere eseguita nel main program. Si occupa di:
 * 			 - Inizializzare la libreria HAL;
 * 			 - Configurare il system clock;
 * 			 - Inizializzare e configurare tutte le periferiche utilizzate; <br>
 * 			 - Inizializzare le variabili ed eseguire le funzioni richieste al reset del sistema. <br>
 */
void setup(void);

/**
 * @brief Funzione che implementa la logica del programma.
 *
 * @details Il dispositivo si mette in attesa di ricevere il codice di richiesta misurazione temperatura (0xFF) dal Master e una volta ricevuto attiva l'ADC
 * 			a bordo per una conversione in modalità non bloccante. Terminata la conversione, il valore di temperatura misurato viene salvato nel buffer di
 * 			trasmissione, che viene inviato al Master durante la seconda comunicazione.
 */
void loop(void);


/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;		//!< Handle della struttura ADC che sara' inizializzata.
SPI_HandleTypeDef hspi1;		//!< Handle della struttura SPI che sara' inizializzata.

uint8_t txBuffer;				//!< Buffer di trasmissione utilizzato per la comunicazione SPI.
uint8_t rxBuffer;				//!< Buffer di ricezione utilizzato per la comunicazione SPI.


int main(void)
{
	setup();
  for(;;) loop();
  return 1;
}


void setup(void){
/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
/* Configure the system clock */
  SystemClock_Config();
/* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  BSP_LED_Init(LED6);				// utilizzato per "debug visivo" su board
/* Azioni e inizializzazioni al reset */
  txBuffer = 0;						// reset del buffer di trasmissione SPI
  rxBuffer = 0;						// reset del buffer di trasmissione SPI
}

void loop(void){
	if(HAL_SPI_TransmitReceive(&hspi1,(uint8_t *)&txBuffer,(uint8_t *)&rxBuffer,1,1000) == HAL_OK){		// in attesa di una richiesta di comunicazione dal Master
	  // valuto il codice ricevuto dal Master:
	  // se ho ricevito 0xFF => codice di richiesta misurazione Temperatura, attivo l'ADC per una conversione in modalita' non bloccante.
	  // altrimenti reset di txBuffer, pulendo così il buffer di trasmissione dal valore di temperatura inviato nella precedente comunicazione.
	  (rxBuffer == 0xFF ? HAL_ADC_Start_IT(&hadc1):(txBuffer=0));

	  BSP_LED_Toggle(LED6);					// utilizzato per "debug visivo" su board
  }
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	uint32_t ADC_raw = HAL_ADC_GetValue(hadc);
	uint8_t temp = (uint8_t)(((3000*ADC_raw)/4095)/10);
	txBuffer = temp;
}

/* System Clock Configuration */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV8;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_SLAVE;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Pinout Configuration
*/
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
