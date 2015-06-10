/*
 *  Examples write data in eeprom series Microchip 24xx16 (24AA16/24LC16B)
 *  http://ww1.microchip.com/downloads/en/DeviceDoc/21703L.pdf
 *
 */

#include "ets_sys.h"
#include "driver/i2c.h"
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
    uint8 data;
    char temp[80];
    data = eeprom_readByte(0x50,0x01);
    os_sprintf(temp, "Byte: %x \n\r",data);
    ets_uart_printf(temp);
    data = data + 1;
    if(!eeprom_writeByte(0x50,0x01,data))
        ets_uart_printf("Write failed\n\r");
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

    // i2c
    i2c_init();

    if(!eeprom_writeByte(0x50,0x01,0x00))
    	ets_uart_printf("Write failed\n\r");

    //Disarm timer
    os_timer_disarm(&sensor_timer);

    //Setup timer
    os_timer_setfn(&sensor_timer, (os_timer_func_t *)sensor_timerfunc, NULL);

    //Arm timer for every 10 sec.
    os_timer_arm(&sensor_timer, 5000, 1);

    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}

