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

enum DHTType sensor_type;
#define sleepms(x) os_delay_us(x*1000);
static ETSTimer dhtTimer;

static inline float scale_humidity(int *data) {
	if(sensor_type == DHT11) {
		return data[0];
	} else {
		float humidity = data[0] * 256 + data[1];
		return humidity /= 10;
	}
}

static inline float scale_temperature(int *data) {
	if(sensor_type == DHT11) {
		return data[2];
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

static struct dht_sensor_data reading = {
	.success = 0
};

struct dht_sensor_data *ICACHE_FLASH_ATTR DHTRead(void) {
	return &reading;
}

struct dht_sensor_data *ICACHE_FLASH_ATTR DHTCb(void)
{
        int counter = 0;
	int laststate = 1;
	int i = 0;
        int j = 0;
        int checksum = 0;
        int data[100];
        data[0] = data[1] = data[2] = data[3] = data[4] = 0;

        // Wake up device, 250ms of high
        GPIO_OUTPUT_SET(DHT_PIN, 1);
        sleepms(250);
        // Hold low for 20ms
        GPIO_OUTPUT_SET(DHT_PIN, 0);
        sleepms(20);
        // High for 40ns
        GPIO_OUTPUT_SET(DHT_PIN, 1);
        os_delay_us(40);
        // Set DHT_PIN pin as an input
        GPIO_DIS_OUTPUT(DHT_PIN);

        // wait for pin to drop?
        while (GPIO_INPUT_GET(DHT_PIN) == 1 && i < DHT_MAXCOUNT) {
                os_delay_us(1);
                i++;
        }

        if(i == DHT_MAXCOUNT) {
                reading.success = 0;
                os_printf("Failed to get reading, dying\r\n");
                return;
        }

        // read data
        for (i = 0; i < DHT_MAXTIMINGS; i++)
        {
                // Count high time (in approx us)
                counter = 0;
                while (GPIO_INPUT_GET(DHT_PIN) == laststate)
                {
                        counter++;
                        os_delay_us(1);
                        if (counter == 1000)
                                break;
                }
                laststate = GPIO_INPUT_GET(DHT_PIN);
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
                os_printf("DHT: %02x %02x %02x %02x [%02x] CS: %02x", data[0], data[1],data[2],data[3],data[4],checksum);
                if (data[4] == checksum) {
                        // checksum is valid
                        reading.temperature = scale_temperature(data);
                        reading.humidity = scale_humidity(data);
                        os_printf("Temperature =  %d *C, Humidity = %d %%\r\n", (int)(reading.temperature * 100), (int)(reading.humidity * 100));
                        reading.success = 1;
                } else {
                        os_printf("Checksum was incorrect after %d bits. Expected %d but got %d\r\n", j, data[4], checksum);
                        reading.success = 0;
                }
        } else {
                os_printf("Got too few bits: %d should be at least 40\r\n", j);
                reading.success = 0;
        }
        return;
}

void ICACHE_FLASH_ATTR DHTInit(enum DHTType dht_type, uint32_t poll_time)
{
	sensor_type = dht_type;
	PIN_FUNC_SELECT(DHT_MUX, DHT_FUNC);
	PIN_PULLUP_EN(DHT_MUX);
	os_printf("DHT setup for type %d\r\n", dht_type);
	os_timer_disarm(&dhtTimer);
	os_timer_setfn(&dhtTimer, DHTCb, NULL);
	os_timer_arm(&dhtTimer, poll_time, 1);
}

void ICACHE_FLASH_ATTR DHTStop()
{
	os_timer_disarm(&dhtTimer);
}
