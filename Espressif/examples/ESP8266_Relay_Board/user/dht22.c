
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------

 Dec 21 2014: Adopt Mathew Hall's approach https://github.com/mathew-hall/esp8266-dht

 */

#include "ets_sys.h"
#include "osapi.h"
#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "gpio.h"
#include <stdlib.h>

#include "dht22.h"

#define MAXTIMINGS 10000
#define DHT_MAXCOUNT 32000
#define BREAKTIME 32
  
#define DHT_PIN 2

enum sensor_type SENSOR;

static inline float ICACHE_FLASH_ATTR scale_humidity(int *data) {
  if (SENSOR == SENSOR_DHT11) {
    return data[0];
  } else {
    float humidity = data[0] * 256 + data[1];
    return humidity /= 10;
  }
}

static inline float ICACHE_FLASH_ATTR scale_temperature(int *data) {
  if (SENSOR == SENSOR_DHT11) {
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

static inline void ICACHE_FLASH_ATTR delay_ms(int sleep) { 
    os_delay_us(1000 * sleep); 
}

static struct sensor_reading reading = {
    .source = "DHT22", .success = 0
};


/** 
Adapted from: https://github.com/adafruit/Adafruit_Python_DHT/blob/master/source/Raspberry_Pi/pi_dht_read.c
LICENSE:
// Copyright (c) 2014 Adafruit Industries
// Author: Tony DiCola
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
*/

struct sensor_reading * ICACHE_FLASH_ATTR readDHT(void) { 
    return &reading;
}
    
static  void ICACHE_FLASH_ATTR pollDHTCb(void * arg){

  int counter = 0;
  int laststate = 1;
  int i = 0;
  int bits_in = 0;
  // int bitidx = 0;
  // int bits[250];

  int data[100];

  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // Wake up device, 250ms of high
  GPIO_OUTPUT_SET(DHT_PIN, 1);
  delay_ms(500);

  // Hold low for 20ms
  GPIO_OUTPUT_SET(DHT_PIN, 0);
  delay_ms(20);

  // High for 40ms
  // GPIO_OUTPUT_SET(2, 1);

  GPIO_DIS_OUTPUT(DHT_PIN);
  os_delay_us(40);

  // Set pin to input with pullup
  // PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);

  // os_printf("Waiting for gpio2 to drop \n");

  // wait for pin to drop?
  while (GPIO_INPUT_GET(DHT_PIN) == 1 && i < DHT_MAXCOUNT) {
    if (i >= DHT_MAXCOUNT) {
      goto fail;
    }
    i++;
  }

//  os_printf("Reading DHT\n");

  // read data!
  for (i = 0; i < MAXTIMINGS; i++) {
    // Count high time (in approx us)
    counter = 0;
    while (GPIO_INPUT_GET(DHT_PIN) == laststate) {
      counter++;
      os_delay_us(1);
      if (counter == 1000)
        break;
    }
    laststate = GPIO_INPUT_GET(DHT_PIN);

    if (counter == 1000)
      break;

    // store data after 3 reads
    if ((i > 3) && (i % 2 == 0)) {
      // shove each bit into the storage bytes
      data[bits_in / 8] <<= 1;
      if (counter > BREAKTIME) {
        //os_printf("1");
        data[bits_in / 8] |= 1;
      } else {
        //os_printf("0");
      }
      bits_in++;
    }
  }

  if (bits_in < 40) {
    os_printf("Got too few bits: %d should be at least 40", bits_in);
    goto fail;
  }
  

  int checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
  
  //os_printf("DHT: %02x %02x %02x %02x [%02x] CS: %02x\n", data[0], data[1],data[2],data[3],data[4],checksum);
  
  if (data[4] != checksum) {
    os_printf("Checksum was incorrect after %d bits. Expected %d but got %d",
              bits_in, data[4], checksum);
    goto fail;
  }

  reading.temperature = scale_temperature(data);
  reading.humidity = scale_humidity(data);
  //os_printf("Temp =  %d*C, Hum = %d%%\n", (int)(reading.temperature * 100), (int)(reading.humidity * 100));
  
  reading.success = 1;
  return;
fail:
  
  os_printf("Failed to get DHT reading, dying\n");
  reading.success = 0;
}


int ICACHE_FLASH_ATTR dht_temp_str(char *buff) {
	struct sensor_reading* result = readDHT();

	if(result->success)
		return os_sprintf(buff, "%d.%d", (int)(result->temperature),  abs((int)(result->temperature*100)) - abs(((int)result->temperature)*100)    );
	else
		return os_sprintf(buff, "N/A" );		
}

int ICACHE_FLASH_ATTR dht_humi_str(char *buff) {

	struct sensor_reading* result = readDHT();

	if(result->success)
		return os_sprintf(buff, "%d.%d", (int)(result->humidity),(int)((result->humidity - (int)result->humidity)*100));
	else
		return os_sprintf(buff, "N/A" );
}


void ICACHE_FLASH_ATTR DHTInit(enum sensor_type sensor_type, uint32_t polltime) {
  SENSOR = sensor_type;
  // Set GPIO2 to output mode for DHT22
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
  PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
  
  pollDHTCb(NULL);
  
  os_printf("DHT Setup for type %d, poll interval of %d\n", sensor_type, (int)polltime);
  
  static ETSTimer dhtTimer;
  os_timer_setfn(&dhtTimer, pollDHTCb, NULL);
  os_timer_arm(&dhtTimer, polltime, 1);
}