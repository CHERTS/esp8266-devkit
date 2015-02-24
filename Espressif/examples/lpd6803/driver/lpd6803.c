/*
 * lpd6803.c
 *
 *  Created on: 13 Dec 2014 
 *      Author: Aleksey Kudakov <popsodav@gmail.com>
 */
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "driver/lpd6803.h"

uint16_t lpd6803_pixels[numLEDs];

int lpd6803_SendMode;   // Used in interrupt 0=start,1=header,2=data,3=data done
uint32_t lpd6803_BitCount;   // Used in interrupt
uint16_t lpd6803_LedIndex;   // Used in interrupt - Which LED we are sending.
uint32_t lpd6803_BlankCounter;  //Used in interrupt.

uint32_t lpd6803_lastdata = 0;
uint16_t lpd6803_swapAsap = 0; //flag to indicate that the colors need an update asap
uint8_t lpd6803_mode = 0;

//Running Pixel and Running Line modes variables
uint16_t lpd6803_mode_rp_CurrentPixel = 0;
uint16_t lpd6803_mode_rp_PixelColor = 0;
bool lpd6803_mode_rl_initialPixel = true;

void ICACHE_FLASH_ATTR lpd6803_LedOut() {
	switch (lpd6803_SendMode) {
	case LPD6803_DONE:            //Done..just send clocks with zero data
		if (lpd6803_swapAsap > 0) {
			if (!lpd6803_BlankCounter) //AS SOON AS CURRENT pwm IS DONE. BlankCounter
			{
				lpd6803_BitCount = 0;
				lpd6803_LedIndex = lpd6803_swapAsap;  //set current led
				lpd6803_SendMode = LPD6803_HEADER;
				lpd6803_swapAsap = 0;
			}
		}
		break;

	case LPD6803_DATA:               //Sending Data
		if ((1 << (15 - lpd6803_BitCount)) & lpd6803_pixels[lpd6803_LedIndex]) {
			if (!lpd6803_lastdata) { // digitalwrites take a long time, avoid if possible
				// If not the first bit then output the next bits
				// (Starting with MSB bit 15 down.)
				GPIO_OUTPUT_SET(0, 1);
				lpd6803_lastdata = 1;
			}
		} else {
			if (lpd6803_lastdata) { // digitalwrites take a long time, avoid if possible
				GPIO_OUTPUT_SET(0, 0);
				lpd6803_lastdata = 0;
			}
		}
		lpd6803_BitCount++;

		if (lpd6803_BitCount == 16)    //Last bit?
				{
			lpd6803_LedIndex++;        //Move to next LED
			if (lpd6803_LedIndex < numLEDs) //Still more leds to go or are we done?
			{
				lpd6803_BitCount = 0;  //Start from the fist bit of the next LED
			} else {
				// no longer sending data, set the data pin low
				GPIO_OUTPUT_SET(0, 0);
				lpd6803_lastdata = 0;      // this is a lite optimization
				lpd6803_SendMode = LPD6803_DONE; //No more LEDs to go, we are done!
			}
		}
		break;
	case LPD6803_HEADER:            //Header
		if (lpd6803_BitCount < 32) {
			GPIO_OUTPUT_SET(0, 0);
			lpd6803_lastdata = 0;
			lpd6803_BitCount++;
			if (lpd6803_BitCount == 32) {
				lpd6803_SendMode = LPD6803_DATA; //If this was the last bit of header then move on to data.
				lpd6803_LedIndex = 0;
				lpd6803_BitCount = 0;
			}
		}
		break;
	case LPD6803_START:            //Start
		if (!lpd6803_BlankCounter) //AS SOON AS CURRENT pwm IS DONE. BlankCounter
		{
			lpd6803_BitCount = 0;
			lpd6803_LedIndex = 0;
			lpd6803_SendMode = LPD6803_HEADER;
		}
		break;
	}

	// Clock out data (or clock LEDs)
	GPIO_OUTPUT_SET(2, 1);            //High
	GPIO_OUTPUT_SET(2, 0);            //Low

	//Keep track of where the LEDs are at in their pwm cycle.
	lpd6803_BlankCounter++;
}

void ICACHE_FLASH_ATTR lpd6803_setPixelColor(uint16_t n, uint8_t r, uint8_t g,
		uint8_t b) {
	uint16_t data;

	if (n > numLEDs)
		return;

	data = r & 0x1F;
	data <<= 5;
	data |= g & 0x1F;
	data <<= 5;
	data |= b & 0x1F;
	data |= 0x8000;

	lpd6803_pixels[n] = data;
}

void ICACHE_FLASH_ATTR lpd6803_setPixelColorByColor(uint16_t n, uint16_t c) {
	if (n > numLEDs)
		return;

	lpd6803_pixels[n] = 0x8000 | c;
}

void ICACHE_FLASH_ATTR lpd6803_init() {
	gpio_init();
	//Set GPIO2 to output mode for CLCK
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	GPIO_OUTPUT_SET(2, 0);

	//Set GPIO0 to output mode for DATA
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	GPIO_OUTPUT_SET(0, 0);

	uint16_t i;
	for (i = 0; i < numLEDs; i++) {
		lpd6803_setPixelColor(i, 0, 0, 0);
	}
}

void ICACHE_FLASH_ATTR lpd6803_show(void) {
	lpd6803_BitCount = lpd6803_LedIndex = lpd6803_BlankCounter = 0;
	lpd6803_SendMode = LPD6803_START;
}

// Create a 15 bit color value from R,G,B
unsigned int ICACHE_FLASH_ATTR lpd6803_Color(uint8_t r, uint8_t g, uint8_t b) {
	//Take the lowest 5 bits of each value and append them end to end
	return (((unsigned int) r & 0x1F) << 10 | ((unsigned int) g & 0x1F) << 5
			| ((unsigned int) b & 0x1F));
}

unsigned int ICACHE_FLASH_ATTR lpd6803_Wheel(uint8_t WheelPos) {
	uint8_t r, g, b;
	switch (WheelPos >> 5) {
	case 0:
		r = 31 - WheelPos % 32;   //Red down
		g = WheelPos % 32;      // Green up
		b = 0;                  //blue off
		break;
	case 1:
		g = 31 - WheelPos % 32;  //green down
		b = WheelPos % 32;      //blue up
		r = 0;                  //red off
		break;
	case 2:
		b = 31 - WheelPos % 32;  //blue down
		r = WheelPos % 32;      //red up
		g = 0;                  //green off
		break;
	}
	return (lpd6803_Color(r, g, b));
}

//Do nothing function

void ICACHE_FLASH_ATTR lpd6803_loop() {
	switch (lpd6803_mode) {
	case (LPD6803_MODE_RUNNING_PIXEL):
		lpd6803_RunningPixel_loop();
		break;
	case (LPD6803_MODE_RUNNING_LINE):
		lpd6803_RunningLine_loop();
		break;
	}
}

void ICACHE_FLASH_ATTR lpd6803_RunningLine_loop() {
	uint16_t i;
	if (lpd6803_mode_rp_CurrentPixel == 0) {
		for (i = 0; i < numLEDs; i++) {
			lpd6803_setPixelColor(i, 0, 0, 0);
		}
	}

	if (!lpd6803_mode_rl_initialPixel) {
		lpd6803_setPixelColorByColor(lpd6803_mode_rp_CurrentPixel,
				lpd6803_mode_rp_PixelColor);
		lpd6803_mode_rp_CurrentPixel++;
	} else {
		lpd6803_mode_rl_initialPixel = false;
	}

	if (lpd6803_mode_rp_CurrentPixel >= numLEDs) {
		lpd6803_mode_rp_CurrentPixel = 0;
		lpd6803_mode_rl_initialPixel = true;
	}

	lpd6803_show();
}

void ICACHE_FLASH_ATTR lpd6803_startRunningLine(uint16_t color) {
	lpd6803_mode_rp_PixelColor = color;
	lpd6803_mode_rp_CurrentPixel = 0;
	lpd6803_mode_rl_initialPixel = true;
	lpd6803_mode = LPD6803_MODE_RUNNING_LINE;
}

void ICACHE_FLASH_ATTR lpd6803_RunningPixel_loop() {
	uint16_t i;
	lpd6803_setPixelColorByColor(lpd6803_mode_rp_CurrentPixel,
			lpd6803_mode_rp_PixelColor);

	if (lpd6803_mode_rp_CurrentPixel <= 0) {
		for (i = 1; i < numLEDs; i++) {
			lpd6803_setPixelColor(i, 0, 0, 0);
		}
	} else {
		for (i = 0; i < numLEDs; i++) {
			if (i != lpd6803_mode_rp_CurrentPixel) {
				lpd6803_setPixelColor(i, 0, 0, 0);
			}
		}
	}

	lpd6803_mode_rp_CurrentPixel++;

	if (lpd6803_mode_rp_CurrentPixel >= numLEDs) {
		lpd6803_mode_rp_CurrentPixel = 0;
	}

	lpd6803_show();
}

void ICACHE_FLASH_ATTR lpd6803_startRunningPixel(uint16_t color) {
	lpd6803_mode_rp_PixelColor = color;
	lpd6803_mode_rp_CurrentPixel = 0;
	lpd6803_mode = LPD6803_MODE_RUNNING_PIXEL;
}
