/*
	The obligatory blinky demo
	Blink an LED on GPIO pin 2
*/

#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>

// see eagle_soc.h for these definitions
#define LED_GPIO 2
#define LED_GPIO_MUX PERIPHS_IO_MUX_GPIO2_U
#define LED_GPIO_FUNC FUNC_GPIO2

#define DELAY 500000 /* microseconds */

extern void ets_wdt_enable (void);
extern void ets_wdt_disable (void);

void user_rf_pre_init(void)
{
}

void user_init(void)
{
	uint8_t state=0;
	ets_wdt_enable();
	ets_wdt_disable();
	// Configure pin as a GPIO
	PIN_FUNC_SELECT(LED_GPIO_MUX, LED_GPIO_FUNC);
	for(;;)
	{
		GPIO_OUTPUT_SET(LED_GPIO, state);
		os_delay_us(DELAY);
		state ^=1;
	}
}
