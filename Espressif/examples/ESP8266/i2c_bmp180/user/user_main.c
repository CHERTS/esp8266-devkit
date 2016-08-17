/*
 *  Examples read temperature and pressure from BMP180
 *
 *	BMP180 connected to
 *	I2C SDA - GPIO2
 *	I2C SCL - GPIO0
 *
 */

#include "ets_sys.h"
#include "driver/i2c.h"
#include "driver/i2c_bmp180.h"
#include "driver/uart.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "user_config.h"

os_event_t user_procTaskQueue[user_procTaskQueueLen];
extern int ets_uart_printf(const char *fmt, ...);
int (*console_printf)(const char *fmt, ...) = ets_uart_printf;
static void user_procTask(os_event_t *events);
static volatile os_timer_t sensor_timer;

void sensor_timerfunc(void *arg)
{
    int32_t temperature;
    int32_t pressure;
	char buff[20];
    ets_uart_printf("Get temperature and pressure...\r\n");
    temperature = BMP180_GetTemperature();
    pressure = BMP180_GetPressure(OSS_0);
    console_printf("Temperature: %s *C\r\n", BMP180_Int2String(buff, temperature));
    console_printf("Temperature: %d.%d *F\r\n", (int)(9 * temperature / 50 + 32), (int)(9 * temperature / 5 % 10));
    console_printf("Pressure: %d mm rt.st.\r\n", (int)(pressure/133.322368));
    console_printf("Pressure: %d m water.st.\r\n", (int)(pressure/9806.65));
    console_printf("Pressure: %d.%d mbar\r\n", (int)(pressure/100), (int)(pressure%100));
    console_printf("Pressure: %d.%d mmHg\r\n", (int)(pressure * 75 / 10000), (int)((pressure * 75 % 10000) / 1000));
    console_printf("Altitude: %d\r\n", BMP180_CalcAltitude(pressure));
}

static void ICACHE_FLASH_ATTR
user_procTask(os_event_t *events)
{
    os_delay_us(5000);
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
    // Init uart
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(1000);

    ets_uart_printf("\r\nBooting...\r\n");

    // Init
    if (BMP180_Init()) {
    	ets_uart_printf("BMP180 init done.\r\n");
        //Disarm timer
        os_timer_disarm(&sensor_timer);
        //Setup timer
        os_timer_setfn(&sensor_timer, (os_timer_func_t *)sensor_timerfunc, NULL);
        //Arm timer for every 10 sec.
        os_timer_arm(&sensor_timer, 5000, 1);
    }
    else
    	ets_uart_printf("BMP180 init error.\r\n");

    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}

