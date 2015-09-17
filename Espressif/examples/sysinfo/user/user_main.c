/*
	Print some system info
*/

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include "driver/uart.h"

#define DELAY 3000 /* milliseconds */

const char *FlashSizeMap[] =
{
		"512 KB (256 KB + 256 KB)",	// 0x00
		"256 KB",			// 0x01
		"1024 KB (512 KB + 512 KB)", 	// 0x02
		"2048 KB (512 KB + 512 KB)"	// 0x03
		"4096 KB (512 KB + 512 KB)"	// 0x04
		"2048 KB (1024 KB + 1024 KB)"	// 0x05
		"4096 KB (1024 KB + 1024 KB)"	// 0x06
};

extern int ets_uart_printf(const char *fmt, ...);
int (*console_printf)(const char *fmt, ...) = ets_uart_printf;

LOCAL os_timer_t info_timer;

void user_rf_pre_init(void)
{
}

LOCAL void ICACHE_FLASH_ATTR info_cb(void *arg)
{
	console_printf("==== System info: ====\r\n");
	console_printf("SDK version:%s rom %d\r\n", system_get_sdk_version(), system_upgrade_userbin_check());
	console_printf("Time = %ld\r\n", system_get_time());
	console_printf("Chip id = 0x%x\r\n", system_get_chip_id());
	console_printf("CPU freq = %d MHz\r\n", system_get_cpu_freq());
	console_printf("Flash size map = %s\r\n", FlashSizeMap[system_get_flash_size_map()]);
	console_printf("Free heap size = %d\r\n", system_get_free_heap_size());
	console_printf("==== End System info ====\r\n");
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
