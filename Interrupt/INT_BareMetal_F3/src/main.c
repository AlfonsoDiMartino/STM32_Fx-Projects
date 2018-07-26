/**
 * @file main.c
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
 * @brief Esempio di interruzione Bare Metal, utilizzando la libreria myBSP.
 */

/**
 * @addtogroup InterruzioniSTM32Fx
 * @{
 * @addtogroup BareMetal
 * @{
 *
 * @brief Esempio di interruzione Bare Metal, utilizzando la libreria myBSP.
 *
 * @details Implementazione di un semplice esempio che mette in evidenzia la gestione delle interruzioni in modalità bare metal. <br>
 * 			Nel caso specifico, l'interruzione è scatenata alla pressione del Push Button. Nel corpo della EXTI0_IRQHandler(), viene richiamata la
 * 			HAL_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin) e poi eseguite le operazioni che rispecchiano il comportamento voluto (nel caso specifico abbiamo
 * 			il toggle sui led). <br>
 * 			Nell'esempio vengono utilizzate le funzioni fornite dal custom myBSP.
 */

#include "myBSP.h"
			
/**
 * @brief Funzione di inizializzazione.
 *
 * @details E' la prima funziona ad essere eseguita nel main program.<br>
 * 			Viene inizializzata la libreria HAL e configurate le periferiche da utilizzare.
 */
void init();

/**
 * @brief Funzione che implementa la logica del programma.
 *
 * @details Il dispositivo resta in attesa della pressione del Push Button.
 */
void loop();


int main(void)
{
	init();
	for(;;) loop();
}

void init(){
	HAL_Init();
	for(int i=0;i<N_LED;i++)
		myBSP_LED_Init(i);
	myBSP_BUTTON_Init(myBUTTON,myBUTTON_MODE_EXTI);
}

void loop(){
	// In attesa dell'evento di pressione di User Button
}

/**
 * @}
 */

/**
 * @}
 */
