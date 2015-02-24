/*
 * user_ws2801.c
 *
 *  Created on: Nov 18, 2014
 *      Author: frans-willem
 */
#include <c_types.h>
#include <eagle_soc.h>
#include <gpio.h>

#define BIT_CLK		BIT0
#define BIT_DATA	BIT2


void ws2801_byte(uint8_t n) {
	uint8_t bitmask;
	for (bitmask=0x80; bitmask!=0; bitmask>>=1) {
		if (n & bitmask) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, BIT_DATA);
		else GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, BIT_DATA);
		GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, BIT_CLK);
		GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, BIT_CLK);
	}
}

void ws2801_color(uint8_t r, uint8_t g, uint8_t b) {
	ws2801_byte(r);
	ws2801_byte(g);
	ws2801_byte(b);
}

void ws2801_strip(uint8_t *data, uint16_t len) {
	while (len--) {
		ws2801_byte(*(data++));
	}
	GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, BIT_DATA);
}

void ws2801_init() {
    //Set GPIO2 and GPIO0 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);

    //Set both GPIOs low low
    gpio_output_set(0, BIT_CLK|BIT_DATA, BIT_CLK|BIT_DATA, 0);
}
