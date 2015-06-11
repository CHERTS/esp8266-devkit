/*
	Print some system info
*/

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include "driver/uart.h"

#define DELAY 3000 /* milliseconds */

extern int ets_uart_printf(const char *fmt, ...);

LOCAL os_timer_t info_timer;

void user_rf_pre_init(void)
{
}

LOCAL void ICACHE_FLASH_ATTR info_cb(void *arg)
{
	char temp[80];
	ets_uart_printf("System info:\r\n");
	os_sprintf(temp, "SDK version: %s\r\n", system_get_sdk_version());
	ets_uart_printf(temp);
	os_sprintf(temp, "Time = %ld\r\n", system_get_time());
	ets_uart_printf(temp);
	os_sprintf(temp, "Chip id = 0x%x\r\n", system_get_chip_id());
	ets_uart_printf(temp);
	os_sprintf(temp, "CPU freq = %d MHz\r\n", system_get_cpu_freq());
	ets_uart_printf(temp);
	os_sprintf(temp, "Free heap size = %d\r\n", system_get_free_heap_size());
	ets_uart_printf(temp);
	ets_uart_printf("==========================================\r\n");
}

void user_init(void)
{
	// Configure the UART
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	// Set up a timer to send the message
	os_printf("\r\n");
	// os_timer_disarm(ETSTimer *ptimer)
	os_timer_disarm(&info_timer);
	// os_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg)
	os_timer_setfn(&info_timer, (os_timer_func_t *)info_cb, (void *)0);
	// void os_timer_arm(ETSTimer *ptimer,uint32_t milliseconds, bool repeat_flag)
	os_timer_arm(&info_timer, DELAY, 1);
}
