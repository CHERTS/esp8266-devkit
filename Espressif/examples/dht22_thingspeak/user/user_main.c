/*
 *  Example of working sensor DHT22 (temperature and humidity) and send data on the service thingspeak.com
 *  https://thingspeak.com
 *
 *  For a single device, connect as follows:
 *  DHT22 1 (Vcc) to Vcc (3.3 Volts)
 *  DHT22 2 (DATA_OUT) to ESP Pin GPIO2
 *  DHT22 3 (NC)
 *  DHT22 4 (GND) to GND
 *
 *  Between Vcc and DATA_OUT need to connect a pull-up resistor of 10 kOh.
 *
 *  (c) 2015 by Mikhail Grigorev <sleuthhound@gmail.com>
 *
 */

#include <user_interface.h>
#include <osapi.h>
#include <c_types.h>
#include <mem.h>
#include <os_type.h>
#include "httpclient.h"
#include "driver/uart.h"
#include "driver/dht22.h"
#include "user_config.h"

LOCAL os_timer_t dht22_timer;
LOCAL void ICACHE_FLASH_ATTR setup_wifi_st_mode(void);
static struct ip_info ipconfig;

const char *WiFiMode[] =
{
		"NULL",		// 0x00
		"STATION",	// 0x01
		"SOFTAP", 	// 0x02
		"STATIONAP"	// 0x03
};

const char *WiFiStatus[] =
{
	    "STATION_IDLE", 			// 0x00
	    "STATION_CONNECTING", 		// 0x01
	    "STATION_WRONG_PASSWORD", 	// 0x02
	    "STATION_NO_AP_FOUND", 		// 0x03
	    "STATION_CONNECT_FAIL", 	// 0x04
	    "STATION_GOT_IP" 			// 0x05
};

LOCAL void ICACHE_FLASH_ATTR thingspeak_http_callback(char * response, int http_status, char * full_response)
{
	os_printf("Answers: \r\n");
	if (http_status == 200)
	{
		//os_printf("strlen(response)=%d\r\n", strlen(response));
		//os_printf("strlen(full_response)=%d\r\n", strlen(full_response));
		os_printf("response=%s\r\n", response);
		//os_printf("full_response=%s\r\n", full_response);
		os_printf("---------------------------\r\n");
	}
	else
	{
		os_printf("http_status=%d\r\n", http_status);
		os_printf("strlen(response)=%d\r\n", strlen(response));
		os_printf("strlen(full_response)=%d\r\n", strlen(full_response));
		os_printf("response=%s\r\n", response);
		os_printf("---------------------------\r\n");
	}
}

LOCAL void ICACHE_FLASH_ATTR dht22_cb(void *arg)
{
	static char data[256];
	static char temp[10];
	static char hum[10];
	uint8_t status;
	os_timer_disarm(&dht22_timer);
	struct dht_sensor_data* r = DHTRead();
	float lastTemp = r->temperature;
	float lastHum = r->humidity;
	if(r->success)
	{
		status = wifi_station_get_connect_status();
		os_printf("ESP8266 wifi_station_get_connect_status returns %s\r\n", WiFiStatus[status]);
		if (status != STATION_GOT_IP)
		{
			setup_wifi_st_mode();
		}
		status = wifi_station_get_connect_status();
		os_printf("ESP8266 wifi_station_get_connect_status now %s\r\n", WiFiStatus[status]);
		if (status == STATION_GOT_IP)
		{
			wifi_get_ip_info(STATION_IF, &ipconfig);
			os_sprintf(temp, "%d.%d",(int)(lastTemp),(int)((lastTemp - (int)lastTemp)*100));
			os_sprintf(hum, "%d.%d",(int)(lastHum),(int)((lastHum - (int)lastHum)*100));
			os_printf("Temperature: %s *C, Humidity: %s %%\r\n", temp, hum);
			// Start the connection process
			os_sprintf(data, "http://%s/update?key=%s&field1=%s&field2=%s&status=dev_ip:%d.%d.%d.%d", THINGSPEAK_SERVER, THINGSPEAK_API_KEY, temp, hum, IP2STR(&ipconfig.ip));
			os_printf("Request: %s\r\n", data);
			http_get(data, thingspeak_http_callback);
			/*os_sprintf(data, "key=%s&field1=%s&field2=%s&status=dev_ip:%d.%d.%d.%d", THINGSPEAK_SERVER, THINGSPEAK_API_KEY, temp, hum, IP2STR(&ipconfig.ip));
			os_printf("Request: http://184.106.153.149/update?%s\r\n", data);
			http_post("http://184.106.153.149/update", data, thingspeak_http_callback);*/
		}
		else
		{
			os_printf("ESP8266 failed to connect to network! Will retry later.\r\n");
			return;
		}
	}
	else
	{
		os_printf("Error reading temperature and humidity.\r\n");
	}
	os_timer_setfn(&dht22_timer, (os_timer_func_t *)dht22_cb, (void *)0);
	os_timer_arm(&dht22_timer, DATA_SEND_DELAY, 1);
}

LOCAL void ICACHE_FLASH_ATTR setup_wifi_st_mode(void)
{
	wifi_set_opmode(STATION_MODE);
	struct station_config stconfig;
	wifi_station_disconnect();
	wifi_station_dhcpc_stop();
	if(wifi_station_get_config(&stconfig))
	{
		os_memset(stconfig.ssid, 0, sizeof(stconfig.ssid));
		os_memset(stconfig.password, 0, sizeof(stconfig.password));
		os_sprintf(stconfig.ssid, "%s", WIFI_CLIENTSSID);
		os_sprintf(stconfig.password, "%s", WIFI_CLIENTPASSWORD);
		if(!wifi_station_set_config(&stconfig))
		{
			os_printf("ESP8266 not set station config!\r\n");
		}
	}
	wifi_station_connect();
	wifi_station_dhcpc_start();
	wifi_station_set_auto_connect(1);
	os_printf("ESP8266 in STA mode configured.\r\n");
}

void user_init(void)
{
	// Configure the UART
	uart_init(BIT_RATE_115200,0);
	// Enable system messages
	system_set_os_print(1);
	os_printf("\r\nSystem init...\r\n");

	if(wifi_get_opmode() != STATION_MODE)
	{
		os_printf("ESP8266 is %s mode, restarting in %s mode...\r\n", WiFiMode[wifi_get_opmode()], WiFiMode[STATION_MODE]);
		setup_wifi_st_mode();
	}
	if(wifi_get_phy_mode() != PHY_MODE_11N)
		wifi_set_phy_mode(PHY_MODE_11N);
	if(wifi_station_get_auto_connect() == 0)
		wifi_station_set_auto_connect(1);

	// Init DHT22 sensor
	DHTInit(DHT22);

	// Set up a timer to send the message
	os_timer_disarm(&dht22_timer);
	os_timer_setfn(&dht22_timer, (os_timer_func_t *)dht22_cb, (void *)0);
	os_timer_arm(&dht22_timer, DATA_SEND_DELAY, 1);

	os_printf("System init done.\n");
}
