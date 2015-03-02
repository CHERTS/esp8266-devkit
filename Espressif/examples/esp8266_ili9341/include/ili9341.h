/*
 * ili9341.h
 *
 *  Created on: 12 џэт. 2015 у.
 *      Author: Sem
 */

#ifndef INCLUDE_ILI9341_H_
#define INCLUDE_ILI9341_H_


#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <mem.h>
#include "hspi.h"
#include "font.h"

typedef union
{
   	struct
	{
   		uint8_t b0 :8;
   		uint8_t b1 :8;
   		uint8_t b2 :8;
   		uint8_t b3 :8;
   	} bytes;

   	uint32_t all;
} uint32_t_bytes;

typedef struct
{
	uint16_t xLeft;
	uint16_t xRight;
	uint16_t yUp;
	uint16_t yDown;
	uint16_t *color;
	uint8_t isOneColor;
} Rectangle;

#define TFT_CS_ACTIVE	GPIO_OUTPUT_SET(4, 0)
#define TFT_CS_DEACTIVE GPIO_OUTPUT_SET(4, 1)
#define TFT_CS_INIT		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4); TFT_CS_DEACTIVE

#define TFT_DC_DATA		GPIO_OUTPUT_SET(2, 1)
#define TFT_DC_COMMAND	GPIO_OUTPUT_SET(2, 0)
#define TFT_DC_INIT 	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2); TFT_DC_DATA

#define TFT_RST_ACTIVE		GPIO_OUTPUT_SET(0, 0)
#define TFT_RST_DEACTIVE 	GPIO_OUTPUT_SET(0, 1)
#define TFT_RST_INIT		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0); TFT_RST_DEACTIVE

#define MIN_TFT_Y				0
#define MAX_TFT_Y				319
#define MIN_TFT_X				0
#define MAX_TFT_X				239

extern uint32_t tft_readId(void);
extern void tft_init(void);
extern void tft_configRegister(void);
extern void tft_fillRectangle(uint16_t xLeft, uint16_t xRight, uint16_t yUp, uint16_t yDown, uint16_t *color, uint32_t amountColor);
extern void tft_setPixel(uint16_t poX, uint16_t poY, uint16_t color);
extern void tft_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
extern void tft_drawChar(int16_t ascii, uint16_t posX, uint16_t posY, uint8_t sizeFont, uint16_t colorFont, uint16 colorBackGround);
extern void tft_drawString(char *str, uint16_t posX, uint16_t posY, uint8_t sizeFont, uint16_t colorFont, uint16 colorBackGround);

#endif /* INCLUDE_ILI9341_H_ */
