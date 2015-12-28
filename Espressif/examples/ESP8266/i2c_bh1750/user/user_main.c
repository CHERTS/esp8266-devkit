/*
 *  Driver for BH1750
 *  http://rohmfs.rohm.com/en/products/databook/datasheet/ic/sensor/light/bh1750fvi-e.pdf
 * 
 *  For a single device, connect as follows:
 *  BH1750 GND to GND
 *  BH1750 Vcc to Vcc (3.3 Volts)
 *  BH1750 SDA to ESP Pin GPIO2
 *  BH1750 SCL to ESP Pin GPIO0 (see i2c.h)
 *  BH1750 ADD to GND
 *
 */

#include "ets_sys.h"
#include "driver/i2c.h"
#include "driver/bh1750.h"
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
	int data;
    char temp[80];
    ets_uart_printf("Get light...\r\n");
    data = GetLight();
    if(data == -1)
    	ets_uart_printf("Error get light value\r\n");
    else
    {
    	os_sprintf(temp,"Value#: %d\r\n", data);
    	ets_uart_printf(temp);
    }
}

static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events)
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

    ets_uart_printf("\r\nBooting...\r\n");

    // Init
    if (BH1750Init()) {
    	ets_uart_printf("BH1750 init done.\r\n");
        //Disarm timer
        os_timer_disarm(&sensor_timer);
        //Setup timer
        os_timer_setfn(&sensor_timer, (os_timer_func_t *)sensor_timerfunc, NULL);
        //Arm timer for every 10 sec.
        os_timer_arm(&sensor_timer, 2000, 1);
    }
    else
    	ets_uart_printf("BH1750 init error.\r\n");

    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}

