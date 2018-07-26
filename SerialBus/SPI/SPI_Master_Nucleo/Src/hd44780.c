/**
 * @file hd44780.c
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
 */

#include "hd44780.h"
#include <assert.h>
#include <stdlib.h>

// Le macro seguenti definiscono i comandi che e' possibile inviare al display
#define HD44780_clear			0x01
#define HD44780_home			0x02
#define HD44780_row1			0x80
#define HD44780_row2			0xC0
#define HD44780_cursor_r		0x14
#define HD44780_cursor_l		0x10
#define HD44780_display_off		0x08
#define HD44780_cursor_off		0x0C
#define HD44780_cursor_on		0x0E
#define HD44780_cursor_blink	0x0F
#define HD44780_clear			0x01
#define HD44780_dec_no_shift	0x04
#define HD44780_dec_shift		0x05
#define HD44780_inc_no_shift	0x06
#define HD44780_inc_shift		0x07


/*================================================================================================
 * Dichiarazione funzioni private del modulo
 *==============================================================================================*/

void HD44780_SetByte(HD44780_LCD_t* lcd, uint8_t byte);

void HD44780_WriteCommand(HD44780_LCD_t* lcd, uint8_t command);

void HD44780_WriteData(HD44780_LCD_t* lcd, uint8_t data);

int HD44780_ValidatePair(HD44780_LCD_t* lcd);

void HD44780_ConfigurePin(HD44780_LCD_t* lcd);

/*================================================================================================
 * Dichiarazione macro private del modulo
 *==============================================================================================*/

#define lcd_command(lcd)	HAL_GPIO_WritePin(lcd->RS.Port, lcd->RS.Pin, GPIO_PIN_RESET)

#define lcd_data(lcd)		HAL_GPIO_WritePin(lcd->RS.Port, lcd->RS.Pin, GPIO_PIN_SET)

#define lcd_write(lcd)		HAL_GPIO_WritePin(lcd->RW.Port, lcd->RW.Pin, GPIO_PIN_RESET)

#define lcd_read(lcd)		HAL_GPIO_WritePin(lcd->RW.Port, lcd->RW.Pin, GPIO_PIN_SET)

#define lcd_enable(lcd)		HAL_GPIO_WritePin(lcd->E.Port, lcd->E.Pin, GPIO_PIN_SET); \
							timer_wait_us(100); \
							HAL_GPIO_WritePin(lcd->E.Port, lcd->E.Pin, GPIO_PIN_RESET)


/*================================================================================================
 * Implementazione funzioni di inizializzazione
 *==============================================================================================*/

void HD44780_Init4(	HD44780_LCD_t* lcd,
					PortPinPair_t RS,
					PortPinPair_t RW,
					PortPinPair_t E,
					PortPinPair_t Data7,
					PortPinPair_t Data6,
					PortPinPair_t Data5,
					PortPinPair_t Data4)
{
	assert(lcd);
	lcd->RS.Port = RS.Port;
	lcd->RS.Pin = RS.Pin;
	lcd->RW.Port = RW.Port;
	lcd->RW.Pin = RW.Pin;
	lcd->E.Port = E.Port;
	lcd->E.Pin = E.Pin;
	lcd->Data7.Port = Data7.Port;
	lcd->Data7.Pin = Data7.Pin;
	lcd->Data6.Port = Data6.Port;
	lcd->Data6.Pin = Data6.Pin;
	lcd->Data5.Port = Data5.Port;
	lcd->Data5.Pin = Data5.Pin;
	lcd->Data4.Port = Data4.Port;
	lcd->Data4.Pin = Data4.Pin;
	lcd->Data3.Port = NULL;
	lcd->Data2.Port = NULL;
	lcd->Data1.Port = NULL;
	lcd->Data0.Port = NULL;
	lcd->InterfaceMode = HD44780_INTERFACE_4bit;
	assert(HD44780_ValidatePair(lcd));
	HD44780_ConfigurePin(lcd);
	// sequenza di inizializzazione del device
	timer_wait_ms(50);
	HAL_GPIO_WritePin(lcd->RS.Port, lcd->RS.Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(lcd->RW.Port, lcd->RW.Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(lcd->Data7.Port, lcd->Data7.Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(lcd->Data6.Port, lcd->Data6.Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(lcd->Data5.Port, lcd->Data5.Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(lcd->Data4.Port, lcd->Data4.Pin, GPIO_PIN_SET);
	lcd_enable(lcd);
	timer_wait_ms(10);
	lcd_enable(lcd);
	timer_wait_ms(10);
	lcd_enable(lcd);
	timer_wait_ms(10);
	HAL_GPIO_WritePin(lcd->Data7.Port, lcd->Data7.Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(lcd->Data6.Port, lcd->Data6.Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(lcd->Data5.Port, lcd->Data5.Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(lcd->Data4.Port, lcd->Data4.Pin, GPIO_PIN_RESET);
	lcd_enable(lcd);
	timer_wait_ms(5);
	HD44780_SetByte(lcd, 0x28);
	timer_wait_ms(10);
	HD44780_SetByte(lcd, 0x08);
	timer_wait_ms(10);
	HD44780_SetByte(lcd, 0x01);
	timer_wait_ms(10);
	HD44780_SetByte(lcd, 0x06);
	timer_wait_ms(10);
	HD44780_SetByte(lcd, 0x0F);
	timer_wait_ms(10);
}

void HD44780_Init4_v2(	HD44780_LCD_t* lcd,
						GPIO_TypeDef* RS_Port,		uint16_t RS_Pin,
						GPIO_TypeDef* RW_Port,		uint16_t RW_Pin,
						GPIO_TypeDef* E_Port,		uint16_t E_Pin,
						GPIO_TypeDef* Data7_Port,	uint16_t Data7_Pin,
						GPIO_TypeDef* Data6_Port,	uint16_t Data6_Pin,
						GPIO_TypeDef* Data5_Port,	uint16_t Data5_Pin,
						GPIO_TypeDef* Data4_Port,	uint16_t Data4_Pin)
{
	PortPinPair_t 	RS = {RS_Port, RS_Pin}, RW = {RW_Port, RW_Pin},
					E = {E_Port, E_Pin}, Data7 = {Data7_Port, Data7_Pin},
					Data6 = {Data6_Port, Data6_Pin}, Data5 = {Data5_Port, Data5_Pin},
					Data4 = {Data4_Port, Data4_Pin};
	HD44780_Init4(lcd, RS, RW, E, Data7, Data6, Data5, Data4);

}

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
					PortPinPair_t Data0)
{
	assert(lcd);
	lcd->RS.Port = RS.Port;
	lcd->RS.Pin = RS.Pin;
	lcd->RW.Port = RW.Port;
	lcd->RW.Pin = RW.Pin;
	lcd->E.Port = E.Port;
	lcd->E.Pin = E.Pin;
	lcd->Data7.Port = Data7.Port;
	lcd->Data7.Pin = Data7.Pin;
	lcd->Data6.Port = Data6.Port;
	lcd->Data6.Pin = Data6.Pin;
	lcd->Data5.Port = Data5.Port;
	lcd->Data5.Pin = Data5.Pin;
	lcd->Data4.Port = Data4.Port;
	lcd->Data4.Pin = Data4.Pin;
	lcd->Data3.Port = Data3.Port;
	lcd->Data3.Pin = Data3.Pin;
	lcd->Data2.Port = Data2.Port;
	lcd->Data2.Pin = Data2.Pin;
	lcd->Data1.Port = Data1.Port;
	lcd->Data1.Pin = Data1.Pin;
	lcd->Data0.Port = Data0.Port;
	lcd->Data0.Pin = Data0.Pin;
	lcd->InterfaceMode = HD44780_INTERFACE_8bit;
	assert(HD44780_ValidatePair(lcd));
	HD44780_ConfigurePin(lcd);

	// sequenza di inizializzazione del device
	timer_wait_ms(50);
	HAL_GPIO_WritePin(lcd->RS.Port, lcd->RS.Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(lcd->RW.Port, lcd->RW.Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(lcd->Data7.Port, lcd->Data7.Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(lcd->Data6.Port, lcd->Data6.Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(lcd->Data5.Port, lcd->Data5.Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(lcd->Data4.Port, lcd->Data4.Pin, GPIO_PIN_SET);
	lcd_enable(lcd);
	timer_wait_ms(10);
	lcd_enable(lcd);
	timer_wait_ms(10);
	lcd_enable(lcd);
	timer_wait_ms(10);
	HD44780_SetByte(lcd, 0x38);
	timer_wait_ms(10);
	HD44780_SetByte(lcd, 0x08);
	timer_wait_ms(10);
	HD44780_SetByte(lcd, 0x01);
	timer_wait_ms(10);
	HD44780_SetByte(lcd, 0x06);
	timer_wait_ms(10);
	HD44780_SetByte(lcd, 0x0F);
	timer_wait_ms(10);
}

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
						GPIO_TypeDef* Data0_Port,	uint16_t Data0_Pin)
{
	PortPinPair_t 	RS = {RS_Port, RS_Pin}, RW = {RW_Port, RW_Pin},
					E = {E_Port, E_Pin}, Data7 = {Data7_Port, Data7_Pin},
					Data6 = {Data6_Port, Data6_Pin}, Data5 = {Data5_Port, Data5_Pin},
					Data4 = {Data4_Port, Data4_Pin}, Data3 = {Data3_Port, Data3_Pin},
					Data2 = {Data2_Port, Data2_Pin}, Data1 = {Data1_Port, Data1_Pin},
					Data0 = {Data0_Port, Data0_Pin};
	HD44780_Init8(lcd, RS, RW, E, Data7, Data6, Data5, Data4, Data3, Data2, Data1, Data0);
}

/*================================================================================================
 * Implementazione  funzioni stampa
 *==============================================================================================*/

void HD44780_Printc(HD44780_LCD_t* lcd, char c) {
	HD44780_WriteData(lcd, (uint8_t)c);
}

void HD44780_Print(HD44780_LCD_t* lcd, const char *s) {
	assert(s);
	int i = 0;
	while ((uint8_t)s[i] != 0)
		HD44780_WriteData(lcd, (uint8_t)s[i++]);
}

void HD44780_printBinary8(HD44780_LCD_t *lcd, uint8_t b) {
	int i;
	for (i = 7; i != -1; i--)
		HD44780_Printc(lcd, ((b>>i)&1)+'0');
}

void HD44780_printBinary32(HD44780_LCD_t *lcd, uint32_t w) {
	int i;
	for (i = 31; i != -1; i--)
		HD44780_Printc(lcd, ((w>>i)&1)+'0');
}

void HD44780_printBinary64(HD44780_LCD_t *lcd, uint64_t b) {
	int i;
	for (i = 63; i != -1; i--)
		HD44780_Printc(lcd, ((b>>i)&1)+'0');
}

void HD44780_printHex8(HD44780_LCD_t *lcd, uint8_t b) {
	int i;
	for (i = 7; i != -1; i-=4)
		HD44780_Printc(lcd, ((b>>i)&0xF) + (((b>>i)&0xF) < 10 ? '0' : 'A'));
}

void HD44780_printHex32(HD44780_LCD_t *lcd, uint32_t w) {
	int i;
	for (i = 31; i != -1; i-=4)
		HD44780_Printc(lcd, ((w>>i)&0xF) + (((w>>i)&0xF) < 10 ? '0' : 'A'));
}

void HD44780_printHex64(HD44780_LCD_t *lcd, uint64_t b) {
	int i;
	for (i = 63; i != -1; i-=4)
		HD44780_Printc(lcd, ((b>>i)&0xF) + (((b>>i)&0xF) < 10 ? '0' : 'A'));
}


/*================================================================================================
 * Implementazione  funzioni comando
 *==============================================================================================*/

void HD44780_Clear(HD44780_LCD_t* lcd) {
	HD44780_WriteCommand(lcd, HD44780_clear);
}

void HD44780_Home(HD44780_LCD_t* lcd) {
	HD44780_WriteCommand(lcd, HD44780_home);
}

void HD44780_MoveToRow1(HD44780_LCD_t* lcd) {
	HD44780_WriteCommand(lcd, HD44780_row1);
}

void HD44780_MoveToRow2(HD44780_LCD_t* lcd) {
	HD44780_WriteCommand(lcd, HD44780_row2);
}

void HD44780_MoveCursor(HD44780_LCD_t* lcd, HD44780_Direction_t dir){
	HD44780_WriteCommand(lcd, (dir == HD44780_CursorLeft ? HD44780_cursor_l : HD44780_cursor_r));
}

void HD44780_DisplayOff(HD44780_LCD_t* lcd) {
	HD44780_WriteCommand(lcd, HD44780_display_off);
}

void HD44780_CursorOff(HD44780_LCD_t* lcd) {
	HD44780_WriteCommand(lcd, HD44780_cursor_off);
}

void HD44780_CursorOn(HD44780_LCD_t* lcd) {
	HD44780_WriteCommand(lcd, HD44780_cursor_on);
}

void HD44780_CursorBlink(HD44780_LCD_t* lcd) {
	HD44780_WriteCommand(lcd, HD44780_cursor_blink);
}

/*================================================================================================
 * Implementazione  funzioni private del modulo
 *==============================================================================================*/

void HD44780_SetByte(HD44780_LCD_t* lcd, uint8_t byte)
{
	assert(lcd);
	if (lcd->InterfaceMode == HD44780_INTERFACE_8bit) {
		HAL_GPIO_WritePin(lcd->Data7.Port, lcd->Data7.Pin, ((byte & 0x80) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data6.Port, lcd->Data6.Pin, ((byte & 0x40) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data5.Port, lcd->Data5.Pin, ((byte & 0x20) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data4.Port, lcd->Data4.Pin, ((byte & 0x10) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data3.Port, lcd->Data3.Pin, ((byte & 0x08) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data2.Port, lcd->Data2.Pin, ((byte & 0x04) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data1.Port, lcd->Data1.Pin, ((byte & 0x02) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data0.Port, lcd->Data0.Pin, ((byte & 0x01) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		lcd_enable(lcd);
	}
	else {
		HAL_GPIO_WritePin(lcd->Data7.Port, lcd->Data7.Pin, ((byte & 0x80) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data6.Port, lcd->Data6.Pin, ((byte & 0x40) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data5.Port, lcd->Data5.Pin, ((byte & 0x20) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data4.Port, lcd->Data4.Pin, ((byte & 0x10) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		lcd_enable(lcd);
		HAL_GPIO_WritePin(lcd->Data7.Port, lcd->Data7.Pin, ((byte & 0x08) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data6.Port, lcd->Data6.Pin, ((byte & 0x04) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data5.Port, lcd->Data5.Pin, ((byte & 0x02) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		HAL_GPIO_WritePin(lcd->Data4.Port, lcd->Data4.Pin, ((byte & 0x01) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET));
		lcd_enable(lcd);
	}
}

void HD44780_WriteCommand(HD44780_LCD_t* lcd, uint8_t command) {
	assert(lcd);
	lcd_write(lcd);
	lcd_command(lcd);
	HD44780_SetByte(lcd, command);
	timer_wait_ms(2);
}

void HD44780_WriteData(HD44780_LCD_t* lcd, uint8_t data)
{
	assert(lcd);
	lcd_write(lcd);
	lcd_data(lcd);
	HD44780_SetByte(lcd, data);
	timer_wait_ms(2);
}

int HD44780_ValidatePair(HD44780_LCD_t* lcd)
{
	assert(lcd);
	int array_dim = (lcd->InterfaceMode == HD44780_INTERFACE_8bit ? 11 : 7);
	int i, j;
	const PortPinPair_t pair[] = {	lcd->RS, lcd->RW, lcd->E,
									lcd->Data7, lcd->Data6, lcd->Data5, lcd->Data4,
									lcd->Data3, lcd->Data2, lcd->Data1, lcd->Data0};
	for (i = 0; i < array_dim; i++)
		for (j = i+1; j < array_dim; j++)
			if (	pair[i].Port != 0 &&
					pair[j].Port != 0 &&
					pair[i].Port == pair[j].Port &&
					pair[i].Pin == pair[j].Pin)
				return 0;
	return 1;
}

void HD44780_ConfigurePin(HD44780_LCD_t* lcd)
{
	assert(lcd);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	int array_dim = (lcd->InterfaceMode == HD44780_INTERFACE_8bit ? 11 : 7);
	int i;
	const PortPinPair_t pair[] = {	lcd->RS, lcd->RW, lcd->E,
									lcd->Data7, lcd->Data6, lcd->Data5, lcd->Data4,
									lcd->Data3, lcd->Data2, lcd->Data1, lcd->Data0};
	for (i = 0; i < array_dim; i++)
	{
		GPIO_InitStruct.Pin = pair[i].Pin;
		HAL_GPIO_Init(pair[i].Port, &GPIO_InitStruct);
		HAL_GPIO_WritePin(pair[i].Port, pair[i].Pin, GPIO_PIN_RESET);
	}
}
