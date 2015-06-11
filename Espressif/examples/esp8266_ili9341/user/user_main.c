/*
 *  Example of working ESP8266 and 2.2 Inch SPI TFT LCD Serial Port Module Display ILI9341
 *  Specification on ILI9341 - http://www.newhavendisplay.com/app_notes/ILI9341.pdf
 *  Specification on ESP8266 - http://www.esp8266.com/
 *
 *  Original source https://github.com/Perfer/esp8266_ili9341
 *
 *  Author: Semen Sachkov
 *
 */

#include "user_config.h"

extern int ets_uart_printf(const char *fmt, ...);

LOCAL os_timer_t timerHandler;

LOCAL double degree = -180.0;
LOCAL double scale = 1.0;
LOCAL double scale_inc = 0.01;

LOCAL uint16_t count = 0;

LOCAL void ICACHE_FLASH_ATTR  test(void)
{
	char time[10];
	uint8_t size = 2;


	cube_draw(0);
	if (degree >= 180.0) degree = -180.0;
	if ((scale < 0.5) && (scale_inc < 0)) scale_inc = -scale_inc;
	if ((scale > 1.5) && (scale_inc > 0)) scale_inc = -scale_inc;
	cube_calculate(degree, degree, degree, scale, 0, 0, 0);
	degree += 0.5;
	scale += scale_inc;
	cube_draw(0xFFFF);
	ets_uart_printf("Degree: %d \r\n", (int)degree);
/*

	count += 1;
	os_sprintf(time, "%d", count);
	tft_drawString(time, 0, 0, size, 0xFFFF, 0x0000);
	//tft_drawString("12:42:34", 0, 40, size, 0xFFFF, 0x0000);
	//tft_drawString("00:01:18", 0, 60, size, 0xFFFF, 0x0000);
*/
}

LOCAL void ICACHE_FLASH_ATTR sendMsgToHandler(void *arg)
{
	system_os_post(USER_TASK_PRIO_0, RUN_TEST, 'a');
}

LOCAL void ICACHE_FLASH_ATTR handler_task (os_event_t *e)
{
	switch (e->sig)
	{
		case RUN_TEST: test(); break;
		default: break;
	}
}

void user_rf_pre_init(void)
{
}

void user_init(void)
{
	os_event_t *handlerQueue;
	// Configure the UART
	uart_init(BIT_RATE_115200,BIT_RATE_115200);
	ets_uart_printf("\r\nSystem init...\r\n");

	// Initialize TFT
	tft_init();

	// Set up a timer to send the message to handler
	os_timer_disarm(&timerHandler);
	os_timer_setfn(&timerHandler, (os_timer_func_t *)sendMsgToHandler, (void *)0);
	os_timer_arm(&timerHandler, DELAY_TIMER, 1);

	// Set up a timerHandler to send the message to handler
	handlerQueue = (os_event_t *)os_malloc(sizeof(os_event_t)*TEST_QUEUE_LEN);
	system_os_task(handler_task, USER_TASK_PRIO_0, handlerQueue, TEST_QUEUE_LEN);

	ets_uart_printf("System init done \r\n");

}
