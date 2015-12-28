/*
 *  Example read temperature and humidity from DHT22
 *  
 *  https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf
 *  https://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf
 * 
 *  For a single device, connect as follows:
 *  DHT22 1 (Vcc) to Vcc (3.3 Volts)
 *  DHT22 2 (DATA_OUT) to ESP GPIO (see Pin number)
 *  DHT22 3 (NC)
 *  DHT22 4 (GND) to GND
 *
 *  Pin number:
 *  -----------
 *  Pin 0 = GPIO16
 *  Pin 1 = GPIO5
 *  Pin 2 = GPIO4
 *  Pin 3 = GPIO0
 *  Pin 4 = GPIO2
 *  Pin 5 = GPIO14
 *  Pin 6 = GPIO12
 *  Pin 7 = GPIO13
 *  Pin 8 = GPIO15
 *  Pin 9 = GPIO3
 *  Pin 10 = GPIO1
 *  Pin 11 = GPIO9
 *  Pin 12 = GPIO10
 *
 * Between Vcc and DATA_OUT need to connect a pull-up resistor of 5 kOh.
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include "driver/uart.h"
#include "driver/dht22.h"
#include "driver/gpio16.h"

#define DELAY 2000 /* milliseconds */

LOCAL os_timer_t dht22_timer;
extern int ets_uart_printf(const char *fmt, ...);
int (*console_printf)(const char *fmt, ...) = ets_uart_printf;

extern uint8_t pin_num[GPIO_PIN_NUM];
DHT_Sensor sensor;
#define DHT_NUMBER_OF_SENSORS 2
DHT_Sensor sensors[DHT_NUMBER_OF_SENSORS];

LOCAL void ICACHE_FLASH_ATTR dht22_cb(void *arg)
{
	static uint8_t i;
	DHT_Sensor_Data data;
	uint8_t pin;
	os_timer_disarm(&dht22_timer);
#if 0
	// One DHT22 sensor
	pin = pin_num[sensor.pin];
	if (DHTRead(&sensor, &data))
	{
	    char buff[20];
	    console_printf("GPIO%d\r\n", pin);
	    console_printf("Temperature: %s *C\r\n", DHTFloat2String(buff, data.temperature));
	    console_printf("Humidity: %s %%\r\n", DHTFloat2String(buff, data.humidity));
	} else {
	    console_printf("Failed to read temperature and humidity sensor on GPIO%d\n", pin);
	}
#else
	// Two DHT22 sensors
	for (i = 0; i < DHT_NUMBER_OF_SENSORS; i++)
	{
	    pin = pin_num[sensors[i].pin];
		if (DHTRead(&sensors[i], &data))
		{
		    char buff[20];
		    console_printf("GPIO%d\r\n", pin);
		    console_printf("Temperature: %s *C\r\n", DHTFloat2String(buff, data.temperature));
		    console_printf("Humidity: %s %%\r\n", DHTFloat2String(buff, data.humidity));
		    console_printf("--------------------\r\n");
		} else {
		    console_printf("Failed to read temperature and humidity sensor on GPIO%d\n", pin);
		}
	}
#endif
	os_timer_arm(&dht22_timer, DELAY, 1);
}

void user_rf_pre_init(void)
{
}

void user_init(void)
{
	UARTInit(BIT_RATE_115200);
	console_printf("\r\n");

/*
 *  Pin number:
 *  -----------
 *  Pin 0 = GPIO16
 *  Pin 1 = GPIO5
 *  Pin 2 = GPIO4
 *  Pin 3 = GPIO0
 *  Pin 4 = GPIO2
 *  Pin 5 = GPIO14
 *  Pin 6 = GPIO12
 *  Pin 7 = GPIO13
 *  Pin 8 = GPIO15
 *  Pin 9 = GPIO3
 *  Pin 10 = GPIO1
 *  Pin 11 = GPIO9
 *  Pin 12 = GPIO10
 */

#if 0
	// One DHT22 sensor
	// Pin number 4 = GPIO2
	sensor.pin = 4;
	sensor.type = DHT22;
	console_printf("DHT22 init on GPIO%d\r\n", pin_num[sensor.pin]);
	DHTInit(&sensor);
#else
	// Two DHT22 sensors
	// Sensor 1: Pin number 3 = GPIO0
	sensors[0].pin = 3;
	sensors[0].type = DHT22;
	if (DHTInit(&sensors[0]))
	    console_printf("DHT22 #0 init on GPIO%d\r\n", pin_num[sensors[0].pin]);
	else
	    console_printf("Error init DHT22 #0 on GPIO%d\r\n", pin_num[sensors[0].pin]);
	// Sensor 2: Pin number 4 = GPIO2
	sensors[1].pin = 4;
	sensors[1].type = DHT22;
	if (DHTInit(&sensors[1]))
	    console_printf("DHT22 #1 init on GPIO%d\r\n", pin_num[sensors[1].pin]);
	else
		console_printf("Error init DHT22 #1 on GPIO%d\r\n", pin_num[sensors[1].pin]);
#endif

	console_printf("--------------------\r\n");
	os_timer_disarm(&dht22_timer);
	os_timer_setfn(&dht22_timer, (os_timer_func_t *)dht22_cb, (void *)0);
	os_timer_arm(&dht22_timer, DELAY, 1);
}
