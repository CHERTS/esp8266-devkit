/*
	Example read temperature from DS18B20 chip
	DS18B20 connected to GPIO2

*/

#include <ets_sys.h>
#include <osapi.h>
#include "user_interface.h"
#include <os_type.h>
#include <gpio.h>
#include "driver/uart.h"
#include "driver/ds18b20.h"

#define DELAY 2000 /* milliseconds */
#define sleepms(x) os_delay_us(x*1000);

LOCAL os_timer_t ds18b20_timer;
extern int ets_uart_printf(const char *fmt, ...);
int (*console_printf)(const char *fmt, ...) = ets_uart_printf;

int ds18b20();

LOCAL void ICACHE_FLASH_ATTR ds18b20_cb(void *arg)
{
	ds18b20();
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

void ICACHE_FLASH_ATTR user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_timer_disarm(&ds18b20_timer);
	os_timer_setfn(&ds18b20_timer, (os_timer_func_t *)ds18b20_cb, (void *)0);
	os_timer_arm(&ds18b20_timer, DELAY, 1);
}

int ICACHE_FLASH_ATTR ds18b20()
{
	int r, i;
	uint8_t addr[8], data[12];
	
	ds_init();

	r = ds_search(addr);
	if(r)
	{
		console_printf("Found Device @ %02x %02x %02x %02x %02x %02x %02x %02x\r\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
		if(crc8(addr, 7) != addr[7])
			console_printf( "CRC mismatch, crc=%xd, addr[7]=%xd\r\n", crc8(addr, 7), addr[7]);

		switch(addr[0])
		{
		case 0x10:
			console_printf("Device is DS18S20 family.\r\n");
			break;

		case 0x28:
			console_printf("Device is DS18B20 family.\r\n");
			break;

		default:
			console_printf("Device is unknown family.\r\n");
			return 1;
		}
	}
	else { 
		console_printf("No DS18B20 detected, sorry.\r\n");
		return 1;
	}
	// perform the conversion
	reset();
	select(addr);

	write(DS1820_CONVERT_T, 1); // perform temperature conversion

	sleepms(65); // sleep 65ms

	console_printf("Scratchpad: ");
	reset();
	select(addr);
	write(DS1820_READ_SCRATCHPAD, 0); // read scratchpad
	
	for(i = 0; i < 9; i++)
	{
		data[i] = read();
		console_printf("%2x ", data[i]);
	}
	console_printf("\r\n");

	int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;
	LowByte = data[0];
	HighByte = data[1];
	TReading = (HighByte << 8) + LowByte;
	SignBit = TReading & 0x8000;  // test most sig bit
	if (SignBit) // negative
		TReading = (TReading ^ 0xffff) + 1; // 2's comp
	
	Whole = TReading >> 4;  // separate off the whole and fractional portions
	Fract = (TReading & 0xf) * 100 / 16;

	console_printf("Temperature: %c%d.%d Celsius\r\n", SignBit ? '-' : '+', Whole, Fract < 10 ? 0 : Fract);
	return r;
}
