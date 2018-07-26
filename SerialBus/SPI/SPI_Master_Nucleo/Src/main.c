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
 * @addtogroup SPI_Master
 * @{
 *
 * @brief Implementazione del dispositivo Master che effettua una richiesta di misurazione temperatura alla pressione del tasto blu (user button) e
 * 			ricevuta la misura, la visualizza su display LCD esterno. <br>
 * @details
 * 			Il dispositivo è configurato come una macchina a 2 stati:
 * 			 - IDLE indica lo stato di riposo in cui la macchina si porta al reset o dopo aver servito una richiesta di misurazione temperatura e successiva
 * 			visualizzazione su display. Una richiesta di misurazione (o altra operazione o funzionalita' aggiunta successivamente) viene servita solo se la macchina
 * 			si trova in questo stato. La macchina passa dallo stato IDLE allo stato RUNNING alla pressione del tasto blu.
 * 			 - RUNNING indica lo stato di servizio in corso della richiesta e se la macchina si trova in questo stato ignora altre richieste scatenate dalla
 * 			pressione del tasto blu (user button). La macchina ritorna nello stato IDLE dopo aver terminato la richiesta che l'ha portata nello stato RUNNING,
 * 			richiesta che si ritiene conclusa quando il valore di temperatura ricevuto viene visualizzato sul diplay LCD esterno.
 * 			La comunicazione con il dispositivo Slave è realizzata su bus seriale SPI.
 */

/**
 * @brief System Clock Configuration
*/
void SystemClock_Config(void);

/**
 * @brief Funzione di configurazione ed inizializzazione delle periferiche GPIO utilizzate.
 *
 * @details Le periferiche configurate sono:
 * 		 	 - il LED2 (led verde);
 * 		 	 - lo user button (button blu), in modalità interrupt. La linea di interruzione associatamè la linea EXTI15_10_IRQn, con PreemptPriority = 0 e SubPriority = 0.
 */
static void MX_GPIO_Init(void);

/**
 * @brief Funzione di configurazione ed inizializzazione del modulo SPI.
 *
 * @details Il modulo è configurato come Master, la comunicazione è bidirezionale con dimensione dei blocchi trasferiti di 8 bit. La coppia CPOL-CPHA è 0-0.
 * 		 	La modalità di selezione dello slave è impostata come SOFT => software, quindi gestita automaticamente dal modulo, senza necessità di avere una linea
 * 		 	fisica per l'abilitazione dello slave.
 */
static void MX_SPI1_Init(void);

/**
 * @brief Funzione di configurazione ed inizializzazione dei GPIO cui è collegato il display LCD HD44780 utilizzato per mostrare il risultato della misurazione di temperatura. <br>
 *
 * @ warning Tale configurazione è utilizzabile su NUCLEO64-F401RE. <br>
 * 			Per maggiori informazioni sull'utilizzo del display LCD HD44780 si rimanda alla documentazione di riferimento e driver associati.
 */
static void LCD_Init(void);

/**
 * @brief Callback associata alla pressione del BUTTON USER.
 *
 * @details Alla pressione del tasto blu e' associata una variazione di stato che permette di servire in modo corretto una richiesta di misurazione di temperatura. <br>
 * 			In particolare, nel caso in cui si e' nello stato di reset, indicato come IDLE, posso servire una richiesta di misurazione temperatura. Setto la
 * 			variabile di stato a RUNNING, così da rifiutare nuove richieste fino a quando quella in corso non viene terminata, altrimenti non viene compiuta
 * 			alcuna operazione.
 *
 * @param[in] GPIO_Pin : specifica il pin connesso alla linea EXTI.
 * @param[inout] stato : variabile associata allo stato del dispositivo Master.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

/**
 * @brief Funzione che realizza la richiesta di una nuova misurazione di temperatura alla board "slave" cui è collegato il sensore di temperatura.
 *
 * @details Vengono effettuate due transazioni SPI complete (TxRX):
 * 			 -# nella prima transazione viene "svegliata" la board slave e viene inviato il codice 0xFF, a cui è stato associato, lato slave, l'azione di
 * 			"avvio misura su sensore di temperatura esterno". La scelta di inviare un codice che viene valutato lato slave è giustificata dal fatto che,
 * 			se necessario, è possibile aggiungere nuovi codici ai quali asscoiare, lato slave, nuove operazioni o funzionalità (per esempio, posso aggiungere
 * 			il codice 0x01 e associare, lato slave, l'operazione di "misurazione sul dipositivo accellerometro").
 * 			 -# nella seconda transazione viene inviato un dummy byte (0x00) e ricevuto il valore di temperatura misurato lato slave.
 *
 * @param [in] hspi : puntatore alla struttura SPI_HandleTypedef che contiene la configurazione del modulo SPI.
 * @retval temp : Valore di temperatura misurata.
 *
 */
uint8_t richiediTemperatura(SPI_HandleTypeDef *hspi);

/**
 * @brief Funzione di stampa su display LCD.
 *
 * @details La funzione utilizza delle primitive definite ad-hoc per il display LCD utilizzato, nello specifico un dipositivo HD44780, la cui documentazione e
 * 		  driverPack completo è disponibile nei riferimenti.
 *
 * @param[in] lcd : puntatore alla struttura HD44780_LCD_t.
 * @param[in] temp : il valore di temperatura da visualizzare.
 */
void visualizzaTemperatura(HD44780_LCD_t *lcd, uint8_t temp);

/**
 * @brief Funzione che implementa una transazione tra Master e Slave.
 *
 *@details La transazione è realizzata utilizzando la primitiva HAL_SPI_TransmitReceive, che permette l'invio e la ricezione di una fissata quantita' di dati,
 * 			in modalita' bloccante, con dimensione del blocco fissata in fase di configurazione del modulo SPI e numero di blocchi definito dal parametro Size.
 *
 * @param[in] hspi : puntatore alla struttura SPI_HandleTypedef che contiene la configurazione del modulo SPI.
 * @param[in] spiTxBuffer : puntatore al buffer di trasmissione dati.
 * @param[in] spiRxBuffer : puntatore al buffer di ricezione dati.
 */
void spiTransaction(SPI_HandleTypeDef *hspi, uint8_t *spiTxBuffer, uint8_t *spiRXBuffer);

/**
 * @brief Funzione di inizializzazione.
 *
 * @details E' la prima funzione ad essere eseguita nel main program. Si occupa di:
 * 			 - Inizializzare la libreria HAL;
 * 			 - Configurare il system clock;
 * 			 - Inizializzare e configurare tutte le periferiche utilizzate;
 * 			 - Inizializzare le variabili ed eseguire le funzioni richieste al reset del sistema.
 */
void setup(void);

/**
 * @brief Funzione che implementa la logica del programma.
 *
 * @details Ogni 5 secondi si valuta lo stato in cui si trova la macchina: se lo stato è RUNNING, si effettua una misura di temperatura e il risultato è
 * 			visualizzato sul display LCD esterno. Lo stato è poi riportato a IDLE, così da poter accettare un'altra richiesta.
 *
 */
void loop(void);

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;	//!< Handle della struttura SPI che sara' inizializzata.
HD44780_LCD_t lcd;			//!< Handle della struttura HD44780 che sara' inizializzata.
StateTypeDef stato;			//!< Variabile di stato che contiene lo stato corrente dell'elaborazione

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
  MX_SPI1_Init();
  LCD_Init();
/* Azioni e inizializzazioni al reset */
  stato = IDLE;											// Inizializza lo stato della macchina ad IDLE
  visualizzaTemperatura(&lcd,0);						// Stampa sul display il valore di temperatura di default (0 °C);
}

void loop(void){
// ... in attesa della pressione del BUTTON USER e conseguente cambio di stato IDLE => RUNNING

  HAL_Delay(5000);									// tempo di aggiornamento del display LCD pari a 5 secondi;
  if(stato==RUNNING){								// se lo stato è settato a RUNNING, si procede con la nuova misurazione della temperatura:
	  // a) viene richiesto al master di effettuare una misura di temperatura e ritornare il risultato
	  // b) viene aggiornato il valore di temperatura sul display LCD con quello appena ricevuto dallo Slave
	  visualizzaTemperatura(&lcd,richiediTemperatura(&hspi1));
	  // c) lo stato ritorna IDLE, alla prossima pressione del button user sara' effettuata una nuova misurazione
	  stato = IDLE;
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	(stato == IDLE ? stato = RUNNING : 0);			// se lo stato è IDLE, cambio di stato da IDLE -> RUNNING, altrimenti NOP
}

uint8_t richiediTemperatura(SPI_HandleTypeDef *hspi){

	uint8_t *txBuffer = (uint8_t *) malloc(1);
	uint8_t *rxBuffer = (uint8_t *) malloc(1);
	*txBuffer = 0xFF;								// codice di start misurazione Temperatura
	*rxBuffer = 0;									// reset del contenuto di rxBuffer
	spiTransaction(hspi,txBuffer,rxBuffer);			// transazione SPI 1, invio il codice operazione e ricevo un dummy byte
	BSP_LED_Toggle(LED2);							// utilizzato per "debug visivo" su board nucleo

	HAL_Delay(200);									// mi assicura la terminzaione dell'operazione lato slave

	uint8_t temp = 0;
	*txBuffer = 0;			//dummy byte			// reset del contenuto di txBuffer, utilizzato come dummy byte
	spiTransaction(hspi,txBuffer,rxBuffer);			// transazione SPI 2, invio un dummy byte e ricevo il valore di temparatura misurato
	temp = *rxBuffer;								// leggo il valore di temperatura dal registro di ricezione
	BSP_LED_Toggle(LED2);							// utilizzato per "debug visivo" su board nucleo

	free(txBuffer);
	free(rxBuffer);
	return temp;
}

void visualizzaTemperatura(HD44780_LCD_t *lcd, uint8_t temp){
	HD44780_Clear(lcd);						// pulisce il registro dato del display LCD
	HD44780_Print(lcd,"Temperatura :");
	HD44780_MoveToRow2(lcd);				// mi sposto alla seconda riga del display
	char str[10];   // assicurarsi di allocare sufficiente spazio per la stampa del numero
	sprintf(str,"%u C", temp);
	HD44780_Print(lcd,str);					// stampa della temperatura acquisita sul display
	HD44780_CursorOff(lcd);					// disabilita il cursore sul display
}

void spiTransaction(SPI_HandleTypeDef *hspi, uint8_t *spiTxBuffer, uint8_t *spiRXBuffer){
	if(HAL_SPI_TransmitReceive(hspi,spiTxBuffer,spiRXBuffer,1,1000) != HAL_OK){
		BSP_LED_Toggle(LED2);								// error code
	}
}

/* System Clock Configuration */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}


static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

static void LCD_Init(void){
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	HD44780_Init8_v2(
			&lcd,
			LCD_RS_GPIO_Port,LCD_RS_Pin,
			LCD_RW_GPIO_Port,LCD_RW_Pin,
			LCD_E_GPIO_Port,LCD_E_Pin,
			LCD_D0_GPIO_Port,LCD_D0_Pin,
			LCD_D1_GPIO_Port,LCD_D1_Pin,
			LCD_D2_GPIO_Port,LCD_D2_Pin,
			LCD_D3_GPIO_Port,LCD_D3_Pin,
			LCD_D4_GPIO_Port,LCD_D4_Pin,
			LCD_D5_GPIO_Port,LCD_D5_Pin,
			LCD_D6_GPIO_Port,LCD_D6_Pin,
			LCD_D7_GPIO_Port,LCD_D7_Pin
			);

  HD44780_Clear(&lcd);
}

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
