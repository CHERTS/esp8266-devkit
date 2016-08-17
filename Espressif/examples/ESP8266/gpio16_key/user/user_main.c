/*
 *  Example testing 2 buttons
 *  
 *  Button 1 is connected to GPIO0 (Pin 3) ESP-01
 *  Button 2 is connected to GPIO2 (Pin 4) ESP-01
 *
 *  Pin number:
 *  -----------
 *  Pin 0 = GPIO16
 *  Pin 1 = GPIO5
 *  Pin 2 = GPIO4
 *  Pin 3 = GPIO0
 *  Pin 4 = GPIO2
 *  Pin 5 = GPIO14
 *  Pin 6 = GPIO12
 *  Pin 7 = GPIO13
 *  Pin 8 = GPIO15
 *  Pin 9 = GPIO3
 *  Pin 10 = GPIO1
 *  Pin 11 = GPIO9
 *  Pin 12 = GPIO10
 *
 */

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "os_type.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "driver/gpio16.h"

extern int ets_uart_printf(const char *fmt, ...);
int (*console_printf)(const char *fmt, ...) = ets_uart_printf;

extern uint8_t pin_num[GPIO_PIN_NUM];

// GPIO_PIN_INTR_NEGEDGE - down
// GPIO_PIN_INTR_POSEDGE - up
// GPIO_PIN_INTR_ANYEDGE - both
// GPIO_PIN_INTR_LOLEVEL - low level
// GPIO_PIN_INTR_HILEVEL - high level
// GPIO_PIN_INTR_DISABLE - disable interrupt
const char *gpio_type_desc[] =
{
	    "GPIO_PIN_INTR_DISABLE (DISABLE INTERRUPT)",
	    "GPIO_PIN_INTR_POSEDGE (UP)",
	    "GPIO_PIN_INTR_NEGEDGE (DOWN)",
	    "GPIO_PIN_INTR_ANYEDGE (BOTH)",
	    "GPIO_PIN_INTR_LOLEVEL (LOW LEVEL)",
	    "GPIO_PIN_INTR_HILEVEL (HIGH LEVEL)"
};

void ICACHE_FLASH_ATTR intr_callback(unsigned pin, unsigned level)
{
	console_printf("INTERRUPT: GPIO%d = %d\r\n", pin_num[pin], level);
	if(pin == 3) {
		if (gpio_intr_deattach(pin))
			console_printf("GPIO%d deattach\r\n", pin_num[pin]);
	}
}

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABBBCDDD
 *                A : rf cal
 *                B : at parameters
 *                C : rf init data
 *                D : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 8;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{
}

void ICACHE_FLASH_ATTR user_init(void)
{
	GPIO_INT_TYPE gpio_type;
	uint8_t gpio_pin;

	UARTInit(BIT_RATE_115200);
	console_printf("\r\n");
	// Set Wifi softap mode
	wifi_station_disconnect();
	wifi_station_set_auto_connect(0);
	wifi_set_opmode(SOFTAP_MODE);

	// Test GPIO interrupt
	// Pin number 3 = GPIO0
	gpio_pin = 3;
	gpio_type = GPIO_PIN_INTR_POSEDGE;
	if (set_gpio_mode(gpio_pin, GPIO_PULLUP, GPIO_INT)) {
		console_printf("GPIO%d set interrupt mode\r\n", pin_num[gpio_pin]);
		if (gpio_intr_init(gpio_pin, gpio_type)) {
			console_printf("GPIO%d enable %s mode\r\n", pin_num[gpio_pin], gpio_type_desc[gpio_type]);
			gpio_intr_attach(intr_callback);
		} else {
			console_printf("Error: GPIO%d not enable %s mode\r\n", pin_num[gpio_pin], gpio_type_desc[gpio_type]);
		}
	} else {
		console_printf("Error: GPIO%d not set interrupt mode\r\n", pin_num[gpio_pin]);
	}

	// Pin number 4 = GPIO2
	gpio_pin = 4;
	gpio_type = GPIO_PIN_INTR_POSEDGE;
	if (set_gpio_mode(gpio_pin, GPIO_PULLUP, GPIO_INT)) {
		console_printf("GPIO%d set interrupt mode\r\n", pin_num[gpio_pin]);
		if (gpio_intr_init(gpio_pin, gpio_type)) {
			console_printf("GPIO%d enable %s mode\r\n", pin_num[gpio_pin], gpio_type_desc[gpio_type]);
			gpio_intr_attach(intr_callback);
		} else {
			console_printf("Error: GPIO%d not enable %s mode\r\n", pin_num[gpio_pin], gpio_type_desc[gpio_type]);
		}
	} else {
		console_printf("Error: GPIO%d not set interrupt mode\r\n", pin_num[gpio_pin]);
	}

}
