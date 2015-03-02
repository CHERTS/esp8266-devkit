/*
 * ili9341.c


 *
 *  Created on: 12 янв. 2015 г.
 *      Author: Sem
 */

#include "ili9341.h"

static void transmitCmdData(uint8_t cmd, uint8_t * data, uint8_t numDataByte)
{
    TFT_CS_ACTIVE;
   	TFT_DC_COMMAND;
   	hspi_TxRx(&cmd, 1);
    if (numDataByte > 0)
    {
    	TFT_DC_DATA;
        hspi_TxRx(data, numDataByte);
    }
    TFT_CS_DEACTIVE;
}

static void transmitData(uint8_t *data, uint8_t numByte, uint32_t numRepeat)
{
	TFT_DC_DATA;
    TFT_CS_ACTIVE;
    hspi_Tx(data, numByte, numRepeat);
    TFT_CS_DEACTIVE;
}

static uint8_t readRegister(uint8_t cmd, uint8_t numParameter)
{
    uint8_t data = 0;
    data = 0x10 + numParameter;
    transmitCmdData(0xD9, &data, 1); // secret command and 0x11 is the first Parameter
    transmitCmdData(cmd, &data, 1);
    return data;
}

static void swap(uint16_t *a, uint16_t *b)
{
    (*a) = (*a) ^ (*b);
    (*b) = (*a) ^ (*b);
    (*a) = (*a) ^ (*b);
}

static void constrain(uint16_t *value, uint16_t min, uint16_t max)
{
	if (*value < min)
		*value = min;
	else if (*value > max)
		*value = max;
}

static void setCol(uint16_t start, uint16_t end)
{
	static uint16_t startLast = 0;
	static uint16_t endLast = 0;
	if ((startLast == start) && (endLast == end))
	{
		return;
	}
	else
	{
		uint8_t data[4] = {start >> 8, start & 0xFF, end >> 8, end & 0xFF};
		transmitCmdData(0x2A, data, 4);	//Column Command address
		startLast = start;
		endLast = end;
	}
}

static void setPage(uint16_t start, uint16_t end)
{
	static uint16_t startLast = 0;
	static uint16_t endLast = 0;
	if ((startLast == start) && (endLast == end))
	{
		return;
	}
	else
	{
		uint8_t data[4] = {start >> 8, start & 0xFF, end >> 8, end & 0xFF};
		transmitCmdData(0x2B, data, 4);	//Column Command address
		startLast = start;
		endLast = end;
	}
}

uint32_t tft_readId(void)
{
    uint8_t i = 0;
	uint32_t_bytes id = {0};

    id.bytes.b2 = readRegister(0xd3, 1);
    id.bytes.b1 = readRegister(0xd3, 2);
    id.bytes.b0 = readRegister(0xd3, 3);

    return  id.all;
}

void tft_configRegister(void)
{
	uint8_t data[15] = {0};

	data[0] = 0x39;
	data[1] = 0x2C;
	data[2] = 0x00;
	data[3] = 0x34;
	data[4] = 0x02;
	transmitCmdData(0xCB, data, 5);

	data[0] = 0x00;
	data[1] = 0XC1;
	data[2] = 0X30;
	transmitCmdData(0xCF, data, 3);

	data[0] = 0x85;
	data[1] = 0x00;
	data[2] = 0x78;
	transmitCmdData(0xE8, data, 3);

	data[0] = 0x00;
	data[1] = 0x00;
	transmitCmdData(0xEA, data, 2);

	data[0] = 0x64;
	data[1] = 0x03;
	data[2] = 0X12;
	data[3] = 0X81;
	transmitCmdData(0xED, data, 4);

	data[0] = 0x20;
	transmitCmdData(0xF7, data, 1);

	data[0] = 0x23;   	//VRH[5:0]
	transmitCmdData(0xC0, data, 1);    	//Power control

	data[0] = 0x10;   	//SAP[2:0];BT[3:0]
	transmitCmdData(0xC1, data, 1);    	//Power control

	data[0] = 0x3e;   	//Contrast
	data[1] = 0x28;
	transmitCmdData(0xC5, data, 2);    	//VCM control

	data[0] = 0x86;  	 //--
	transmitCmdData(0xC7, data, 1);    	//VCM control2

	data[0] = 0x48;  	//C8	   //48 68绔栧睆//28 E8 妯睆
	transmitCmdData(0x36, data, 1);    	// Memory Access Control

	data[0] = 0x55;
	transmitCmdData(0x3A, data, 1);

	data[0] = 0x00;
	data[1] = 0x18;
	transmitCmdData(0xB1, data, 2);

	data[0] = 0x08;
	data[1] = 0x82;
	data[2] = 0x27;
	transmitCmdData(0xB6, data, 3);    	// Display Function Control

	data[0] = 0x00;
	transmitCmdData(0xF2, data, 1);    	// 3Gamma Function Disable

	data[0] = 0x01;
	transmitCmdData(0x26, data, 1);    	//Gamma curve selected

	data[0] = 0x0F;
	data[1] = 0x31;
	data[2] = 0x2B;
	data[3] = 0x0C;
	data[4] = 0x0E;
	data[5] = 0x08;
	data[6] = 0x4E;
	data[7] = 0xF1;
	data[8] = 0x37;
	data[9] = 0x07;
	data[10] = 0x10;
	data[11] = 0x03;
	data[12] = 0x0E;
	data[13] = 0x09;
	data[14] = 0x00;
	transmitCmdData(0xE0, data, 15);    	//Set Gamma

	data[0] = 0x00;
	data[1] = 0x0E;
	data[2] = 0x14;
	data[3] = 0x03;
	data[4] = 0x11;
	data[5] = 0x07;
	data[6] = 0x31;
	data[7] = 0xC1;
	data[8] = 0x48;
	data[9] = 0x08;
	data[10] = 0x0F;
	data[11] = 0x0C;
	data[12] = 0x31;
	data[13] = 0x36;
	data[14] = 0x0F;
	transmitCmdData(0xE1, data, 15);    	//Set Gamma

	transmitCmdData(0x11, 0, 0);    	//Exit Sleep
	os_delay_us(120000);

	transmitCmdData(0x29, 0, 0);    //Display on
	transmitCmdData(0x2c, 0, 0);
}

void tft_init(void)
{
	uint16_t initColor = 0;
	hspi_init();

	TFT_CS_INIT;
	TFT_DC_INIT;
	TFT_RST_INIT;

	TFT_RST_ACTIVE;
	os_delay_us(10000);
	TFT_RST_DEACTIVE;
	os_delay_us(1000);

	tft_readId();
	tft_configRegister();
	tft_fillRectangle(MIN_TFT_X, MAX_TFT_X, MIN_TFT_Y, MAX_TFT_Y, &initColor, 1);
}

void tft_fillRectangle(uint16_t xLeft, uint16_t xRight, uint16_t yUp, uint16_t yDown, uint16_t *colors, uint32_t amountColor)
{
    uint32_t i = 0;

    if (xLeft > xRight) swap(&xLeft, &xRight);
    if (yUp > yDown) swap(&yUp, &yDown);

    constrain(&xLeft, MIN_TFT_X, MAX_TFT_X);
    constrain(&xRight, MIN_TFT_X, MAX_TFT_X);
    constrain(&yUp, MIN_TFT_Y, MAX_TFT_Y);
    constrain(&yDown, MIN_TFT_Y, MAX_TFT_Y);

    setCol(xLeft, xRight);
    setPage(yUp, yDown);
    transmitCmdData(0x2C, 0, 0);//  start to write to display RAM

    if (amountColor == 1)
    {
    	uint32_t numRepeat = (xRight - xLeft + 1) * (yDown - yUp + 1);
        transmitData((uint8_t *)colors, 2, numRepeat);
    }
    else
    {
        transmitData((uint8_t *)colors, 2 * amountColor, 1);
    }
}




void tft_setPixel(uint16_t poX, uint16_t poY, uint16_t color)
{
	tft_fillRectangle(poX, poX, poY, poY, &color, 1);
}

void tft_drawLine( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{

#define USE_OPTIMIZATION_DRAWLINE

    int16_t dx = abs(x1 - x0);
    int16_t dy = -abs(y1 - y0);
    int8_t sx = (x0 < x1) ? 1 : -1;
    int8_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx + dy;
    int16_t e2 = 0;

#ifdef USE_OPTIMIZATION_DRAWLINE
	uint16_t startX = x0;
	uint16_t startY = y0;
#endif

    for (;;)
    {
		#ifdef USE_OPTIMIZATION_DRAWLINE
    		// Require correction
			if ((startX != x0) && (startY != y0)) // draw line and not draw point
			{
				tft_fillRectangle(startX, x0 - sx, startY, y0 - sy, &color, 1);
				startX = x0;
				startY = y0;
			}
		#else
			tft_setPixel(x0, y0, color);
		#endif

        e2 = 2*err;
        if (e2 >= dy)
        {
            if (x0 == x1) break;
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            if (y0 == y1) break;
            err += dx;
            y0 += sy;
        }
    }
#ifdef USE_OPTIMIZATION_DRAWLINE
	tft_fillRectangle(startX, x0, startY, y0, &color, 1);
#endif
}

void tft_drawChar(int16_t ascii, uint16_t posX, uint16_t posY, uint8_t sizeFont, uint16_t colorFont, uint16 colorBackground)
{
	uint8_t i = 0;
	uint16_t * color = (uint16_t *)os_malloc(sizeof(uint16_t) * FONT_Y * sizeFont);

	ascii = ( (ascii < 0x20) || (ascii > 0x7F) ) ? 0x20 : ascii - 0x20;

    for (i = 0; i < FONT_X; ++i )
    {
    	uint8_t j = 0;
    	uint8_t k = 0;
    	uint16_t coordX = posX + i * sizeFont;

		for(j = 0; j < FONT_Y; ++j)
		{
			uint16_t colorActual = ((simpleFont[ascii][i] >> j) & 0x01) ? colorFont : colorBackground;
			for (k = 0; k < sizeFont; k++)
				color[j * sizeFont + k] = colorActual;
		}

		for (k = 0; k < sizeFont; k++)
		{
			tft_fillRectangle( coordX, coordX, posY, posY + FONT_Y * sizeFont, color, FONT_Y * sizeFont);
			coordX++;
		}
    }
    os_free(color);
}


void tft_drawString(char *str, uint16_t posX, uint16_t posY, uint8_t sizeFont, uint16_t colorFont, uint16 colorBackground)
{

	void nextString(void)
	{

	    tft_fillRectangle(posX, MAX_TFT_X, posY, posY + FONT_Y * sizeFont - 1, &colorBackground, 1);

	    posX = 0;
	    posY += FONT_Y * sizeFont + 1;

	    if( (posY + FONT_Y * sizeFont) > (MAX_TFT_Y + 1) )
	    {
	    	posX = 0;
	    	posY = 0;
	    }

	    //tft_fillRectangle(posX, MAX_TFT_X, posY, posY + FONT_Y * sizeFont - 1, &colorBackGround, 1);
	}
    while(*str)
    {
        tft_drawChar(*str, posX, posY, sizeFont, colorFont, colorBackground);
        str++;

        posX += FONT_X * sizeFont; // Move cursor right

        if ( (posX + FONT_X * sizeFont) > (MAX_TFT_X + 1) )
        	nextString();

        if (*str == 0x0A)
        {
        	str++;
        	nextString();
        }
    }
}

