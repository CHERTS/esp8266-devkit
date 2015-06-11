/*
 *  Examples read voltage from INA219
 *
 *	INA219 connected to
 *	I2C SDA - GPIO2
 *	I2C SCK - GPIO14
 *
 */

#include "ets_sys.h"
#include "driver/i2c.h"
#include "driver/i2c_ina219.h"
#include "driver/uart.h"
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
    if(INA219_GetVal(CONFIGURE_READ_POWERDOWN))
    {
        ets_uart_printf("Get voltage...\r\n");
    	data = INA219_GetVal(GET_VOLTAGE);
    	os_sprintf(temp, "Voltage: %d\r\n", data);
    	ets_uart_printf(temp);
    }
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
    // Init uart
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(1000);

    ets_uart_printf("Booting...\r\n");

    // Init
    if (INA219_Init()) {
    	ets_uart_printf("INA219 init done.\r\n");
        //Disarm timer
        os_timer_disarm(&sensor_timer);
        //Setup timer
        os_timer_setfn(&sensor_timer, (os_timer_func_t *)sensor_timerfunc, NULL);
        //Arm timer for every 10 sec.
        os_timer_arm(&sensor_timer, 5000, 1);
    }
    else
    	ets_uart_printf("INA219 init error.\r\n");

    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}

