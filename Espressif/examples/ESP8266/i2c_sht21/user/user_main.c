/*
 *  Examples using Humidity and Temperature Sensor SHT21
 *
 *  http://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/Humidity/Sensirion_Humidity_SHT21_Datasheet_V4.pdf
 *
 */

#include "ets_sys.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "driver/i2c_sht21.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "user_config.h"

os_event_t user_procTaskQueue[user_procTaskQueueLen];
extern int ets_uart_printf(const char *fmt, ...);
static void user_procTask(os_event_t *events);
static volatile os_timer_t sensor_timer;

void sensor_timerfunc(void *arg)
{
	uint32_t data;
    char temp[80];
    ets_uart_printf("Get temperature...\r\n");
    data = SHT21_GetVal(GET_SHT_TEMPERATURE);
    os_sprintf(temp, "Temperature: %d\r\n", data);
    ets_uart_printf(temp);
    ets_uart_printf("Get humidity...\r\n");
    data = SHT21_GetVal(GET_SHT_HUMIDITY);
    os_sprintf(temp, "Humidity: %d\r\n", data);
    ets_uart_printf(temp);
}

static void ICACHE_FLASH_ATTR
user_procTask(os_event_t *events)
{
    os_delay_us(5000);
}

void user_rf_pre_init(void)
{
}

void user_init(void)
{
    //Init uart
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(1000);

    ets_uart_printf("Booting...\r\n");

    // Init
    if (SHT21_Init()) {
    	ets_uart_printf("SHT21 init done.\r\n");
        //Disarm timer
        os_timer_disarm(&sensor_timer);
        //Setup timer
        os_timer_setfn(&sensor_timer, (os_timer_func_t *)sensor_timerfunc, NULL);
        //Arm timer for every 10 sec.
        os_timer_arm(&sensor_timer, 5000, 1);
    }
    else
    	ets_uart_printf("SHT21 init error.\r\n");

    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}

