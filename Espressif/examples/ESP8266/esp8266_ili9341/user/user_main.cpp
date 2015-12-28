//#define UIDEMO


#include "Adafruit_ILI9341_fast_as.h"
#include "cube.h"
// =============================================================================================
// C includes and declarations
// =============================================================================================
#include "cpp_routines/routines.h"
#include <time.h>

extern "C"
{
#include "espmissingincludes.h"
// declare lib methods
extern int ets_uart_printf(const char *fmt, ...);
void ets_timer_disarm(ETSTimer *ptimer);
void ets_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg);
}

LOCAL os_timer_t timerHandler;
Adafruit_ILI9341 tft;

#ifdef UIDEMO
extern void updateScreen(bool mode);
extern void setupUI();

float target_room_temperature = 23.5;
float RW_temperature = 65;
float target_RW_temperature = 70;
float room1_temperature = 23.4;
float room2_temperature = 23.3;
float outside_temperature = 2.4;
float min_target_temp = 18;
float max_target_temp = 26;
bool heater_enabled = false;
unsigned long room1_updated = -1;
unsigned long room2_updated = -1;
unsigned long outside_updated = -1;
unsigned long heater_state_changed_time = 0;
time_t total_on_time = 1;
time_t total_off_time = 1;
time_t last24h_on_time = 1;
time_t last24h_off_time = 1;

ICACHE_FLASH_ATTR static void updateScreen(void)
{
	REG_SET_BIT(0x3ff00014, BIT(0));
	os_update_cpu_frequency(160);

	target_room_temperature += 0.1;
	if (target_room_temperature > max_target_temp)
		target_room_temperature = min_target_temp;
	updateScreen((system_get_rtc_time() / 1000000) % 2);
}
#else
LOCAL double degree = -180.0;
LOCAL double scale = 1.5;
int16_t current[VERTEX_COUNT][3];
int16_t previous[VERTEX_COUNT][3];

ICACHE_FLASH_ATTR static void updateScreen(void)
{
	REG_SET_BIT(0x3ff00014, BIT(0));
	os_update_cpu_frequency(160);
	cube_calculate(current, degree, 0, 0, scale, 0, 0, 0);
	degree += 4;
	if (degree > 180.0) degree -= 360.0;
	cube_draw(previous, 0);
	cube_draw(current, 0XFFFF);
    memcpy(previous, current, sizeof (previous));
}
#endif

ICACHE_FLASH_ATTR void sendMsgToHandler(void *arg)
{
	system_os_post(USER_TASK_PRIO_0, UPDATE_SCREEN, 'a');
}

ICACHE_FLASH_ATTR void handler_task (os_event_t *e)
{
	switch (e->sig)
	{
		case UPDATE_SCREEN: updateScreen(); break;
		default: break;
	}
}

extern "C" void user_rf_pre_init(void)
{
}

extern "C" ICACHE_FLASH_ATTR void user_init(void)
{
	// Configure the UART
	uart_init(BIT_RATE_115200,BIT_RATE_115200);
	ets_uart_printf("\r\nSystem init...\r\n");
	do_global_ctors();
	ets_uart_printf("\r\nGlobal constructors invoked\r\n");
	os_event_t *handlerQueue;

	// Initialize TFT
	tft.begin();

	tft.fillScreen(0);
#ifdef UIDEMO
	setupUI();
	target_room_temperature = min_target_temp;
#endif

	// Set up a timer to send the message to handler
	os_timer_disarm(&timerHandler);
	os_timer_setfn(&timerHandler, (os_timer_func_t *)sendMsgToHandler, (void *)0);
	os_timer_arm(&timerHandler, 1, 1);

	// Set up a timerHandler to send the message to handler
	handlerQueue = (os_event_t *)os_malloc(sizeof(os_event_t)*TEST_QUEUE_LEN);
	system_os_task(handler_task, USER_TASK_PRIO_0, handlerQueue, TEST_QUEUE_LEN);

	ets_uart_printf("System init done \r\n");
}
