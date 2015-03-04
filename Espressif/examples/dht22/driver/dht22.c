/*
    Driver for the temperature and humidity sensor DHT11 and DHT22
    Official repository: https://github.com/CHERTS/esp8266-dht11_22

    Copyright (C) 2014 Mikhail Grigorev (CHERTS)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "ets_sys.h"
#include "osapi.h"
#include "c_types.h"
#include "user_interface.h"
#include "gpio.h"
#include "driver/dht22.h"
#include "driver/gpio16.h"

#ifdef DHT_DEBUG
#undef DHT_DEBUG
#define DHT_DEBUG(...) os_printf(__VA_ARGS__);
#else
#define DHT_DEBUG(...)
#endif

#define sleepms(x) os_delay_us(x*1000);
extern uint8_t pin_num[GPIO_PIN_NUM];

static inline float scale_humidity(DHTType sensor_type, int *data)
{
	if(sensor_type == DHT11) {
		return (float) data[0];
	} else {
		float humidity = data[0] * 256 + data[1];
		return humidity /= 10;
	}
}

static inline float scale_temperature(DHTType sensor_type, int *data)
{
	if(sensor_type == DHT11) {
		return (float) data[2];
	} else {
		float temperature = data[2] & 0x7f;
		temperature *= 256;
		temperature += data[3];
		temperature /= 10;
		if (data[2] & 0x80)
			temperature *= -1;
		return temperature;
	}
}

char* DHTFloat2String(char* buffer, float value)
{
  os_sprintf(buffer, "%d.%d", (int)(value),(int)((value - (int)value)*100));
  return buffer;
}

bool DHTRead(DHT_Sensor *sensor, DHT_Sensor_Data* output)
{
	int counter = 0;
	int laststate = 1;
	int i = 0;
	int j = 0;
	int checksum = 0;
	int data[100];
	data[0] = data[1] = data[2] = data[3] = data[4] = 0;
	uint8_t pin = pin_num[sensor->pin];

	// Wake up device, 250ms of high
	GPIO_OUTPUT_SET(pin, 1);
	sleepms(250);
	// Hold low for 20ms
	GPIO_OUTPUT_SET(pin, 0);
	sleepms(20);
	// High for 40ns
	GPIO_OUTPUT_SET(pin, 1);
	os_delay_us(40);
	// Set DHT_PIN pin as an input
	GPIO_DIS_OUTPUT(pin);

	// wait for pin to drop?
	while (GPIO_INPUT_GET(pin) == 1 && i < DHT_MAXCOUNT) {
		os_delay_us(1);
		i++;
	}

	if(i == DHT_MAXCOUNT)
	{
		DHT_DEBUG("DHT: Failed to get reading from GPIO%d, dying\r\n", pin);
	    return false;
	}

	// read data
	for (i = 0; i < DHT_MAXTIMINGS; i++)
	{
		// Count high time (in approx us)
		counter = 0;
		while (GPIO_INPUT_GET(pin) == laststate)
		{
			counter++;
			os_delay_us(1);
			if (counter == 1000)
				break;
		}
		laststate = GPIO_INPUT_GET(pin);
		if (counter == 1000)
			break;
		// store data after 3 reads
		if ((i>3) && (i%2 == 0)) {
			// shove each bit into the storage bytes
			data[j/8] <<= 1;
			if (counter > DHT_BREAKTIME)
				data[j/8] |= 1;
			j++;
		}
	}

	if (j >= 39) {
		checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
	    DHT_DEBUG("DHT%s: %02x %02x %02x %02x [%02x] CS: %02x (GPIO%d)\r\n",
	              sensor->type==DHT11?"11":"22",
	              data[0], data[1], data[2], data[3], data[4], checksum, pin);
		if (data[4] == checksum) {
			// checksum is valid
			output->temperature = scale_temperature(sensor->type, data);
			output->humidity = scale_humidity(sensor->type, data);
			//DHT_DEBUG("DHT: Temperature =  %d *C, Humidity = %d %%\r\n", (int)(reading.temperature * 100), (int)(reading.humidity * 100));
			DHT_DEBUG("DHT: Temperature*100 =  %d *C, Humidity*100 = %d %% (GPIO%d)\n",
		          (int) (output->temperature * 100), (int) (output->humidity * 100), pin);
		} else {
			//DHT_DEBUG("Checksum was incorrect after %d bits. Expected %d but got %d\r\n", j, data[4], checksum);
			DHT_DEBUG("DHT: Checksum was incorrect after %d bits. Expected %d but got %d (GPIO%d)\r\n",
		                j, data[4], checksum, pin);
		    return false;
		}
	} else {
		//DHT_DEBUG("Got too few bits: %d should be at least 40\r\n", j);
	    DHT_DEBUG("DHT: Got too few bits: %d should be at least 40 (GPIO%d)\r\n", j, pin);
	    return false;
	}
	return true;
}


bool DHTInit(DHT_Sensor *sensor)
{
	if (set_gpio_mode(sensor->pin, GPIO_PULLUP, GPIO_INPUT)) {
		DHT_DEBUG("DHT: Setup for type %s connected to GPIO%d\n", sensor->type==DHT11?"DHT11":"DHT22", pin_num[sensor->pin]);
		return true;
	} else {
		DHT_DEBUG("DHT: Error in function set_gpio_mode for type %s connected to GPIO%d\n", sensor->type==DHT11?"DHT11":"DHT22", pin_num[sensor->pin]);
		return false;
	}
}
