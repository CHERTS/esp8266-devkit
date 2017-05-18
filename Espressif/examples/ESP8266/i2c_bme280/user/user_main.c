/*
 *  Examples read temperature, humidity and pressure from BME280
 *
 *	BME280 connected to
 *	I2C SDA - GPIO2
 *	I2C SCL - GPIO0
 *
 */

#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include <user_interface.h>
#include "user_config.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "driver/bme280.h"

static os_timer_t sensor_timer;

void sensor_timerfunc(void *arg)
{
	sint32 temperature;
	uint32 pressure, humidity;
	char data[100];

	if(BME280_ReadAll(&temperature, &pressure, &humidity))
		ets_uart_printf("Sensor read error!\r\n");
	else
	{
		os_sprintf(data, "Temperature: %d.%02u C, Humidity: %u.%02u rH, Pressure: %u.%02u hPa",
					temperature / 100, temperature % 100,							//C
					humidity >> 10, ((humidity & 0x000003FF) * 100) >> 10,			//rH
					//pressure >> 8, ((pressure & 0x000000FF) * 100) >> 8,			//Pa
					(pressure >> 8) / 100, (pressure >> 8) % 100);					//hPa
		ets_uart_printf(data);
		ets_uart_printf("\r\n");
	}
}

void user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(65535);

	i2c_master_gpio_init();

	ets_uart_printf("Started!\r\n");

	if(BME280_Init(BME280_OS_T_16, BME280_OS_P_16, BME280_OS_H_16,
					BME280_FILTER_16, BME280_MODE_NORMAL, BME280_TSB_05))
    		ets_uart_printf("BMP180 init error.\r\n");
	else {
    		ets_uart_printf("BMP180 init done.\r\n");
	        os_timer_disarm(&sensor_timer);
	        os_timer_setfn(&sensor_timer, (os_timer_func_t *)sensor_timerfunc, NULL);
        	os_timer_arm(&sensor_timer, 5000, 1);
	}

}

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
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

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
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
