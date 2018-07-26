/**
 * @file hd44780.h
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
 * Questo modulo permette di utilizzare il display Hitachy HD44780 pilotandolo con un device
 * STM32. Il modulo utilizza l' HAL della libreria STMCube per la gestione del processore STM32.
 */

/* Changelog
 * 11 agosto 2016:
 * 	Terminata l'implementazione di tutte le funzioni eccetto quelle di validazione delle coppie
 * 	porta-pin.
 * 	Completati i test preliminari su funzioni di inizializzazione e stampa.
 *
 * 12 agosto 2016
 * 	Terminala l'implementazione della funzione di validazione delle coppie.
 * 	Eseguiti i test su funzioni per interfacciamento ad 8 e a 4 bit.
 * 	Terminata scrittura documentazione
 *
 * 18 agosto 2016
 *   Modifiche alle funzioni di inizializzazione. Adesso la struttura HD44780_LCD_t viene
 *   inizializzata solo se l'inizializzazione puo' concludersi esclusivamente con successo.
 *
 * 17-19 maggio 2017
 *  - Eliminazione del tipo HD44780_error_t
 *  - uso di assert per la verifica dei parametri passati alle funzioni
 *  - completamento della documentazione doxygen
 *  - test di modulo
 *
 */

/**
 * @addtogroup LCD
 * @{
 *
 * @defgroup HD44780
 * @{
 *
 * Un oggetto di tipo HD44780_LCD_t rappresenta un device lcd HD44780. Il modulo e' pensato per
 * permettere la gestione di piu' display da parte dello stesso processore, agendo su oggetti
 * HD44780_LCD_t diversi.<br>
 * La struttura HD44780_LCD_t specifica quali siano i pin del microcontrollore che pilotano un
 * determinato segnale del device. Ciascuno dei pin, cosi' come previsto dalla libreria STMCube,
 * e' identificato attraverso una coppia porta-pin (ad esempio la coppia GPIOD-GPIO_PIN_9 si
 * riferisce al pin 9 della porta D, quindi PD9). L'assegnazione segnale-coppia, quindi l'
 * inizializzazione della struttura HD44780_LCD_t relativa ad un device lcd, DEVE essere effettuata
 * tassativamente utilizzando le funzioni<br>
 * 	- HD44780_Init4()
 * 	- HD44780_Init4_v2()
 * 	- HD44780_Init8()
 * 	- HD44780_Init8_v2()<br>
 *
 * le quali provvedono anche ad effettuare un test di connessione volto ad individuare eventuali
 * segnali erroneamente associati.<br>
 * Tali funzioni restituiscono un codice di errore, il quale puo' essere utilizzato per identificare
 * la problematica sorta durante l'inizializzazione e provvedere alla sua gestione. Per i dettagli
 * si rimanda alla documentazione delle specifiche funzioni.<br>
 * <br>
 * Oltre alle funzioni di inizializzazione, il modulo fornisce anche funzioni basilari per la
 * stampa su display lcd di
 * - caratteri, con la funzione HD44780_Printc()
 * - stringhe null-terminated di caratteri, con la funzione HD44780_Print()<br>
 *
 * Sono disponibili, inoltre, anche funzioni specifiche per inviare comandi al device:
 *  - HD44780_Clear()
 *  - HD44780_Home()
 *  - HD44780_MoveToRow1()
 *  - HD44780_MoveToRow2()
 *  - HD44780_MoveCursor()
 *  - HD44780_DisplayOff()
 *  - HD44780_CursorOff()
 *  - HD44780_CursorOn()
 *  - HD44780_CursorBlink()<br>
 *
 * Per ulteriori dettagli si rimanda alla documentazione delle specifiche funzioni ed alla
 * documentazione esterna che accompagna il modulo, reperibile nella cartella Doc.
 */

#ifndef __HD44780_LCD__
#define __HD44780_LCD__

#include <inttypes.h>

#include "common.h"

/*================================================================================================
 * Strutture e tipi dato utilizzate dal modulo
 *==============================================================================================*/


typedef enum {
	HD44780_INTERFACE_4bit,
	HD44780_INTERFACE_8bit
} HD44780_InterfaceMode_t;

/**
 * @brief L'oggetto di tipo HD44780_LCD_t rappresenta un device HD44780.
 *
 * La struttura raggruppa tutte le coppie porta-pin utilizzate per l'interfacciamento con il
 * device.
 * La struttura va inizializzata esclusivamente attraverso l'uso delle funzioni HD44780_Init4() o
 * HD44780_Init8(). E' fortemente sconsigliato inizializzare le coppie campo per campo manualmente
 * in quanto, oltre la mera inizializzazione, le suddette funzioni effettuano anche la validazione
 * delle connessioni, segnalando eventuali errori di configurazione.
 *
 * @see HD44780_Init4
 * @see HD44780_Init8
  *
 * @warning Non modificare i campi della struttura dopo che essa sia stata inizializzata.
 */
typedef struct
{
	PortPinPair_t 	RS;
	PortPinPair_t 	RW;
	PortPinPair_t 	E;
	PortPinPair_t 	Data7;
	PortPinPair_t 	Data6;
	PortPinPair_t 	Data5;
	PortPinPair_t 	Data4;
	PortPinPair_t 	Data3;
	PortPinPair_t 	Data2;
	PortPinPair_t 	Data1;
	PortPinPair_t 	Data0;
	HD44780_InterfaceMode_t InterfaceMode;
} HD44780_LCD_t;

/*================================================================================================
 * Funzioni inizializzazione
 *==============================================================================================*/

/**
 * @brief Inizializza un oggetto display lcd HD44780 affinche' si utilizzi l'interfaccia a 4 bit.
 *
 * Inizializza un oggetto HD44780_LCD_t verificando la validita' delle coppie porta-pin per l'
 * interfacciamento, configurando i pin GPIO e inizializzando il device.
 *
 * @warning Se i pin associati ai segnali di pilotaggio del device non sono correttamente
 * configurati come pin di output, il dispositivo non funzionera' correttamente.
 *
 * @warning Non modificare i campi della struttura HD44780_LCD_t dopo che essa sia stata
 * inizializzata.
 *
 * @param lcd 	struttura di tipo HD44780_LCD_t che descrive un display HD44780 da inizializzare
 * @param RS 	struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale RS (data/command)
 * @param RW	struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale RW (read/write)
 * @param E 	struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale E (Enable)
 * @param Data7 struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale Data7
 * @param Data6 struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale Data6
 * @param Data5 struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale Data5
 * @param Data4 struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale Data4
 *
 * @warning Usa la macro assert() per verificare che:
 *          - lcd non sia un puntatore nullo
 *          - i pin a cui sono associati i segnali di controllo/dato siano tutti diversi
 *
 * @code
 * HD44780_LCD_t lcd;
 * PortPinPair_t 	RS = {GPIOE, GPIO_PIN_13}, RW = {GPIOE, GPIO_PIN_14},
 *					E = {GPIOE, GPIO_PIN_15}, Data7 = {GPIOD, GPIO_PIN_7},
 *					Data6 = {GPIOD, GPIO_PIN_6}, Data5 = {GPIOD, GPIO_PIN_5},
 *					Data4 = {GPIOD, GPIO_PIN_4};
 * @endcode
 *
 * @warning E' necessario abilitare il clock dei GPIO!
 * __HAL_RCC_GPIOE_CLK_ENABLE();
 * __HAL_RCC_GPIOC_CLK_ENABLE();
 * __HAL_RCC_GPIOF_CLK_ENABLE();
 * __HAL_RCC_GPIOA_CLK_ENABLE();
 * __HAL_RCC_GPIOD_CLK_ENABLE();
 * __HAL_RCC_GPIOB_CLK_ENABLE();
 *
 */
void HD44780_Init4(	HD44780_LCD_t* lcd,
					PortPinPair_t RS, PortPinPair_t RW,	PortPinPair_t E,
					PortPinPair_t Data7, PortPinPair_t Data6, PortPinPair_t Data5,PortPinPair_t Data4);
/**
 * @brief Inizializza un display lcd HD44780 con interfacciamento a 4 bit.
 *
 * Inizializza un oggetto HD44780_LCD_t verificando la validita' delle coppie porta-pin per l'
 * interfacciamento, configurando i pin GPIO e inizializzando il device.
 *
 * @warning Se i pin associati ai segnali di pilotaggio del device non sono correttamente
 * configurati come pin di output, il dispositivo non funzionera' correttamente.
 *
 * @warning Non modificare i campi della struttura dopo che essa sia stata inizializzata.
 *
 * @param lcd			struttura di tipo HD44780_LCD_t che descrive un display HD44780 da
 * 						inizializzare
 * @param RS_Port		porta GPIO associata al segnale RS
 * @param RS_Pin		pin di RS_Port associato al segnale RS
 * @param RW_Port		porta GPIO associata al segnale RW
 * @param RW_Pin		pin di RW_Port associato al segnale RW
 * @param E_Port		porta GPIO associata al segnale RE
 * @param E_Pin			pin di E_Port associato al segnale E
 * @param Data7_Port	porta GPIO associata al segnale Data7
 * @param Data7_Pin		pin di Data7_Port associato al segnale Data7
 * @param Data6_Port	porta GPIO associata al segnale Data6
 * @param Data6_Pin		pin di Data6_Port associato al segnale Data6
 * @param Data5_Port	porta GPIO associata al segnale Data5
 * @param Data5_Pin		pin di Data5_Port associato al segnale Data5
 * @param Data4_Port	porta GPIO associata al segnale Data4
 * @param Data4_Pin		pin di Data4_Port associato al segnale Data4
 *
 *
 * @warning Usa la macro assert() per verificare che:
 *          - lcd non sia un puntatore nullo
 *          - i pin a cui sono associati i segnali di controllo/dato siano tutti diversi
 * @code
 * HD44780_LCD_t lcd;
 * HD44780_Init4_v2(	&lcd,
 *						GPIOC, GPIO_PIN_12,
 *						GPIOC, GPIO_PIN_10,
 *						GPIOC, GPIO_PIN_11,
 *						GPIOD, GPIO_PIN_0,
 *						GPIOD, GPIO_PIN_1,
 *						GPIOD, GPIO_PIN_2,
 *						GPIOD, GPIO_PIN_3);
 * @endcode
 *
 * @warning E' necessario abilitare il clock dei GPIO!
 * __HAL_RCC_GPIOE_CLK_ENABLE();
 * __HAL_RCC_GPIOC_CLK_ENABLE();
 * __HAL_RCC_GPIOF_CLK_ENABLE();
 * __HAL_RCC_GPIOA_CLK_ENABLE();
 * __HAL_RCC_GPIOD_CLK_ENABLE();
 * __HAL_RCC_GPIOB_CLK_ENABLE();
 */
void HD44780_Init4_v2(	HD44780_LCD_t* lcd,
						GPIO_TypeDef* RS_Port,		uint16_t RS_Pin,
						GPIO_TypeDef* RW_Port,		uint16_t RW_Pin,
						GPIO_TypeDef* E_Port,		uint16_t E_Pin,
						GPIO_TypeDef* Data7_Port,	uint16_t Data7_Pin,
						GPIO_TypeDef* Data6_Port,	uint16_t Data6_Pin,
						GPIO_TypeDef* Data5_Port,	uint16_t Data5_Pin,
						GPIO_TypeDef* Data4_Port,	uint16_t Data4_Pin);

/**
 * @brief Inizializza un display lcd HD44780 con interfacciamento ad 8 bit.
 *
 * @warning Se i pin associati ai segnali di pilotaggio del device non sono correttamente
 * configurati come pin di output, il dispositivo non funzionera' correttamente.
 *
 * @warning Non modificare i campi della struttura dopo che essa sia stata inizializzata.
 *
 * @param lcd 	struttura di tipo HD44780_LCD_t che descrive un display HD44780 da inizializzare
 * @param RS 	struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale RS (data/command)
 * @param RW	struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale RW (read/write)
 * @param E 	struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale E (Enable)
 * @param Data7 struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale Data7
 * @param Data6 struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale Data6
 * @param Data5 struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale Data5
 * @param Data4 struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale Data4
 * @param Data3 struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale Data3
 * @param Data2 struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale Data2
 * @param Data1 struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale Data1
 * @param Data0 struttura di tipo PortPinPair_t che descrive la coppia porta-pin per il
 * 				segnale Data0
 *
 *
 * @warning Usa la macro assert() per verificare che:
 *          - lcd non sia un puntatore nullo
 *          - i pin a cui sono associati i segnali di controllo/dato siano tutti diversi
 *
 * @code
 * HD44780_LCD_t lcd;
 * PortPinPair_t 	RS = {GPIOE, GPIO_PIN_13}, RW = {GPIOE, GPIO_PIN_14},
 *					E = {GPIOE, GPIO_PIN_15}, Data7 = {GPIOD, GPIO_PIN_7},
 *					Data6 = {GPIOD, GPIO_PIN_6}, Data5 = {GPIOD, GPIO_PIN_5},
 *					Data4 = {GPIOD, GPIO_PIN_4}, Data3 = {GPIOD, GPIO_PIN_3},
 *					Data2 = {GPIOD, GPIO_PIN_2}, Data1 = {GPIOD, GPIO_PIN_1},
 *					Data0 = {GPIOD, GPIO_PIN_0};
 *	HD44780_Init8(&lcd, RS, RW, E, Data7, Data6, Data5, Data4, Data3, Data2, Data1, Data0);
 *	@endcode
 *
 * @warning E' necessario abilitare il clock dei GPIO!
 * __HAL_RCC_GPIOE_CLK_ENABLE();
 * __HAL_RCC_GPIOC_CLK_ENABLE();
 * __HAL_RCC_GPIOF_CLK_ENABLE();
 * __HAL_RCC_GPIOA_CLK_ENABLE();
 * __HAL_RCC_GPIOD_CLK_ENABLE();
 * __HAL_RCC_GPIOB_CLK_ENABLE();
 */
void HD44780_Init8(	HD44780_LCD_t* lcd,
					PortPinPair_t RS,
					PortPinPair_t RW,
					PortPinPair_t E,
					PortPinPair_t Data7,
					PortPinPair_t Data6,
					PortPinPair_t Data5,
					PortPinPair_t Data4,
					PortPinPair_t Data3,
					PortPinPair_t Data2,
					PortPinPair_t Data1,
					PortPinPair_t Data0);

/**
 * @brief Inizializza un display lcd HD44780 con interfacciamento ad 8 bit.
 *
 * Inizializza un oggetto HD44780_LCD_t verificando la validita' delle coppie porta-pin per l'
 * interfacciamento, configurando i pin GPIO e inizializzando il device.
 *
 * @warning Se i pin associati ai segnali di pilotaggio del device non sono correttamente
 * configurati come pin di output, il dispositivo non funzionera' correttamente.
 *
 * @warning Non modificare i campi della struttura dopo che essa sia stata inizializzata.
 *
 * @param lcd			struttura di tipo HD44780_LCD_t che descrive un display HD44780 da
 * 						inizializzare
 * @param RS_Port		porta GPIO associata al segnale RS
 * @param RS_Pin		pin di RS_Port associato al segnale RS
 * @param RW_Port		porta GPIO associata al segnale RW
 * @param RW_Pin		pin di RW_Port associato al segnale RW
 * @param E_Port		porta GPIO associata al segnale RE
 * @param E_Pin			pin di E_Port associato al segnale E
 * @param Data7_Port	porta GPIO associata al segnale Data7
 * @param Data7_Pin		pin di Data7_Port associato al segnale Data7
 * @param Data6_Port	porta GPIO associata al segnale Data6
 * @param Data6_Pin		pin di Data6_Port associato al segnale Data6
 * @param Data5_Port	porta GPIO associata al segnale Data5
 * @param Data5_Pin		pin di Data5_Port associato al segnale Data5
 * @param Data4_Port	porta GPIO associata al segnale Data4
 * @param Data4_Pin		pin di Data4_Port associato al segnale Data4
 * @param Data3_Port	porta GPIO associata al segnale Data3
 * @param Data3_Pin		pin di Data3_Port associato al segnale Data3
 * @param Data2_Port	porta GPIO associata al segnale Data2
 * @param Data2_Pin		pin di Data2_Port associato al segnale Data2
 * @param Data1_Port	porta GPIO associata al segnale Data1
 * @param Data1_Pin		pin di Data1_Port associato al segnale Data1
 * @param Data0_Port	porta GPIO associata al segnale Data0
 * @param Data0_Pin		pin di Data0_Port associato al segnale Data0
 *
 *
 * @warning Usa la macro assert() per verificare che:
 *          - lcd non sia un puntatore nullo
 *          - i pin a cui sono associati i segnali di controllo/dato siano tutti diversi
 *
 * @code
 * HD44780_LCD_t lcd;
 * HD44780_Init8_v2(	&lcd,
 *						GPIOC, GPIO_PIN_12,
 *						GPIOC, GPIO_PIN_10,
 *						GPIOC, GPIO_PIN_11,
 *						GPIOD, GPIO_PIN_0,
 *						GPIOD, GPIO_PIN_1,
 *						GPIOD, GPIO_PIN_2,
 *						GPIOD, GPIO_PIN_3,
 *						GPIOD, GPIO_PIN_4,
 *						GPIOD, GPIO_PIN_5,
 *						GPIOD, GPIO_PIN_6,
 *						GPIOD, GPIO_PIN_7);
 * @endcode
 *
 * @warning E' necessario abilitare il clock dei GPIO!
 * __HAL_RCC_GPIOE_CLK_ENABLE();
 * __HAL_RCC_GPIOC_CLK_ENABLE();
 * __HAL_RCC_GPIOF_CLK_ENABLE();
 * __HAL_RCC_GPIOA_CLK_ENABLE();
 * __HAL_RCC_GPIOD_CLK_ENABLE();
 * __HAL_RCC_GPIOB_CLK_ENABLE();
 */
void HD44780_Init8_v2(	HD44780_LCD_t* lcd,
						GPIO_TypeDef* RS_Port,		uint16_t RS_Pin,
						GPIO_TypeDef* RW_Port,		uint16_t RW_Pin,
						GPIO_TypeDef* E_Port,		uint16_t E_Pin,
						GPIO_TypeDef* Data7_Port,	uint16_t Data7_Pin,
						GPIO_TypeDef* Data6_Port,	uint16_t Data6_Pin,
						GPIO_TypeDef* Data5_Port,	uint16_t Data5_Pin,
						GPIO_TypeDef* Data4_Port,	uint16_t Data4_Pin,
						GPIO_TypeDef* Data3_Port,	uint16_t Data3_Pin,
						GPIO_TypeDef* Data2_Port,	uint16_t Data2_Pin,
						GPIO_TypeDef* Data1_Port,	uint16_t Data1_Pin,
						GPIO_TypeDef* Data0_Port,	uint16_t Data0_Pin);

/*================================================================================================
 * Funzioni stampa
 *==============================================================================================*/

/**
 * @brief Stampa un carattere
 * @param lcd display da pilotare;
 * @param c carattere da stampare sul display;
 *
 * @warning Usa la macro assert() per verificare che lcd non sia un puntatore nullo
 */
void HD44780_Printc(HD44780_LCD_t* lcd, char c);

/**
 * @brief Stampa una stringa null-terminated di caratteri
 *
 * La funzione può essere utilizzata per stampare anche numeri interi e floating point. Si veda
 * gli esempi di cui sotto.
 *
 * @param lcd display da pilotare;
 * @param s puntatore alla stringa null-terminated da stampare sul display;
 *
 * @warning Usa la macro assert() per verificare che lcd non sia un puntatore nullo
 *
 * @code
 * // stampa di un intero
 * #include <stdlib.h>
 * 	...
 * char str[10];	// assicurarsi di allocare sufficiente spazio per la stampa del numero
 * sprintf(str,"%d", integer_number);
 * error = HD44780_Print(lcd, str);
 * // stampa di un intero
 * #include <stdlib.h>
 * ...
 * char str[10];
 * snprintf(str, 10,"%d", integer_number);
 * error = HD44780_Print(lcd, str);
 * // stampa di un float
 * #include <stdlib.h>
 * ...
 * char str[20];	// assicurarsi di allocare sufficiente spazio per la stampa del numero
 * sprintf(str,"%f", float_number);
 * error = HD44780_Print(lcd, str);
 * // stampa di un float, nel caso in cui la soluzione precedente dovesse non funzionare
 * #include <stdlib.h>
 * ...
 * char str[20];
 * int parte_intera, parte_decimale, moltiplicatore = 1000;
 * // se si desiderano piu' di tre cifre decimali basta aumentare la potenza del
 * // moltiplicatore
 * // es. cinque cifre decimali ==> moltiplicatore = 100000
 * // si sconsiglia di stampare piu' di quattro cifre decimali per non causare overflow
 * // nelle istruzioni che seguono
 * parte_intera = (int) float_number;
 * parte_decimale = (int)(float_number * moltiplicatore) - (parte_intera * moltiplicatore);
 * snprintf(str, 20,"%d.%d", parte_intera, parte_decimale);
 * error = HD44780_Print(lcd, str);
 * @endcode
 */
void HD44780_Print(HD44780_LCD_t* lcd, const char *s);

/**
 * @brief Stampa un byte in binario. (bit piu' significativo a sinistra)
 * @param lcd
 * @param b byte da stampare
 */
void HD44780_printBinary8(HD44780_LCD_t *lcd, uint8_t b);

/**
 * @brief Stampa una word di 32 bit in binario. (bit piu' significativo a sinistra)
 * @param lcd
 * @param w word da stampare
 */
void HD44780_printBinary32(HD44780_LCD_t *lcd, uint32_t w);

/**
 * @brief Stampa un blocco di 64 bit in binario. (bit piu' significativo a sinistra)
 * @param lcd
 * @param b blocco da stampare
 */
void HD44780_printBinary64(HD44780_LCD_t *lcd, uint64_t b);

/**
 * @brief Stampa un byte in esadecimale. (bit piu' significativo a sinistra)
 * @param lcd
 * @param b byte da stampare
 */
void HD44780_printHex8(HD44780_LCD_t *lcd, uint8_t b);

/**
 * @brief Stampa una word di 32 bit in esadecimale. (bit piu' significativo a sinistra)
 * @param lcd
 * @param w word da stampare
 */
void HD44780_printHex32(HD44780_LCD_t *lcd, uint32_t w);

/**
 * @brief Stampa un blocco di 64 bit in esadecimale. (bit piu' significativo a sinistra)
 * @param lcd
 * @param b blocco da stampare
 */
void HD44780_printHex64(HD44780_LCD_t *lcd, uint64_t b);

/**
 * @brief Pulisce il display e sposta il cursore all'inizio della prima riga
 * @param lcd display da pilotare;
 * @warning Usa la macro assert() per verificare che lcd non sia un puntatore nullo
 */
void HD44780_Clear(HD44780_LCD_t* lcd);

/**
 * @brief Sposta il cursore all'inizio della prima riga
 * @param lcd display da pilotare;
 * @warning Usa la macro assert() per verificare che lcd non sia un puntatore nullo
 */
void HD44780_Home(HD44780_LCD_t* lcd);

/**
 * @brief Sposta il cursore all'inizio della prima riga
 * @param lcd display da pilotare;
  * @warning Usa la macro assert() per verificare che lcd non sia un puntatore nullo
 */
void HD44780_MoveToRow1(HD44780_LCD_t* lcd);

/**
 * @brief Sposta il cursore all'inizio della seconda riga
 * @param lcd display da pilotare;
 * @warning Usa la macro assert() per verificare che lcd non sia un puntatore nullo
 */
void HD44780_MoveToRow2(HD44780_LCD_t* lcd);


/**
 * Direzioni di spostamento del cursore
 */
typedef enum
{
	HD44780_CursorLeft,//!< sposta il cursore a sinistra
	HD44780_CursorRight//!< sposta il cursore a destra
} HD44780_Direction_t;

/**
 * @brief Sposta il cursore di una posizione a destra o sinistra
 * @param lcd display da pilotare;
 * @param dir direzione in cui spostare il cursore, @see direction_t;
 * @warning Usa la macro assert() per verificare che lcd non sia un puntatore nullo
 */
void HD44780_MoveCursor(HD44780_LCD_t* lcd, HD44780_Direction_t dir);

/**
 * @brief Disattiva il display
 * @param lcd display da pilotare;
 * @warning Usa la macro assert() per verificare che lcd non sia un puntatore nullo
 */
void HD44780_DisplayOff(HD44780_LCD_t* lcd);

/**
 * @brief Disattiva la visualizzazione del cursore
 * @param lcd display da pilotare;
 * @warning Usa la macro assert() per verificare che lcd non sia un puntatore nullo
 */
void HD44780_CursorOff(HD44780_LCD_t* lcd);

/**
 * @brief Attiva la visualizzazione del cursore
 * @param lcd display da pilotare;
 * @warning Usa la macro assert() per verificare che lcd non sia un puntatore nullo
 */
void HD44780_CursorOn(HD44780_LCD_t* lcd);

/**
 * @brief Attiva il cursore lampeggiante
 * @param lcd display da pilotare;
 * @warning Usa la macro assert() per verificare che lcd non sia un puntatore nullo
 */
void HD44780_CursorBlink(HD44780_LCD_t* lcd);

#endif

/**
 * @}
 * @}
 */
