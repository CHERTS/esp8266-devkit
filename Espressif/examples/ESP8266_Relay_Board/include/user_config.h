#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#define ICACHE_STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define ICACHE_STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_RAM_ATTR __attribute__((section(".iram0.text")))
#define ICACHE_RODATA_ATTR __attribute__((section(".irom.text")))

#define CFG_HOLDER	0x00FF55A2
#define CFG_LOCATION	0x3C

#define FWVER "0.65/March 15 2015"

/*DEFAULT CONFIGURATIONS*/

#define STA_MODE     "static"
#define STA_IP       "192.168.1.17"
#define STA_MASK     "255.255.255.0"
#define STA_GW       "192.168.1.1"
#define STA_SSID     "" 
#define STA_PASS     ""
#define STA_TYPE AUTH_WPA2_PSK

#define AP_IP        "192.168.4.1"
#define AP_MASK      "255.255.255.0"
#define AP_GW        "192.168.4.1"

#define HTTPD_PORT      80
#define HTTPD_AUTH      0
#define HTTPD_USER      "admin"
#define HTTPD_PASS      "pass"

#define BROADCASTD_ENABLE	0
#define BROADCASTD_PORT     80
#define BROADCASTD_HOST		"api.thingspeak.com"
#define BROADCASTD_URL		"/update?key=**RWAPI**&field1=%d&field2=%d&field3=%d&field4=%s&field5=%s&field6=%s"		
#define BROADCASTD_THINGSPEAK_CHANNEL 0
#define BROADCASTD_RO_APIKEY "**ROAPI**"

#define NTP_ENABLE    1
#define NTP_TZ  	  2

#define MQTT_ENABLE			0
#define MQTT_HOST			"19.168.1.1=6" //host name or IP "192.168.11.1"
#define MQTT_PORT			1883
#define MQTT_KEEPALIVE		120	 /*seconds*/
#define MQTT_DEVID			"ESP_%08X"
#define MQTT_USER			""
#define MQTT_PASS			""
#define MQTT_USE_SSL		0
#define MQTT_RELAY_SUBS_TOPIC 		"esp_%08X/out/relay/#"
#define MQTT_DHT22_TEMP_PUB_TOPIC  	"esp_%08X/in/dht22/temperature" 
#define MQTT_DHT22_HUMI_PUB_TOPIC   "esp_%08X/in/dht22/humidity" 		
#define MQTT_DS18B20_TEMP_PUB_TOPIC "esp_%08X/in/ds18b20/temperature"

#define SENSOR_DS18B20_ENABLE     0
#define SENSOR_DHT22_ENABLE       0

#define RELAY_LATCHING_ENABLE       0
#define RELAY1NAME       "Relay 1"
#define RELAY2NAME       "Relay 2"
#define RELAY3NAME       "Relay 3"
	
#define MQTT_BUF_SIZE		255
#define MQTT_RECONNECT_TIMEOUT 	5	/*second*/
#define MQTT_CONNTECT_TIMER 	5 	/**/

#endif

