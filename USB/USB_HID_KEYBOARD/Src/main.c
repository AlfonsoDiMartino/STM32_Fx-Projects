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
 * @defgroup KeyBoard
 * @{
 *
 * @brief Realizzazione di una periferica USB HID KeyBoard
 *
 * @details Progetto di una periferica usb hid che definisce un device di tipo tastiera.<br>
 * 			 La tastiera permette di dare come comandi le frecce a destra, sinistra, sopra e sotto, oltre che al tasto spazio. <br>
 * 			 Inclinando il dispositivo in avanti, a sinistra, destra o all'indietro diamo un detirminato valore delle frecce direzionali.
 * 			 Ciò viene reso possibile attraverso l'accellerometro a bordo, che riporta le accellerazioni angolari lungo i tre assi X, Y, Z.
 * 			 I valori di accellerazione angolare letti vengono processati tenendo conto di opportuni valori di #soglia e sensitivita'  che sono personalizzabili
 * 			 a seconda delle esigenze di progetto o semplicemente a seconda della sensibilita' percepita e voluta dall'utente. <br>
 * 			 La pressione del tasto blu della board (User Button) viene acquisita come pressione del tasto space della tastiera. <br>
 */

/* USB HID Usage Table */
		/* USAGE NAME */			/*USAGE ID HEX */
#define USB_HID_KEY_LEFT_ARROW			0x50		//!< Usage ID associato alla freccia sx
#define USB_HID_KEY_RIGHT_ARROW			0x4F		//!< Usage ID associato alla freccia dx
#define USB_HID_KEY_DOWN_ARROW			0x51		//!< Usage ID associato alla freccia dw
#define USB_HID_KEY_UP_ARROW			0x52		//!< Usage ID associato alla freccia up
#define USB_HID_KEY_SPACEBAR 			0x2C		//!< Usage ID associato alla barra aspaziatrice


/**
 * @brief System Clock Configuration
*/
void SystemClock_Config(void);
/**
 * @brief Funzione di configurazione ed inizializzazione delle periferiche GPIO utilizzate.
 */
static void MX_GPIO_Init(void);

/**
 * @brief Struttura che astrae il device keyBoard.
 */
struct keyboardHID_t {
    uint8_t id;				//!< Id della Keyboard
    uint8_t modifiers;		//!< Valore di un tasto modificatore della KeyBoard
    uint8_t key1;			//!< Valore di un tasto della KeyBoard
    uint8_t key2;			//!< Valore di un tasto della KeyBoard
    uint8_t key3;			//!< Valore di un tasto della KeyBoard
};

/**
 * @brief Struttura che astrae le 3 componenti di accellerazione angolare lungo gli assi X, Y, Z.
 */
typedef struct {
	int16_t asse_x;			//!< Componente di accellerazione angolare lungo l'asse X
	int16_t asse_y;			//!< Componente di accellerazione angolare lungo l'asse Y
	int16_t asse_z;			//!< Componente di accellerazione angolare lungo l'asse Z
}XYZ_t ;

int soglia=64;				//!< Soglia che permette di regolare lo la sensibilità minima del dispositivo.
XYZ_t XYZ;					//!< Oggetto di tipo XYZ_t
static int16_t value_x,value_y;

/**
 * @brief Funzione che implementa la logica del programma.<br>
 *
 * @details
 * 			Vengono inizializzate le librerie HAL, configurato il system clock, inizializzate e configurate tutte le periferiche utilizzate,
 * 			inizializzate le variabili ed eseguite le funzioni richieste al reset del sistema. <br>
 * 			 - viene valutata la pressione del tasto blu (button User) e aggiornato il campo buttons dell'oggetto mouseHID;
 * 			 - viene effettuata la lettura delle componenti di accellerazione angolare lungo i 3 assi (sfruttando l'accellerometro a bordo) aggiornati i campi
 * 			 dell'oggetto muoseHID;
 * 			 - vengono aggiornati i campi x e y del'oggetto mouseHID. Questo è fatto tenendo conto del valore di soglia e sensitivita'
 * 			 (s) impostato;
 * 			 - viene invocata la funzione "uint8_t USBD_HID_SendReport(USBD_HandleTypeDef  *pdev, uint8_t *report,uint16_t len)", tramite la quale viene comunicato
 * 			 il report contenente i dati acquisiti;
 * 			 - prima di uscire vengono resettati tutti i valori della struttura mouseHID.
 */
int main(void)
{

	struct keyboardHID_t keyboardHID;
	keyboardHID.id = 1;
	keyboardHID.modifiers = 0;
	keyboardHID.key1 = 0;
	keyboardHID.key2 = 0;
	keyboardHID.key3 = 0;


	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_USB_DEVICE_Init();
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);
	BSP_PB_Init(BUTTON_KEY,BUTTON_MODE_GPIO);
	BSP_ACCELERO_Init();
	value_x=0;
	value_y=0;


  while (1)
  {
	  BSP_ACCELERO_GetXYZ((int16_t*)&XYZ);
	  value_x=XYZ.asse_x;
	  value_y=XYZ.asse_y;


/* Button User */
	  if(BSP_PB_GetState(BUTTON_KEY)!=GPIO_PIN_SET)
		  keyboardHID.key3=0;
	  else
		  keyboardHID.key3=USB_HID_KEY_SPACEBAR;

/* Accellerazione lungo l'asse y */
	  if (value_y>soglia){
		  BSP_LED_Toggle(LED3);
		  keyboardHID.key1 = USB_HID_KEY_UP_ARROW;
	  }
	  else if (value_y<-soglia){
		  BSP_LED_Toggle(LED6);
		  keyboardHID.key1 = USB_HID_KEY_DOWN_ARROW;
	  }

  /* Accellerazione lungo l'asse x */
	  if (value_x>soglia){
		  BSP_LED_Toggle(LED5);
		  keyboardHID.key2 = USB_HID_KEY_RIGHT_ARROW;
	  }
	  else if (value_x<-soglia){
		  BSP_LED_Toggle(LED4);
		  keyboardHID.key2 = USB_HID_KEY_LEFT_ARROW;
	  }

/* Invio Report */
	  USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t*)&keyboardHID, sizeof(struct keyboardHID_t));
	  HAL_Delay(25);

/* Reset dei valori della struttura keyboardHID*/
	 keyboardHID.key1 = 0;
	 keyboardHID.key2 = 0;
	 keyboardHID.key3 = 0;

/* Reset Led */
	  for(int i=0; i<4;i++)
	  		  BSP_LED_Off(i);
/* Reset dei valori in attesa della prossima misurazione */
	  value_x=0;
	  value_y=0;
	  HAL_Delay(25);
  }
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


