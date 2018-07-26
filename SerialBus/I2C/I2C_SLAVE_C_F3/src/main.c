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

/**
 * @addtogroup busSeriali
 * @{
 * @addtogroup I2C
 * @{
 * @addtogroup I2C_Slave_C
 * @{
 *
 * @brief Implementazione di un dispositivo Slave che effettua la somma dei due valori ricevuti dai 2 dispositivi Master.
 *
 * @details
 * 			Il dispositivo somma i valori ricevuti dai 2 Master e ne visualizza il risultato,in codifca binaria, sugli 8 led a bordo.
 * 			La comunicazione con i dispositivi Master avviene attraverso una comunicazione seriale I2C.
 */


#include "stm32f3xx.h"
#include "stm32f3_discovery.h"
#include "stdlib.h"

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
 * @details Il dispositivo Slave resta in attesa di ricevere un valore da uno dei due Master (receive bloccante). Una volta ricevuto, effettua la somma con
 * 			l'altro addendo (o valore 0 di default se l'altro addendo non è mai stato ricevuto). <br>
 * 			Il risultato della somma è codificato in binario e mostrato sugli 8 led a bordo.
 *
 */
void loop();

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 72000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            HSE PREDIV                     = 1
  *            PLLMUL                         = RCC_PLL_MUL9 (9)
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void);

/**
 * @brief Funzione eseguita nel caso in cui viene riscontrato un errore.
 *  Attiva il Toggle sul LED6 ed entra in un loop infinito.
 *  @param none
 *  @retval none
 */
static void Error_Handler(void);

/**
 * @brief Funzione di Init della struttura I2C_HandleTypeDef.
 * 	Definisce i parametri della comunicazione I2C per la specifica periferica.
 * 	Nello specifico, l'istanza è I2C1, modalita' Fast Mode con I2C Speed Frequency a 400KHz, indirizzo di periferica a 7 bit.
 * @param none
 * @retval none
 */
static void I2Cx_Init(void);

/**
 * @brief I2C MCU Specific Package.
 *  Questa funzione configura le risorse hardware usate:
 *   - configurazione della periferica GPIO;
 *   - abilita il clock dell' interfaccia I2Cx;
 *   - configura NVIC per I2C;
 *
 * @param[inout] hi2c: puntatore alla struttura I2C_HandleTypeDef, inizializzata con i parametri della comunicazione.
 * @retval none
 */
static void I2Cx_MspInit(I2C_HandleTypeDef *hi2c);


#define I2C_ADDRESS_C 0x0C <<1		//!< Indirizzo del dispositivo Slave C

I2C_HandleTypeDef I2cHandle; 		//!< Handle della struttura I2C che sarà utilizzata.
uint8_t rxBuffer[2];				//!< Buffer di ricezione utilizzato per la comunicazione I2C.
int add1;							//!< Variabile addendo 1.
int add2;							//!< Variabile addendo 2.
int sum;							//!< Variabile somma.


/**
 * @brief Main Program
 * @param none
 * @retval none
 *
 */
int main(void)
{
	setup();
	for(;;) loop();
}

/**
 * @brief Effettua il mapping tra un valore intero (da 0 a 31) e la sua codifica in binario su 5 bit.
 *	La codifica è data dall'accension di 5 degli 8 led presenti sulla board STM32F3.
 *	La corrispondenza bitX => LEDx è la seguente:
 *	- bit0 => LED4;
 *	- bit1 => LED3;
 *	- bit2 => LED5;
 *	- bit3 => LED7;
 *	- bit4 => LED9;
 *	- bit5 => LED10;
 * @code
 * HAL_GPIO_WritePin(GPIOE,SUM_BIN[7],GPIO_PIN_SET); //Accende LED5, LED3, LED4 (MSB->LSB)
 * @endcode
 */
uint16_t SUM_BIN[32] = {
	0x0000,
	0x0100,
	0x0200,
	0x0300,
	0x0400,
	0x0500,
	0x0600,
	0x0700,
	0x0800,
	0x0900,
	0x0A00,
	0x0B00,
	0x0C00,
	0x0D00,
	0x0E00,
	0x0F00,
	0x1000,
	0x1100,
	0x1200,
	0x1300,
	0x1400,
	0x1500,
	0x1600,
	0x1700,
	0x1800,
	0x1900,
	0x1A00,
	0x1B00,
	0x1C00,
	0x1D00,
	0x1E00,
	0x1F00
};


void setup(){
	HAL_Init();
	SystemClock_Config();
	for(int i=0; i< LEDn; i++)
		BSP_LED_Init(i);
	I2Cx_Init();
	add1=0;
	add2=0;
	sum=0;
}

void loop(){
	if(HAL_OK == HAL_I2C_Slave_Receive(&I2cHandle,rxBuffer,2,500)){
		switch (rxBuffer[0]){
		case 'A':
			add1=rxBuffer[1];
			break;
		case 'B':
			add2=rxBuffer[1];
			break;
		}
		sum = add1+add2;
		HAL_GPIO_WritePin(GPIOE,SUM_BIN[sum],GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,~SUM_BIN[sum],GPIO_PIN_RESET);
	}
}


static void SystemClock_Config(void)
  {
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct)!=HAL_OK){
    	Error_Handler();
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
  	 clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)!=HAL_OK){
    	Error_Handler();
    }
  }

static void Error_Handler(void){
	/* Error Code */
	BSP_LED_Toggle(LED6);
	while(1){}
}

static void I2Cx_Init(void)
{
  if(HAL_I2C_GetState(&I2cHandle) == HAL_I2C_STATE_RESET)
  {
	I2cHandle.Instance = I2C1;
	//I2cHandle.Init.Timing = 0x00C4092A;
	I2cHandle.Init.Timing = 0x0000020C; //Fast Mode, 400KHz, RiseTime 0ns, FallTime 0ns
    I2cHandle.Init.OwnAddress1 =  I2C_ADDRESS_C;
    I2cHandle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    I2cHandle.Init.OwnAddress2 = 0;
    I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    I2cHandle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    /* Init the I2C */
    I2Cx_MspInit(&I2cHandle);
    HAL_I2C_Init(&I2cHandle);
  }
}

static void I2Cx_MspInit(I2C_HandleTypeDef *hi2c){

  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable SCK and SDA GPIO clocks */
  DISCOVERY_I2Cx_GPIO_CLK_ENABLE();

  /* I2Cx SD1 & SCK pin configuration */
  GPIO_InitStructure.Pin = (DISCOVERY_I2Cx_SDA_PIN | DISCOVERY_I2Cx_SCL_PIN);
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_PULLDOWN;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStructure.Alternate = DISCOVERY_I2Cx_AF;
  HAL_GPIO_Init(DISCOVERY_I2Cx_GPIO_PORT, &GPIO_InitStructure);

  /* Enable the I2C clock */
  DISCOVERY_I2Cx_CLK_ENABLE();

  /* Configure the NVIC for I2C */
	/* NVIC for I2C1 */
	HAL_NVIC_SetPriority(I2C1_ER_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
	HAL_NVIC_SetPriority(I2C1_EV_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
}

/**
 * @}
 * @}
 * @}
 */
