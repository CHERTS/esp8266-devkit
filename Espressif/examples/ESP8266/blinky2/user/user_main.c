/*
	The blinky demo using an os timer
	TODO : work out why this resets after a while
*/

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>

// see eagle_soc.h for these definitions
#define LED_GPIO 2
#define LED_GPIO_MUX PERIPHS_IO_MUX_GPIO2_U
#define LED_GPIO_FUNC FUNC_GPIO2

#define DELAY 100 /* milliseconds */

LOCAL os_timer_t blink_timer;
LOCAL uint8_t led_state=0;

LOCAL void ICACHE_FLASH_ATTR blink_cb(void *arg)
{
	GPIO_OUTPUT_SET(LED_GPIO, led_state);
	led_state ^=1;
}

void user_rf_pre_init(void)
{
}

void user_init(void)
{
	// Configure pin as a GPIO
	PIN_FUNC_SELECT(LED_GPIO_MUX, LED_GPIO_FUNC);
	// Set up a timer to blink the LED
	// os_timer_disarm(ETSTimer *ptimer)
	os_timer_disarm(&blink_timer);
	// os_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg)
	os_timer_setfn(&blink_timer, (os_timer_func_t *)blink_cb, (void *)0);
	// void os_timer_arm(ETSTimer *ptimer,uint32_t milliseconds, bool repeat_flag)
	os_timer_arm(&blink_timer, DELAY, 1);
}
