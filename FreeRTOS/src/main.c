/**
 * @file main.c
 * @author  Salvatore Barone <salvator.barone@gmail.com> ,
 *      Alfonso Di Martino <alfonsodimartino160989@gmail.com> ,
 *      Sossio Fiorillo <fsossio@gmail.com> ,
 *      Pietro Liguori <pie.liguori@gmail.com> .
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
 * @defgroup FreeRTOS
 * @{
 *
 * @brief Progetto FreeRTOS di un Orologio / Cronometro realizzato attraverso task periodici.
 *
 * @details Quattro task periodici, di periodo rispettivamente un millisecondo,
 * 			un secondo, 1 minuto e 1 ora, aggiornano le cifre di conteggio accedendo
 * 			a tali valori con mutua esclusione (utilizzo di mutex), realizzando un orologio. <br>
 * 			Un quinto task periodico, di periodo pari a un millisecondo, accede in lettura
 * 			ai valori aggiornati dagli altri quattro task (sempre con l'utilizzo di mutex)
 * 			ogni qual volta viene premuto il push button. Questo task assume così il ruolo di
 * 			Polling Server. <br>
 * 			Ogni volta che il server accede in lettura alle cifre di conteggio, invia il valore letto
 * 			ad un dispositivo lcd esterno ed effettua la differenza tra un tempo di inzio ed uno di fine,
 * 			realizzando di fatto la funzione di un cronometro.
 * 			Ai task sono assegnate priorità diverse (priorità maggiore al server). <br>
 *
*/

/*Inclusione librerie */
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "hd44780.h"
#include <string.h>


/*Define */
#define N_TASK 4										//!< numero di task
int TASK_PERIOD_MS[N_TASK]={100,1000,60000,3600000};	//!< periodi assegnati ai task contatori
#define SERVER_PERIOD_MS 100							//!< periodo del server



/**
 * @brief Funzione di inizializzazione.
 *
 * @details E' la prima funziona ad essere eseguita nel main program.<br>
 * Viene inizializzata la libreria HAL, configurate le periferiche da utilizzare
 * e inizializzato il display esterno su cui visualizzare l'output.<br>
 */
void Init(void);

/**
 * @brief Funzione di configurazione del clock del sistema.
 *
 * @details Viene richiamata nella funzione Init() all'interno del main program.<br>
 * Vengono settati tutti i parametri relativi al clock del sistema.<br>
 */
void SystemClock_Config(void);


/**
 * @brief Funzione che viene eseguita dai task creati.
 *
 * @details La funzione rende i task di tipo periodico, con periodo pari al periodo del contatore
 * passato come parametro di ingresso. In base alla priorità assegnata al task nella funzione
 * xTaskCreate, accedono a risorse condivise per poter aggiornare il valore del contatore.
 * L'id del contatore passato come parametro di ingresso specifica sia la cifra di conteggio da aggiornare
 * e sia la modalità di aggiornamento.
 * Gli accessi vengono regolati attraverso l'utilizzo di semafori. Ogni volta che viene acquisito
 * l'accesso alla risorsa, viene anche effettuato il toggle di uno specifico led associato all'id del
 * contatore. Dopodiché viene rilasciato il semaforo in attesa della successiva esecuzione
 * del task periodico.
 *
 * @param[in] parametri: il contatore.
 */
static void vPeriodTask(void *parametri);


/**
 * @brief Funzione che viene eseguita dal task server.
 *
 * @details La funzione rende i task di tipo periodico, con periodo pari al SERVER_PERIOD_MS  definito.
 * Ogni volta che viene premuto il button sulla board, il task accede al valore del contatori
 * in lettura, e invia tale valore a un dispositivo esterno che ne permetta la visualizzazione su
 * schermo. I valori letti vengono converititi da interi a stringa per poter essere stampati sul lcd
 * esterno attraverso le funzioni opportunamente scritte per esso.
 * Gli accessi vengono regolati attraverso l'utilizzo di semafori.
 * Dopodiché viene rilasciato il semaforo in attesa della successiva esecuzione del task periodico.<br>
 * Ogni volta che viene premuto il button vengono registrati e visualizzati sul display, rispettivamente,
 * un tempo di start, uno di finish ed il tempo che intercorre tra i due tempi.
 * Ripremendo di nuovo il button si ripete il procedimento.
 *
 * @param[in] parametri: contatore.<br>
 */
static void vPollingServer(void *parametri);


char* task_name[N_TASK] = { "Task0","Task1", "Task2", "Task3" }; 	// Identificativi dei task

SemaphoreHandle_t xSemaphore;
HD44780_LCD_t lcd;

//STRUTTURA ASSOCIATA AD OGNI TASK
/**
 * @brief Struttura del counter.
 */
typedef struct
{
	int id;  														//!< id del contatore
	int value;														//!< valore del conteggio
	int period;														//!< periodo del task
} counter;

static counter contatore[N_TASK];


int main()
{
	Init();
	for (int i = 0; i < N_TASK; i++) {
		contatore[i].id = i;
		contatore[i].value = 0;
		contatore[i].period = TASK_PERIOD_MS[i];
	}


	xSemaphore = xSemaphoreCreateMutex(); //creazione di un mutex

	// Creazione task server
	xTaskCreate(vPollingServer, "TaskServer", configMINIMAL_STACK_SIZE, (void*)&contatore, N_TASK+1, NULL);
	// Creazione N_TASK
	for (int i = 0; i <N_TASK; i++) {
		xTaskCreate(vPeriodTask, task_name[i], configMINIMAL_STACK_SIZE, (void*)&contatore[i], N_TASK-i, NULL);
	}

	vTaskStartScheduler(); //avvio dello scheduler
//	osKernelStart();

	vSemaphoreDelete(xSemaphore); // rimozione mutex

	//Rimozione Task
	for(int i=0; i< N_TASK; i++)
		vTaskDelete(NULL);

	for (;; );
	return 1;
}


void Init(){
	HAL_Init();
	SystemClock_Config();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	for (int i=0; i<N_TASK;i++)
		BSP_LED_Init(i);
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	HD44780_Init8_v2(&lcd,
							GPIOC,		GPIO_PIN_13,
							GPIOC,		GPIO_PIN_15,
							GPIOC,		GPIO_PIN_14,
							GPIOE,		GPIO_PIN_6,
							GPIOE,		GPIO_PIN_5,
							GPIOE,		GPIO_PIN_4,
							GPIOE,		GPIO_PIN_3,
							GPIOE,		GPIO_PIN_2,
							GPIOE,		GPIO_PIN_1,
							GPIOE,		GPIO_PIN_0,
							GPIOB,		GPIO_PIN_8);
	HD44780_Clear(&lcd);
	//HD44780_Print(&lcd,"prova");
}


static void vPeriodTask(void * parametri) {
	counter* contatore = (counter*)parametri;
	TickType_t xNextWakeTime;
	const TickType_t xBlockTime = pdMS_TO_TICKS(contatore->period); //converte il periodo da millisecondi a ticks
	xNextWakeTime = xTaskGetTickCount();  							//ritorna il conteggio dei ticks

	for (;; ) {
		vTaskDelayUntil(&xNextWakeTime, xBlockTime); //ritarda il task da "xNextWakeTime" di un periodo pari a "xBlockTime"
		xSemaphoreTake(xSemaphore, portMAX_DELAY);//acquisisce il mutex
		switch(contatore->id){
		case 0:
			contatore->value=(contatore->value+1)%10;
			break;
		case 1:
			contatore->value=(contatore->value+1)%60;
			break;
		case 2:
			contatore->value=(contatore->value+1)%60;
			break;
		case 3:
			contatore->value=(contatore->value+1)%24;
			break;
		default:
			contatore->value=(contatore->value+1)%10;
			break;
		}
		BSP_LED_Toggle(contatore->id);
		xSemaphoreGive(xSemaphore); //rilascia il mutex
	}
}





static void vPollingServer(void *parametri) {
	counter* contatore = (counter*)parametri;
	int row=1;
	char str[10];
	int value_1[N_TASK];
	int value_2[N_TASK];
	int value_3[N_TASK];
	TickType_t xNextWakeTime;
	const TickType_t xBlockTime = pdMS_TO_TICKS(SERVER_PERIOD_MS); //converte il periodo da millisecondi a ticks
	xNextWakeTime = xTaskGetTickCount();   //ritorna il conteggio dei ticks
	for (;;) {
		vTaskDelayUntil(&xNextWakeTime, xBlockTime);
		if (BSP_PB_GetState(BUTTON_KEY) != RESET) {
			// debounce and read again
			vTaskDelay(100);
			xSemaphoreTake(xSemaphore, portMAX_DELAY);
			if(row==1){
				HD44780_Clear(&lcd);
				HD44780_MoveToRow1(&lcd);
				HD44780_Print(&lcd,"S: ");
				for(int i=0; i<N_TASK;i++){
					value_1[N_TASK-1-i]=contatore[N_TASK-1-i].value;
					itoa(value_1[N_TASK-1-i],str,10); //converte il valore decimale in una stringa
					HD44780_Print(&lcd,str);
					HD44780_Print(&lcd,":");
				}
				row=2;
			}
			else if(row==2){
				HD44780_MoveToRow2(&lcd);
				HD44780_Print(&lcd,"F: ");
				for(int i=0; i<N_TASK;i++){
					value_2[N_TASK-1-i]=contatore[N_TASK-1-i].value;
					itoa(value_2[N_TASK-1-i],str,10); //converte il valore decimale in una stringa
					HD44780_Print(&lcd,str);
					HD44780_Print(&lcd,":");
				}
				row=3;
			}
			else {
				HD44780_Clear(&lcd);
				if (value_2[0]<value_1[0]){
					value_2[0]+=10;
					value_2[1]--;
				}
				if (value_2[1]<value_1[1]){
					value_2[1]+=60;
					value_2[2]--;
				}
				if (value_2[2]<value_1[2]){
					value_2[2]+=60;
					value_2[3]--;
				}

				HD44780_MoveToRow1(&lcd);
				HD44780_Print(&lcd, "Time: ");
				HD44780_MoveToRow2(&lcd);
				for(int i=0;i<N_TASK;i++){
					value_3[N_TASK-1-i]=value_2[N_TASK-1-i]-value_1[N_TASK-1-i];
					itoa(value_3[N_TASK-1-i],str,10); //converte il valore decimale in una stringa
					HD44780_Print(&lcd,str);
					HD44780_Print(&lcd,":");
				}
				row=1;
			}
			HD44780_MoveCursor(&lcd,0);
			HD44780_Print(&lcd," ");
			HD44780_CursorOff(&lcd);
			xSemaphoreGive(xSemaphore);
			vTaskDelay(20);
		// read again soon
			}
	}
}


void vApplicationMallocFailedHook(void) {
	/* vApplicationMallocFailedHook() will only be called if
	 configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	 function that will get called if a call to pvPortMalloc() fails.
	 pvPortMalloc() is called internally by the kernel whenever a task, queue,
	 timer or semaphore is created.  It is also called by various parts of the
	 demo application.  If heap_1.c or heap_2.c are used, then the size of the
	 heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	 FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	 to query the size of free heap space that remains (although it does not
	 provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void) {
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	 to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	 task.  It is essential that code added to this hook function never attempts
	 to block in any way (for example, call xQueueReceive() with a block time
	 specified, or call vTaskDelay()).  If the application makes use of the
	 vTaskDelete() API function (as this demo application does) then it is also
	 important that vApplicationIdleHook() is permitted to return to its calling
	 function, because it is the responsibility of the idle task to clean up
	 memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, char *pcTaskName) {
	(void) pcTaskName;
	(void) pxTask;

	/* Run time stack overflow checking is performed if
	 configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	 function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}

void SystemClock_Config() {

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	__HAL_RCC_PWR_CLK_ENABLE()
	;

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 80;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV8;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/** @} */
