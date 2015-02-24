/*
 *  Example read temperature and humidity from DHT22
 *  
 *  https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf
 *  https://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf
 * 
 *  For a single device, connect as follows:
 *  DHT22 1 (Vcc) to Vcc (3.3 Volts)
 *  DHT22 2 (DATA_OUT) to ESP Pin GPIO2
 *  DHT22 3 (NC)
 *  DHT22 4 (GND) to GND
 *
 * Between Vcc and DATA_OUT need to connect a pull-up resistor of 10 kOh.
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include "driver/uart.h"
#include "driver/dht22.h"

#define DELAY 2000 /* milliseconds */

LOCAL os_timer_t dht22_timer;
extern int ets_uart_printf(const char *fmt, ...);
int (*console_printf)(const char *fmt, ...) = ets_uart_printf;

LOCAL void ICACHE_FLASH_ATTR dht22_cb(void *arg)
{
	float * r = DHTRead();
	console_printf("Temperature: %d.%d *C, Humidity: %d.%d %%\r\n", (int)(r[0]),(int)((r[0] - (int)r[0])*100), (int)(r[1]),(int)((r[1] - (int)r[1])*100));
}

void user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	console_printf("\r\nDHT22 init\r\n");
	DHTInit(DHT22);
	os_timer_disarm(&dht22_timer);
	os_timer_setfn(&dht22_timer, (os_timer_func_t *)dht22_cb, (void *)0);
	os_timer_arm(&dht22_timer, DELAY, 1);
}
