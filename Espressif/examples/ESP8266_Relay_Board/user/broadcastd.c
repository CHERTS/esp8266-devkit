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

static struct espconn tempConn;
static struct _esp_tcp socket;
  
static ip_addr_t master_addr;

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
static ETSTimer lookupTimer;

static void ICACHE_FLASH_ATTR networkSentCb(void *arg) {
    //os_printf("sent callback\n");
    struct espconn *pespconn = (struct espconn *)arg;
    espconn_disconnect(pespconn);
}
 
static void ICACHE_FLASH_ATTR networkRecvCb(void *arg, char *data, unsigned short len) {

  //os_printf(data,len);
}
 
static void ICACHE_FLASH_ATTR networkConnectedCb(void *arg) {

    struct espconn* conn = arg;
    char buf[384];
	char buf2[255];
	char dstemp[32];
	char temp[32];
	char humi[32];

	ds_str(dstemp,0);
	dht_temp_str(temp);
	dht_humi_str(humi);
	
	//double expand as sysCfg.broadcastd_url cntains placeholders as well
	os_sprintf(buf2,"GET %s HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\nUser-Agent: Mozilla/4.0 (compatible; esp8266; Windows NT 5.1)\r\n\r\n",sysCfg.broadcastd_url,sysCfg.broadcastd_host);
	os_sprintf(buf,buf2,currGPIO12State,currGPIO13State,currGPIO15State,dstemp,temp,humi);

    espconn_sent(conn, (uint8*)buf, os_strlen(buf));

	os_printf("Sent HTTP GET: %s\n\r",buf);
}
 
static void ICACHE_FLASH_ATTR networkReconCb(void *arg, sint8 err) {
  //os_printf("rcon\n");
}
 
static void ICACHE_FLASH_ATTR networkDisconCb(void *arg) {
    struct espconn *pespconn = (struct espconn *)arg;
    os_free(pespconn->proto.tcp);
    os_free(pespconn);
    //os_printf("disconnect callback\n");
}


static void ICACHE_FLASH_ATTR dnsLookupCb(const char *name, ip_addr_t *ipaddr, void *arg){
    struct espconn* conn = arg;
    
    if(ipaddr == NULL){
        os_printf("Logger: couldn't resolve IP address for %s;\n", name);
        return;
    }
    
    os_printf("Successfully resolved %s as %d.%d.%d.%d\n", name,
			*((uint8 *) &ipaddr->addr),
			*((uint8 *) &ipaddr->addr + 1),
			*((uint8 *) &ipaddr->addr + 2),
			*((uint8 *) &ipaddr->addr + 3));
    
    if(master_addr.addr == 0 && ipaddr->addr != 0){
        master_addr.addr = ipaddr->addr;
        os_memcpy(conn->proto.tcp->remote_ip, &ipaddr->addr, 4);
        os_printf("Will send to %d.%d.%d.%d\n", (int)conn->proto.tcp->remote_ip[0], (int)conn->proto.tcp->remote_ip[1], (int)conn->proto.tcp->remote_ip[2], (int)conn->proto.tcp->remote_ip[3]);
        conn->proto.tcp->local_port = espconn_port();
    }
    
    //espconn_create(conn);
	espconn_regist_connectcb(conn, networkConnectedCb);
	espconn_regist_disconcb(conn, networkDisconCb);
	espconn_regist_reconcb(conn, networkReconCb);
	espconn_regist_recvcb(conn, networkRecvCb);
	espconn_regist_sentcb(conn, networkSentCb);
    os_timer_arm(&broadcastTimer, 60000, 1);
}

static void ICACHE_FLASH_ATTR lookupTask(void* arg){
    os_sprintf("Attempting to resolve %s\n", (char*)sysCfg.broadcastd_host);
    espconn_gethostbyname(&tempConn, (char*)sysCfg.broadcastd_host, &master_addr, dnsLookupCb);
    os_timer_disarm(&lookupTimer);
}

static void ICACHE_FLASH_ATTR broadcastReading(void* arg){
	espconn_connect(&tempConn);
}  

static ICACHE_FLASH_ATTR void MQTTbroadcastReading(void* arg){

	if(sysCfg.mqtt_enable==1) {
		//os_printf("Sending MQTT\n");
		
		if(sysCfg.sensor_dht22_enable) {
			struct sensor_reading* result = readDHT();
			if(result->success) {
				char temp[32];
				char topic[128];
				int len = dht_temp_str(temp);
				os_sprintf(topic,"%s",sysCfg.mqtt_dht22_temp_pub_topic);
				MQTT_Publish(&mqttClient,topic,temp,len,0,0);
				os_printf("Published \"%s\" to topic \"%s\"\n",temp,topic);
				
				len = dht_humi_str(temp);
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
				int len = ds_str(temp,0);
				os_sprintf(topic,"%s",sysCfg.mqtt_ds18b20_temp_pub_topic);
				MQTT_Publish(&mqttClient,topic,temp,len,0,0);
				os_printf("Published \"%s\" to topic \"%s\"\n",temp,topic);
			}
		}

	
    }
}


void ICACHE_FLASH_ATTR broadcastd_init(void){

    os_timer_setfn(&MQTTbroadcastTimer, MQTTbroadcastReading, NULL);
	os_printf("Arming MQTT broadcast timer\n");
	os_timer_arm(&MQTTbroadcastTimer, 60000, 1);

	if(sysCfg.broadcastd_enable==1) {

		os_printf("HTTP logging initialising\n");  
		tempConn.type=ESPCONN_TCP;
		tempConn.state=ESPCONN_NONE;
		tempConn.proto.tcp=&socket;
  
		master_addr.addr = 0;
		socket.remote_port=sysCfg.broadcastd_port;
		os_timer_setfn(&lookupTimer, lookupTask, NULL);
		os_timer_arm(&lookupTimer, 10000, 1);
	
		os_timer_setfn(&broadcastTimer, broadcastReading, NULL);
	}
}