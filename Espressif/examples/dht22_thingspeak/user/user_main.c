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

typedef enum {
	WIFI_CONNECTING,
	WIFI_CONNECTING_ERROR,
	WIFI_CONNECTED,
} tConnState;

unsigned char *default_certificate;
unsigned int default_certificate_len = 0;
unsigned char *default_private_key;
unsigned int default_private_key_len = 0;

extern int ets_uart_printf(const char *fmt, ...);
int (*console_printf)(const char *fmt, ...) = ets_uart_printf;

// Debug output.
#ifdef DHT22_DEBUG
#undef DHT22_DEBUG
#define DHT22_DEBUG(...) console_printf(__VA_ARGS__);
#else
#define DHT22_DEBUG(...)
#endif

LOCAL os_timer_t dht22_timer;
LOCAL void ICACHE_FLASH_ATTR setup_wifi_st_mode(void);
static struct ip_info ipConfig;
static ETSTimer WiFiLinker;
static tConnState connState = WIFI_CONNECTING;

const char *FlashSizeMap[] =
{
		"512 KB (256 KB + 256 KB)",	// 0x00
		"256 KB",			// 0x01
		"1024 KB (512 KB + 512 KB)", 	// 0x02
		"2048 KB (512 KB + 512 KB)"	// 0x03
		"4096 KB (512 KB + 512 KB)"	// 0x04
		"2048 KB (1024 KB + 1024 KB)"	// 0x05
		"4096 KB (1024 KB + 1024 KB)"	// 0x06
};

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
	DHT22_DEBUG("Answers: \r\n");
	if (http_status == 200)
	{
		//DHT22_DEBUG("strlen(response)=%d\r\n", strlen(response));
		//DHT22_DEBUG("strlen(full_response)=%d\r\n", strlen(full_response));
		DHT22_DEBUG("response=%s<EOF>\n", response);
		//DHT22_DEBUG("full_response=%s\r\n", full_response);
		DHT22_DEBUG("---------------------------\r\n");
	}
	else
	{
		DHT22_DEBUG("http_status=%d\r\n", http_status);
		DHT22_DEBUG("strlen(response)=%d\r\n", strlen(response));
		DHT22_DEBUG("strlen(full_response)=%d\r\n", strlen(full_response));
		DHT22_DEBUG("response=%s<EOF>\n", response);
		DHT22_DEBUG("---------------------------\r\n");
	}
	DHT22_DEBUG("Free heap size = %d\r\n", system_get_free_heap_size());
}

LOCAL void ICACHE_FLASH_ATTR dht22_cb(void *arg)
{
	static char data[256];
	static char temp[10];
	static char hum[10];
	struct dht_sensor_data* r;
	float lastTemp, lastHum;

	os_timer_disarm(&dht22_timer);
	switch(connState)
	{
		case WIFI_CONNECTED:
			r = DHTRead();
			lastTemp = r->temperature;
			lastHum = r->humidity;
			if(r->success)
			{
					wifi_get_ip_info(STATION_IF, &ipConfig);
					os_sprintf(temp, "%d.%d",(int)(lastTemp),(int)((lastTemp - (int)lastTemp)*100));
					os_sprintf(hum, "%d.%d",(int)(lastHum),(int)((lastHum - (int)lastHum)*100));
					DHT22_DEBUG("Temperature: %s *C, Humidity: %s %%\r\n", temp, hum);
					// Start the connection process
					os_sprintf(data, "http://%s/update?key=%s&field1=%s&field2=%s&status=dev_ip:%d.%d.%d.%d", THINGSPEAK_SERVER, THINGSPEAK_API_KEY, temp, hum, IP2STR(&ipConfig.ip));
					DHT22_DEBUG("Request: %s\r\n", data);
					http_get(data, "", thingspeak_http_callback);
					/*os_sprintf(data, "key=%s&field1=%s&field2=%s&status=dev_ip:%d.%d.%d.%d", THINGSPEAK_SERVER, THINGSPEAK_API_KEY, temp, hum, IP2STR(&ipconfig.ip));
					DHT22_DEBUG("Request: http://184.106.153.149/update?%s\r\n", data);
					http_post("http://184.106.153.149/update", data, "Content-Type: application/x-www-form-urlencoded\r\n", thingspeak_http_callback);*/
			} else {
				DHT22_DEBUG("Error reading temperature and humidity.\r\n");
			}
			break;
		default:
			DHT22_DEBUG("WiFi not connected...\r\n");
	}
	os_timer_setfn(&dht22_timer, (os_timer_func_t *)dht22_cb, (void *)0);
	os_timer_arm(&dht22_timer, DATA_SEND_DELAY, 1);
}

static void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{
	os_timer_disarm(&WiFiLinker);
	switch(wifi_station_get_connect_status())
	{
		case STATION_GOT_IP:
			wifi_get_ip_info(STATION_IF, &ipConfig);
			if(ipConfig.ip.addr != 0) {
				connState = WIFI_CONNECTED;
				DHT22_DEBUG("WiFi connected, wait DHT22 timer...\r\n");
			} else {
				connState = WIFI_CONNECTING_ERROR;
				DHT22_DEBUG("WiFi connected, ip.addr is null\r\n");
			}
			break;
		case STATION_WRONG_PASSWORD:
			connState = WIFI_CONNECTING_ERROR;
			DHT22_DEBUG("WiFi connecting error, wrong password\r\n");
			break;
		case STATION_NO_AP_FOUND:
			connState = WIFI_CONNECTING_ERROR;
			DHT22_DEBUG("WiFi connecting error, ap not found\r\n");
			break;
		case STATION_CONNECT_FAIL:
			connState = WIFI_CONNECTING_ERROR;
			DHT22_DEBUG("WiFi connecting fail\r\n");
			break;
		default:
			connState = WIFI_CONNECTING;
			DHT22_DEBUG("WiFi connecting...\r\n");
	}
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, WIFI_CHECK_DELAY, 0);
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
			DHT22_DEBUG("ESP8266 not set station config!\r\n");
		}
	}
	wifi_station_connect();
	wifi_station_dhcpc_start();
	wifi_station_set_auto_connect(1);
	DHT22_DEBUG("ESP8266 in STA mode configured.\r\n");
}

void user_rf_pre_init(void)
{
}

void user_init(void)
{
	// Configure the UART
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	// Enable system messages
	system_set_os_print(1);
	DHT22_DEBUG("\n==== System info: ====\n");
	DHT22_DEBUG("SDK version:%s rom %d\n", system_get_sdk_version(), system_upgrade_userbin_check());
	DHT22_DEBUG("Time = %ld\n", system_get_time());
	DHT22_DEBUG("Chip id = 0x%x\n", system_get_chip_id());
	DHT22_DEBUG("CPU freq = %d MHz\n", system_get_cpu_freq());
	DHT22_DEBUG("Flash size map = %s\n", FlashSizeMap[system_get_flash_size_map()]);
	DHT22_DEBUG("Free heap size = %d\n", system_get_free_heap_size());
	DHT22_DEBUG("==== End System info ====\n");
	os_delay_us(10000);
	DHT22_DEBUG("System init...\r\n");

	if(wifi_get_opmode() != STATION_MODE)
	{
		DHT22_DEBUG("ESP8266 is %s mode, restarting in %s mode...\r\n", WiFiMode[wifi_get_opmode()], WiFiMode[STATION_MODE]);
		setup_wifi_st_mode();
	}
	if(wifi_get_phy_mode() != PHY_MODE_11N)
		wifi_set_phy_mode(PHY_MODE_11N);
	if(wifi_station_get_auto_connect() == 0)
		wifi_station_set_auto_connect(1);

	// Init DHT22 sensor
	DHTInit(DHT22);

	// Wait for Wi-Fi connection
	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, WIFI_CHECK_DELAY, 0);

	// Set up a timer to send the message
	os_timer_disarm(&dht22_timer);
	os_timer_setfn(&dht22_timer, (os_timer_func_t *)dht22_cb, (void *)0);
	os_timer_arm(&dht22_timer, DATA_SEND_DELAY, 1);

	DHT22_DEBUG("System init done.\n");
}
