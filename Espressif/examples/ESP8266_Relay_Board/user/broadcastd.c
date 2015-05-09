#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "io.h"
#include "broadcastd.h"
#include "dht22.h"
#include "ds18b20.h"
#include "config.h"
#include "mqtt.h"
#include "utils.h"
#include "httpclient.h"
  
MQTT_Client mqttClient;

/*
 * ----------------------------------------------------------------------------
 * "THE MODIFIED BEER-WARE LICENSE" (Revision 42):
 * Mathew Hall wrote this file. As long as you
 * retain
 * this notice you can do whatever you want with this stuff. If we meet some
 * day,
 * and you think this stuff is worth it, you can buy sprite_tm a beer in return.
 * ----------------------------------------------------------------------------
 */

static ETSTimer MQTTbroadcastTimer;
static ETSTimer broadcastTimer; 
 
static void ICACHE_FLASH_ATTR broadcastReading(void *arg) {

    char buf[384];
	char buf2[255];
	char t1[32];
	char t2[32];
	char t3[32];
	
	//double expand as sysCfg.broadcastd_url cntains placeholders as well
	os_sprintf(buf2,"http://%s:%d/%s",sysCfg.broadcastd_host,(int)sysCfg.broadcastd_port,sysCfg.broadcastd_url);
	
	if(sysCfg.sensor_dht22_enable)  {
		dht_temp_str(t2);
		dht_humi_str(t3);
		os_sprintf(buf,buf2,currGPIO12State,currGPIO13State,currGPIO15State,"N/A",t2,t3);
	}
	
	if(sysCfg.sensor_ds18b20_enable)  { // If DS18b20 daemon is enabled, then send up to 3 sensor's data instead
		ds_str(t1,0);
		if(numds>1) ds_str(t2,1); //reuse to save space
		if(numds>2)  ds_str(t3,2); //reuse to save space
		os_sprintf(buf,buf2,currGPIO12State,currGPIO13State,currGPIO15State,t1,t2,t3);
	}
		
	http_get(buf, http_callback_example);	
	os_printf("Sent HTTP GET: %s\n\r",buf);
}
 

static ICACHE_FLASH_ATTR void MQTTbroadcastReading(void* arg){
	if(sysCfg.mqtt_enable==1) {
		//os_printf("Sending MQTT\n");
		
		if(sysCfg.sensor_dht22_enable) {
			struct sensor_reading* result = readDHT();
			if(result->success) {
				char temp[32];
				char topic[128];
				int len;
				
				dht_temp_str(temp);
				len = os_strlen(temp);
				os_sprintf(topic,"%s",sysCfg.mqtt_dht22_temp_pub_topic);
				MQTT_Publish(&mqttClient,topic,temp,len,0,0);
				os_printf("Published \"%s\" to topic \"%s\"\n",temp,topic);
				
				dht_humi_str(temp);
				len = os_strlen(temp);
				os_sprintf(topic,"%s",sysCfg.mqtt_dht22_humi_pub_topic);
				MQTT_Publish(&mqttClient,topic,temp,len,0,0);
				os_printf("Published \"%s\" to topic \"%s\"\n",temp,topic);
			}
		}
		
		if(sysCfg.sensor_ds18b20_enable) {
			struct sensor_reading* result = read_ds18b20();
			if(result->success) {
				char temp[32];
				char topic[128];
				int len;
				ds_str(temp,0);
				len = os_strlen(temp);
				os_sprintf(topic,"%s",sysCfg.mqtt_ds18b20_temp_pub_topic);
				MQTT_Publish(&mqttClient,topic,temp,len,0,0);
				os_printf("Published \"%s\" to topic \"%s\"\n",temp,topic);
			}
		}
    }
}


void ICACHE_FLASH_ATTR broadcastd_init(void){

	if(sysCfg.mqtt_enable==1) {
		os_printf("Arming MQTT broadcast timer\n");
		os_timer_setfn(&MQTTbroadcastTimer, MQTTbroadcastReading, NULL);
		os_timer_arm(&MQTTbroadcastTimer, 60000, 1);
	}
	
	if(sysCfg.broadcastd_enable==1) {
		os_printf("Arming HTTP broadcast timer\n");  	
		os_timer_setfn(&broadcastTimer, broadcastReading, NULL);
		os_timer_arm(&broadcastTimer, 60000, 1);		
	}
}
