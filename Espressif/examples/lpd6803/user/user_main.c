#include "user_config.h"
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "driver/uart.h"
#include "driver/lpd6803.h"

extern int ets_uart_printf(const char *fmt, ...);

os_event_t user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);
static volatile os_timer_t some_timer;
static volatile os_timer_t lpd6803_timer;

static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events)
{
	os_delay_us(40);
}

void user_rf_pre_init(void)
{
}

//Init function 
void ICACHE_FLASH_ATTR user_init()
{
	system_timer_reinit();

	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(1000);
	ets_uart_printf("Loading LPD6803...\r\n");

	lpd6803_init();
	lpd6803_show();

	os_timer_disarm(&lpd6803_timer);
	os_timer_setfn(&lpd6803_timer, (os_timer_func_t *) lpd6803_LedOut, NULL);
	os_timer_arm_us(&lpd6803_timer, 40, 1);

	os_timer_disarm(&some_timer);
	os_timer_setfn(&some_timer, (os_timer_func_t *) lpd6803_loop, NULL);
	os_timer_arm(&some_timer, 200, 1);

	lpd6803_startRunningLine(lpd6803_Color(255, 0, 0));

	//Start os task
	system_os_task(user_procTask, user_procTaskPrio, user_procTaskQueue, user_procTaskQueueLen);
}
