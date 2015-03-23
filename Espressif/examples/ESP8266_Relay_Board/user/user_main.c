

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include "espmissingincludes.h"
#include "ets_sys.h"
//#include "lwip/timers.h"
#include "user_interface.h"

#include "httpd.h"
#include "io.h"
#include "httpdespfs.h"
#include "cgi.h"
#include "cgiwifi.h"
#include "cgithermostat.h"
#include "stdout.h"
#include "auth.h"
#include "sntp.h"
#include "time_utils.h"

#include "config.h"

#include "dht22.h"
#include "ds18b20.h"
#include "broadcastd.h"
#include "thermostat.h"
#include "wifi.h"
#include "mqtt.h"
//#include "netbios.h"


//#include "pwm.h"
//#include "cgipwm.h"

//#include "oled.h"

MQTT_Client mqttClient;

//Function that tells the authentication system what users/passwords live on the system.
//This is disabled in the default build; if you want to try it, enable the authBasic line in
//the builtInUrls below.
int ICACHE_FLASH_ATTR myPassFn(HttpdConnData *connData, int no, char *user, int userLen, char *pass, int passLen) {
	if (no==0) {
		os_strcpy(user, (char *)sysCfg.httpd_user);
		os_strcpy(pass, (char *)sysCfg.httpd_pass);
		return 1;
//Add more users this way. Check against incrementing no for each user added.
//	} else if (no==1) {
//		os_strcpy(user, "user1");
//		os_strcpy(pass, "something");
//		return 1;
	}
	return 0;
}


/*
This is the main url->function dispatching data struct.
In short, it's a struct with various URLs plus their handlers. The handlers can
be 'standard' CGI functions you wrote, or 'special' CGIs requiring an argument.
They can also be auth-functions. An asterisk will match any url starting with
everything before the asterisks; "*" matches everything. The list will be
handled top-down, so make sure to put more specific rules above the more
general ones. Authorization things (like authBasic) act as a 'barrier' and
should be placed above the URLs they protect.
*/
HttpdBuiltInUrl builtInUrls[]={
	{"/", cgiRedirect, "/index.tpl"},
	{"/index.tpl", cgiEspFsTemplate, tplCounter},
	{"/about.tpl", cgiEspFsTemplate, tplCounter},

	//{"/flash.bin", cgiReadFlash, NULL},

	{"/config/*", authBasic, myPassFn},
	{"/control/*", authBasic, myPassFn},

	{"/control/ui.tpl", cgiEspFsTemplate, tplUI},
	{"/control/relay.tpl", cgiEspFsTemplate, tplGPIO},
	{"/control/relay.cgi", cgiGPIO, NULL},
    {"/control/dht22.tpl", cgiEspFsTemplate, tplDHT},
    {"/control/dht22.cgi", cgiDHT22, NULL}, 
    {"/control/ds18b20.tpl", cgiEspFsTemplate, tplDS18b20},
    {"/control/ds18b20.cgi", cgiDS18b20, NULL}, 
    {"/control/state.cgi", cgiState, NULL}, 
    {"/control/reset.cgi", cgiReset, NULL}, 
    {"/control/thermostat.tpl", cgiEspFsTemplate, tplThermostat},
    {"/control/thermostat.cgi", cgiThermostat, NULL}, 
#ifdef CGIPWM_H
	{"/control/pwm.cgi", cgiPWM, NULL},
#endif
	{"/config/wifi", cgiRedirect, "/config/wifi/wifi.tpl"},
	{"/config/wifi/", cgiRedirect, "/config/wifi/wifi.tpl"},
	{"/config/wifi/wifiscan.cgi", cgiWiFiScan, NULL},
	{"/config/wifi/wifi.tpl", cgiEspFsTemplate, tplWlan},
	{"/config/wifi/connect.cgi", cgiWiFiConnect, NULL},
	{"/config/wifi/setmode.cgi", cgiWifiSetMode, NULL},
	{"/config/mqtt.tpl", cgiEspFsTemplate, tplMQTT},
	{"/config/mqtt.cgi", cgiMQTT, NULL},
	{"/config/httpd.tpl", cgiEspFsTemplate, tplHTTPD},
	{"/config/httpd.cgi", cgiHTTPD, NULL},
	{"/config/broadcastd.tpl", cgiEspFsTemplate, tplBroadcastD},
	{"/config/broadcastd.cgi", cgiBroadcastD, NULL},
	{"/config/ntp.tpl", cgiEspFsTemplate, tplNTP},
	{"/config/ntp.cgi", cgiNTP, NULL},
	{"/config/relay.tpl", cgiEspFsTemplate, tplRLYSettings},
	{"/config/relay.cgi", cgiRLYSettings, NULL},
	{"/config/sensor.tpl", cgiEspFsTemplate, tplSensorSettings},
	{"/config/sensor.cgi", cgiSensorSettings, NULL},

	
	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL}
};



void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status)
{
	if(status == STATION_GOT_IP){
		if(sysCfg.mqtt_enable==1) {
			MQTT_Connect(&mqttClient);
		} else {
			MQTT_Disconnect(&mqttClient);
		}
	}	
}

void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	os_printf("MQTT: Connected\r\n");
	MQTT_Subscribe(client, (char *)sysCfg.mqtt_relay_subs_topic,0);
}

void ICACHE_FLASH_ATTR mqttDisconnectedCb(uint32_t *args)
{
//	MQTT_Client* client = (MQTT_Client*)args;
	os_printf("MQTT: Disconnected\r\n");
}
void ICACHE_FLASH_ATTR mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t lengh)
{

	char strTopic[topic_len + 1];
	os_memcpy(strTopic, topic, topic_len);
	strTopic[topic_len] = '\0';

	char strData[lengh + 1];
	os_memcpy(strData, data, lengh);
	strData[lengh] = '\0';

	char relayNum=strTopic[topic_len-1];
	char strSubsTopic[strlen((char *)sysCfg.mqtt_relay_subs_topic)];
	os_strcpy(strSubsTopic,(char *)sysCfg.mqtt_relay_subs_topic);
	strSubsTopic[(strlen((char *)sysCfg.mqtt_relay_subs_topic)-1)]=relayNum;

	os_printf("MQTT strSubsTopic: %s, strTopic: %s \r\n", strSubsTopic, strTopic);

	if (os_strcmp(strSubsTopic,strTopic) == 0 ) {
		os_printf("Relay %d is now: %s \r\n", relayNum-'0', strData);
		
		if(relayNum=='1') {
			currGPIO12State=atoi(strData);
			ioGPIO(currGPIO12State,12);
		}

		if(relayNum=='2') {
			currGPIO13State=atoi(strData);
			ioGPIO(currGPIO13State,13);
		}

		if(relayNum=='3') {
			currGPIO15State=atoi(strData);
			ioGPIO(currGPIO15State,15);
		}
		
		if( sysCfg.relay_latching_enable) {		
			sysCfg.relay_1_state=currGPIO12State;					
			sysCfg.relay_2_state=currGPIO13State;
			sysCfg.relay_3_state=currGPIO15State;
			CFG_Save();
		}	
	}
	os_printf("MQTT topic: %s, data: %s \r\n", strTopic, strData);
}

void ICACHE_FLASH_ATTR mqttPublishedCb(uint32_t *args)
{
//    MQTT_Client* client = (MQTT_Client*)args;
    os_printf("MQTT: Published\r\n");
}


//Main routine
void ICACHE_FLASH_ATTR user_init(void) {

	stdoutInit();	
	os_delay_us(100000);
	CFG_Load();
	ioInit();
	
	WIFI_Connect(wifiConnectCb);
	
	httpdInit(builtInUrls, sysCfg.httpd_port);
	
	if(sysCfg.ntp_enable==1) {
		sntp_init(sysCfg.ntp_tz);	//timezone
	}
	
	if(sysCfg.mqtt_enable==1) {
		MQTT_InitConnection(&mqttClient, (uint8_t *)sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.mqtt_use_ssl );
		MQTT_InitClient(&mqttClient, (uint8_t *)sysCfg.mqtt_devid, (uint8_t *)sysCfg.mqtt_user, (uint8_t *)sysCfg.mqtt_pass, sysCfg.mqtt_keepalive,1);
		MQTT_OnConnected(&mqttClient, mqttConnectedCb);
		MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
		MQTT_OnPublished(&mqttClient, mqttPublishedCb);		
		MQTT_OnData(&mqttClient, mqttDataCb);
		
	}
	
	if(sysCfg.sensor_dht22_enable) 
		DHTInit(SENSOR_DHT22, 30000);
		
	if(sysCfg.sensor_ds18b20_enable) 
		ds_init(30000);

	broadcastd_init();

	thermostat_init(30000);

/*
	//Netbios to set the name
	struct softap_config wiconfig;
	os_memset(netbios_name, ' ', sizeof(netbios_name)-1);
	if(wifi_softap_get_config(&wiconfig)) {
		int i;
		for(i = 0; i < sizeof(netbios_name)-1; i++) {
			if(wiconfig.ssid[i] < ' ') break;
			netbios_name[i] = wiconfig.ssid[i];
		};
	}
	else os_sprintf(netbios_name, "ESP8266");
	netbios_name[sizeof(netbios_name)-1]='\0';
	netbios_init();
*/
		
	os_printf("\nRelay Board Ready\n");	
	os_printf("Free heap size:%d\n",system_get_free_heap_size());

	
#ifdef CGIPWM_H	
	//Mind the PWM pin!! defined in pwm.h
	duty=0;
	pwm_init( 50, &duty);
	pwm_set_duty(duty, 0);
    pwm_start();
#endif
	
	//OLEDInit();
	
}





