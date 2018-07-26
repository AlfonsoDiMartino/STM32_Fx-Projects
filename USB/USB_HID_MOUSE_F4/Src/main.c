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
#include "stm32f4xx_hal.h"
#include "usb_device.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_accelerometer.h"
#include "usbd_hid.h"

/**
 * @defgroup USBD
 * @{
 * @defgroup HID
 * @{
 * @defgroup Mouse
 * @{
 *
 * @brief Realizzazione di una periferica USB HID Mouse
 *
 * @details Progetto di una periferica usb hid che definisce un device di tipo mouse.<br>
 * 			 Per gli spostamenti del cursore viene utilizzato l'accellerometro a bordo, che riporta le accellerazioni angolari lungo i tre assi X, Y, Z.
 * 			 I valori di accellerazione angolare letti vengono processati tenendo conto di opportuni valori di #soglia e sensitivita' (#s) che sono personalizzabili
 * 			 a seconda delle esigenze di progetto o semplicemente a seconda della sensibilita' percepita e voluta dall'utente. <br>
 * 			 La pressione del tasto blu della board (User Button) viene acquisita come pressione del tasto sinistro del mouse. <br>
 */

/* Private variables ---------------------------------------------------------*/
/**
 * @brief Struttura che astrae le 3 componenti di accellerazione angolare lungo gli assi X, Y, Z.
 */
typedef struct{
	int16_t asseX;		//!< Componente di accellerazione angolare lungo l'asse X
	int16_t asseY;		//!< Componente di accellerazione angolare lungo l'asse Y
	int16_t asseZ;		//!< Componente di accellerazione angolare lungo l'asse Z
}accellero_t;

/**
 * @brief Struttura che astrae il device Mouse.
 */
typedef struct {
    uint8_t buttons;	//!< Riporta la pressione del tasto sinistro del mouse
    int8_t x;			//!< Riporta lo spostamento lungo l'asse x
    int8_t y;			//!< Riporta lo spostamento lungo l'asse y
    int8_t wheel;		//!< Riporta lo scroll del mouse
}mouseHID_t;

/**
 * @brief Valore di sensitivita' con cui il valore della componente di accellerazione è scalato.
 *
 * @details Tale valore determina la sensibilità percepita dall'utente del device.<br>
			E' anche necessario per scalare correttamente su 8 bit il valore della componente accellerazione che invece è espresso su 16 bit.<br>
 */
#define s 0.1

/**
 * @brief Soglia che permette di regolare lo la sensibilità minima del dispositivo.
 *
 * @details La regolazione della soglia permette di filtrare i valori della componente accellerazione che rientrano nel range di valori di riposo,
 	 	 	 	 	 	 	 ossia valori per i quali il cursore deve restare fermo.<br>
 	 	 	 	 	 	 	 Per esempio :
 	 	 	 	 	 	 	  - asseX : se -soglia < asseX < soglia, allora considero pari a zero la componente asseX;
 	 	 	 	 	 	 	  - asseY : se -soglia < asseY < soglia, allora considero pari a zero la componente asseY;
 */
#define soglia 64

/* Private function prototypes -----------------------------------------------*/
/**
 * @brief System Clock Configuration
*/
void SystemClock_Config(void);
/**
 * @brief Funzione di configurazione ed inizializzazione delle periferiche GPIO utilizzate.
 */
static void MX_GPIO_Init(void);
/**
 * @brief Funzione di inizializzazione.<br>
 *
 * @details E' la prima funzione ad essere eseguita nel main program. Si occupa di:
 * 			 - inizializzare la libreria HAL;
 * 			 - configurare il system clock;
 * 			 - inizializzare e configurare tutte le periferiche utilizzate;
 * 			 - inizializzare le variabili ed eseguire le funzioni richieste al reset del sistema.
 */
void setup(void);
/**
 * @brief Funzione che implementa la logica del programma.<br>
 *
 * @details Ogni 50 milli secondi :
 * 			 - viene valutata la pressione del tasto blu (button User) e aggiornato il campo buttons dell'oggetto mouseHID;
 * 			 - viene effettuata la lettura delle componenti di accellerazione angolare lungo i 3 assi (sfruttando l'accellerometro a bordo) aggiornati i campi
 * 			 dell'oggetto muoseHID;
 * 			 - vengono aggiornati i campi x e y del'oggetto mouseHID. Questo è fatto tenendo conto del valore di soglia e sensitivita'
 * 			 (s) impostato;
 * 			 - viene invocata la funzione "uint8_t USBD_HID_SendReport(USBD_HandleTypeDef  *pdev, uint8_t *report,uint16_t len)", tramite la quale viene comunicato
 * 			 il report contenente i dati acquisiti;
 * 			 - prima di uscire vengono resettati tutti i valori della struttura mouseHID.
 */
void loop(void);

mouseHID_t mouseHID;		//!< Oggetto di tipo mouseHID_t
accellero_t accellero;		//!< Oggetto di tipo accellero_t

int8_t value_x;				//!< Rappresenta lo spostamento lungo l'asse x del mouse
int8_t value_y;				//!< Rappresenta lo spostamento lungo l'asse y del mouse

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
  MX_USB_DEVICE_Init();
  BSP_PB_Init(BUTTON_KEY,BUTTON_MODE_GPIO);
  BSP_ACCELERO_Init();
  for(int i=0;i<LEDn;i++)
	  BSP_LED_Init(i);

  /* Inizializza la struttura MouseHID */
  mouseHID.buttons=0x00;
  mouseHID.x = 0;
  mouseHID.y = 0;
  mouseHID.wheel = 0;
}

void loop(void){
  /* Valuta la pressione del tasto blu (Button User)*/
	if(BSP_PB_GetState(BUTTON_KEY)!= GPIO_PIN_SET){
		  mouseHID.buttons=0x00;						// se il tasto blu non viene premuto, mouseHID.buttons = 0x00
		  for(int i=0; i<4;i++)							// Reset di tutti i led
			  BSP_LED_Off(i);
	}
	else{
		mouseHID.buttons=0x01;							// se il tastino blu viene premuto, mouseHID.buttons = 0x01 (click tasto sx del mouse)
		for(int i=0;i<LEDn;i++)							// toggle su tutti i led per un debug visivo della pressione del tasto
			BSP_LED_Toggle(i);
	}

	BSP_ACCELERO_GetXYZ((int16_t*)&accellero);			// lettura delle componenti di accellerazione angolare lungo i 3 assi

	value_x = (int8_t)(accellero.asseX*s);				// scaling della componente di accellerazione angolare lungo l'asse x
	value_y = (int8_t)(accellero.asseY*s);				// scaling della componente di accellerazione angolare lungo l'asse y

/* Valutazione delle soglie e aggiornamento dei campi x e y dell'oggetto mouseHID */
  /* Accellerazione lungo l'asse y */
	if (accellero.asseX>soglia){
		BSP_LED_Toggle(LED5);
		mouseHID.x = -value_x; 							// opposto perchè la board è rivolta con il button user verso le dita
		}
	else if (accellero.asseX<-soglia){
		BSP_LED_Toggle(LED4);
		mouseHID.x = -value_x; 							// opposto perchè la board è rivolta con il button user verso le dita
		}

	/* Accellerazione lungo l'asse x */
	if (accellero.asseY>soglia){
		BSP_LED_Toggle(LED3);
		mouseHID.y = -value_y;
	}
	else if (accellero.asseY<-soglia){
		BSP_LED_Toggle(LED6);
		mouseHID.y = -value_y;
	}
  /* Send HID Report */
	USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t*)&mouseHID,sizeof(mouseHID_t));		// viene inviato il report, contenente i dati acquisiti per l'oggetto mouseHID

	HAL_Delay(50);

  /* Reset dei valori della struttura mouseHID*/
	mouseHID.x = 0;
	mouseHID.y = 0;
	mouseHID.wheel = 0;
}


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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
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
 * @} @} @}
 */
/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
