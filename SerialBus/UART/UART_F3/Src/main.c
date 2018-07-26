/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f3xx_hal.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <math.h>
#include <string.h>

/**
 * @addtogroup busSeriali
 * @{
 * @addtogroup UART
 * @{
 * @addtogroup UART_F3
 * @{
 *
 * @brief Implementazione dell' EOP UART F3 che comunica tramite UART1 con il EOP UART PC e tramite UART2 con l'EOP F4.
 *
 * @details
 * 			Il dispositivo viene configurato (per quanto riguarda il numero di campioni da acquisire e il formato della comunicazione dei valori acquisiti)
 * 			tramite comunicazione su UART1 da parte dell'EOP UART PC. Tale configurazione viene poi inoltrata all'EOP UART F4 che rappresenta il dispositivo
 * 			che effettivamente fa la misura.
 * 			 - Per quanto riguarda la configurazione, il dispositivo riceve informazioni riguardo il numero di cifre per campione e il numero di campioni da acquisire
 * 			(queste due "fasi" sono rispettivamente associate ai due stati della macchina chiamati ATTESA_N_CIFRE ed ATTESA_N_SAMPLES) su UART1, PC-->F3.
 * 			 - Una volta ricevuti i parametri della configurazione, l'EOP UART F3 inoltra gli stessi parametri al dispositivo a valle, così da concordare con
 * 			 quest'ultimo i paramentri della comunicazione su UART2. Ho quindi i due stati INVIOCONFNCIFRE e INVIOCONFNSAMPLES.
 * 			 - Configurati i parametri relativi ai campioni su UART2, ci si mette in attesa di ricevere i nuovi parametri dei dati, diponibili non appena l'EOP UART F4
 * 			 ha terminato il campionamento con ADC. Ho quindi i due stati RICEZCONFNCIFRE e RICEZCONFNSAMPLES.
 * 			 - A questo punto ci si mette in attesa di ricevere lo stream di dati campionati dall'EOP UART F4 (stato RICEZDATA) su UART2, per poi concludere
 * 			 con l'inoltro al PC degli stessi dati (stato INVIOPC) su UART1.
 * 			 I dati campionati sono poi processati e graficati da un algoritmo matlab, il quale è responsabile anche della definizione iniziale dei parametri
 * 			 dei campioni.
 * 			La comunicaione tra i tre EOP Uart avviene tramite bus seriale UART, in particolare PC--UART1-->F3 e F3--UART2-->F4.
 */

#define MAXBUF          10				//!< Dimensione max del buffer di comunicazione
#define BUFADC          5000			//!< Dimensione del buffer che contiene i caratteri campionati dall' EOP F4
#define MAXADC          1000			//!< Dimensione max dei campioni nel buffer ADC
#define INVIAACAPO      HAL_UART_Transmit(&huart1,"\r\n",2,10000)	//!< Macro "Invia a Capo", trasmette i caratteri di invia a capo per il debug
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;				//!< Handle della struttura uart1 che sarà inizializzato
UART_HandleTypeDef huart2;				//!< Handle della struttura uart2 che sarà inizializzato

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint8_t buffer[MAXBUF];  				//!< Buffer di comunicazione dei messaggi
char myChar;							//!< Carattere generico
char bufferADC[BUFADC];					//!< Buffer che contiene i caratteri convertiti ricevuti da UART F4 e successivamente trasmessi al PC
unsigned short int nSamp;				//!< Numero di campioni

/**
 * @brief Stati di esecuzione della macchina
 */
enum StatoF3 {			WELCOME, 		//!< Stato di Reset della macchina
					ATTESA_N_CIFRE, 	//!< In attesa su UART1 di conoscere il numero di cifre dei campioni da acquisire.	PC-->F3
					ATTESA_N_SAMPLES,	//!< In attesa su UART1 di conoscere il numero di campioni da acquisire. 			PC-->F3
					INVIOCONFNCIFRE,	//!< Invio su UART2 del numero di cifre di campioni da acquisire. 					F3-->F4
					INVIOCONFNSAMPLES,	//!< Invio su UART2del numero di campioni da acquisire.								F3-->F4
					RICEZCONFNCIFRE,	//!< In attesa su UART2 di conoscere il numero di cifre da acquisire da UART F4		F4-->F3
					RICEZCONFNSAMPLES,	//!< In attesa su UART2 di conoscere il numero di campioni da acquisire da UART F4	F4-->F3
					RICEZDATA, 			//!< Ricezione su UART2  dei dati acquisiti nel formato XXXX;						F4-->F3
					INVIOPC				//!< Invio su UART1 dei dati acquisiti al PC. 										F3-->PC
} myState;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/**
 * @brief Funzione di configurazione ed inizializzazione delle periferiche GPIO utilizzate.
 */
static void MX_GPIO_Init(void);

/**
  * @brief Funzione di abilitazione ed inizializzazione della periferica UART1
  */
static void MX_USART1_UART_Init(void);

/**
  * @brief Funzione di abilitazione ed inizializzazione della periferica UART2
  */
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
  myState = WELCOME;						// Stato iniziale della macchina
  unsigned short int app = 0;				// Variabile di appoggio
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
	  switch (myState) {
	  	  case WELCOME:
	  	        HAL_UART_Transmit(&huart1,"Attesa configurazione\r\n",23,10000);	 //Scrive sul terminale del PC un messaggio
	  	  	  myState = ATTESA_N_CIFRE;		//Prossimo stato
	  	  	  break;
	  	  case ATTESA_N_CIFRE:
	  	        if (HAL_UART_Receive(&huart1,buffer,1,10000)!=0) 	// UART1 Attesa numero di cifre 			PC-->F3
	  	        	break;
	  #ifdef MYDEBUG
	  	        INVIAACAPO;
	  			HAL_UART_Transmit(&huart1,buffer,1,10000);							// Per il debug su terminale
	  #endif
	  			myState = ATTESA_N_SAMPLES;		//Prossimo stato
	  	        break;

	  	      case ATTESA_N_SAMPLES:
	  	    	app = buffer[0]-0x30;	//Cambio di formato da char a int, poichè ho bisogno di definire la dimensione del buffer
	  	    	memset(buffer,0,MAXBUF*sizeof(char));	//reset del buffer
	  #ifdef MYDEBUG
	  	        INVIAACAPO;
	  	    	HAL_UART_Transmit(&huart1,"Attesa numero campion\r\n",23,10000);	// Per il debug su terminale
	  #endif
	  	    	if(HAL_UART_Receive(&huart1,buffer,app,10000)!=0)		// UART1 Attesa numero campioni			PC-->F3
	  	        	break;
	  #ifdef MYDEBUG
	  	        INVIAACAPO;
	  	        HAL_UART_Transmit(&huart1,buffer,app,10000);						// Per il debug su terminale
	  #endif
	  	        nSamp = atoi(buffer);	//Trasformo il contenuto del buffer nel numero di campioni che mi servono, atoi converte caratteri in numeri interi
	  	        myState = INVIOCONFNCIFRE;	//Prossimo stato
	  	        break;

	  	      case INVIOCONFNCIFRE:	//vado a dire al ricevente quante cifre √® il numero di campioni e poi gli do il numero di campioni

	  	        if(HAL_UART_Transmit(&huart2,&app,1,10000)!=0)	// UART2 Invio del numero di cifre 	F3-->F4
	  	        	break;
	  #ifdef MYDEBUG
	  	        INVIAACAPO;
	  	        HAL_UART_Transmit(&huart1,"Buffer Trasmesso\r\n",16,10000);			// Per il debug su terminale
	  #endif
	  	        HAL_Delay(100);	//timeout di 100 millisecondi
	  	        myState = INVIOCONFNSAMPLES;	//Prossimo stato
	  	        break;
	  	      case INVIOCONFNSAMPLES:
	  	        sprintf(buffer,"%d",nSamp);		//copia nel buffer nSamp
	  	        if(HAL_UART_Transmit(&huart2,buffer,app,10000)!=0)	// UART2 Trasmette all'EOP F4 il numero di campioni  F3-->F4
	  	        	break;
	  #ifdef MYDEBUG
	  	        INVIAACAPO;
	  	        HAL_UART_Transmit(&huart1,"Numero di campioni \r\n",21,10000);  // Per il debug su terminale
	  	        HAL_UART_Transmit(&huart1,buffer,app,10000); //Trasmetto il numero di campioni
	  #endif
	  	        myState = RICEZCONFNCIFRE;	//Prossimo stato
	  	        break;

	  	      case RICEZCONFNCIFRE:
	  	        if(HAL_UART_Receive(&huart2,buffer,1,10000)!=0)	// UART2 Attesa numero cifre			F3-->F3
	  	        	break;
	  #ifdef MYDEBUG
	          	INVIAACAPO;
	  	        HAL_UART_Transmit(&huart1,"Numero di cifre \r\n",18,10000);
	  #endif
	  	        uint8_t dato = buffer[0]+0x30;			//converto l'intero in carattere
	  #ifdef MYDEBUG
	  	        HAL_UART_Transmit(&huart1,&dato,1,10000);  // Per il debug su terminale
	  	        INVIAACAPO;
	  #endif
	  	        myState = RICEZCONFNSAMPLES;	//Prossimo stato
	  	        break;

	  	      case RICEZCONFNSAMPLES:
	  	        app = buffer[0];	//in buffer[0] contiene la dimensione del buffer di ricezione
	  	        memset(buffer,0,MAXBUF*sizeof(char));	//azzero il buffer

	  	        if(HAL_UART_Receive(&huart2,buffer,app,10000)!=0) // UART2 Attesa numero campioni 	F4->F3
	  	        	break;
	  #ifdef MYDEBUG
	  	        INVIAACAPO;
	  	        HAL_UART_Transmit(&huart1,"Numero di campioni \r\n",21,10000);
	  	        HAL_UART_Transmit(&huart1,buffer,app,10000); // Per il debug su terminale
	  	        INVIAACAPO;
	  #endif
	  	        app = atoi(buffer);	//Trasformo il contenuto del buffer nel numero di campioni che mi servono, atoi converte caratteri in numeri interi
	  	        myState = RICEZDATA;	//Prossimo stato
	  	        break;

	  	      case RICEZDATA:
	  	        if(HAL_UART_Receive(&huart2,bufferADC,app,10000)!=0) 	// UART2 Attesa fine ricezione dei dati campionati	F4->F3
	  	        	break;
	  #ifdef MYDEBUG
	  	        INVIAACAPO;
	  	        HAL_UART_Transmit(&huart1,"Ricevuto Buffer\r\n",18,10000); // Per il debug su terminale
	  #endif
	  	        myState = INVIOPC;	//Prossimo stato
	  	        break;

	  	      case INVIOPC:
	  #ifdef MYDEBUG
	  	    	INVIAACAPO;
	  #endif
	  	    	HAL_UART_Transmit(&huart1,bufferADC,app,10000); 	// UART1 Invia al PC il buffer che contiene i caratteri campionati da ADC
	  	        INVIAACAPO;
	  	        HAL_Delay(1000);
	  	        myState = WELCOME;	//Ritorno allo stato di Reset
	  	        break;
	  	      default:
	  	        break;
	  	      }
	    }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

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

/** @} @} @} */
/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
