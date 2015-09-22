/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"

#include "httpd.h"
#include "httpdespfs.h"
#include "cgiwifi.h"
#include "espfs.h"
#include "captdns.h"
#include "webpages-espfs.h"
#include "user_cgi.h"
#include "user_switch.h"

#ifdef SERVER_SSL_ENABLE
#include "ssl/cert.h"
#include "ssl/private_key.h"
#else
#ifdef CLIENT_SSL_ENABLE
unsigned char *default_certificate;
unsigned int default_certificate_len = 0;
unsigned char *default_private_key;
unsigned int default_private_key_len = 0;
#endif
#endif



#include "gpio.h"
#include "user_interface.h"
void user_rf_pre_init(void)
{
	user_SwitchInit();
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
    light_switch_action()
{
    user_SwitchReact();
}

HttpdBuiltInUrl builtInUrls[]={
	{"*", cgiRedirectApClientToHostname, "lightswitch.local"},
	{"/", cgiRedirect, "/index.html"},
	{"/wifi", cgiRedirect, "/wifi/wifi.tpl"},
	{"/wifi/wifiscan.cgi", cgiWiFiScan, NULL},
	{"/wifi/wifi.tpl", cgiEspFsTemplate, tplWlan},
	{"/wifi/connect.cgi", cgiWiFiConnect, NULL},
	{"/wifi/connstatus.cgi", cgiWiFiConnStatus, NULL},
	{"/wifi/setmode.cgi", cgiWiFiSetMode, NULL},

	{"/getconfig.cgi", cgiGetConfig, NULL},
	{"/setconfig.cgi", cgiSetConfig, NULL},

	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL} //end marker
};


void scanDone(void *arg, STATUS status) {
	wifi_station_connect();
}

os_timer_t apDownTimer;


void apDown(void *arg) {
	//5 minutes have passed. Kill power.
	configSave();
	_SWITCH_GPIO_RELEASE();
}

void initAPMode() {
	wifi_station_disconnect();
    wifi_set_opmode(STATIONAP_MODE);
	wifi_station_scan(NULL, scanDone);
	//Initialize DNS server for captive portal
	captdnsInit();
	//Initialize espfs containing static webpages
	espFsInit((void*)(webpages_espfs_start));
	//Initialize webserver
	httpdInit(builtInUrls, 80);
	//Make sure we don't kill ourselves
	_SWITCH_GPIO_HOLD();
	//but do so after a few minutes anyway
	os_timer_disarm(&apDownTimer);
	os_timer_setfn(&apDownTimer, apDown, NULL);
	os_timer_arm(&apDownTimer, 1000*60*5,0);
}

uint32 user_GetBatteryVoltageMv() {
	int r=system_get_vdd33();
	int mv=(r*1000)/1024;
	os_printf("BATTERY VAL: %d, raw %d\n", mv, r);
	return mv;
}


void user_init(void)
{
    uart_div_modify(0, 80000000 / 115200);//SET BAUDRATE
    wifi_set_opmode(STATION_MODE);
	wifi_set_channel(1);

	//Load config from flash
	configLoad();

	//Initialize DNS server for captive portal
	captdnsInit();
	//Initialize espfs containing static webpages
	espFsInit((void*)(webpages_espfs_start));
	//Initialize webserver
	httpdInit(builtInUrls, 80);

    //SEND ACTION COMMAND ACCORDING TO GPIO STATUS
	system_init_done_cb(light_switch_action);
}



