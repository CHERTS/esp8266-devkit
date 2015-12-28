/*
* Oryginal by:
* Author : Aaron Lee
* Version: 1.00
* Date   : 2014.3.24
* Email  : hello14blog@gmail.com
* Modification: none
* Mod by reaper7
* found at http://bbs.espressif.com/viewtopic.php?f=15&t=31
*/

#ifndef __I2C_OLED_H
#define	__I2C_OLED_H

#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"

#define OLED_ADDRESS	0x78  // D/C->GND
//#define OLED_ADDRESS	0x7a // D/C->Vcc

void OLED_writeCmd(unsigned char I2C_Command);
void OLED_writeDat(unsigned char I2C_Data);
bool OLED_Init(void);
void OLED_SetPos(unsigned char x, unsigned char y);
void OLED_Fill(unsigned char fill_Data);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
void OLED_Line(unsigned char x, unsigned char ch[], unsigned char TextSize);
void OLED_Print(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);

#endif
