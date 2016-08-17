/*
	Print some system info
*/

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include "user_interface.h"
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
