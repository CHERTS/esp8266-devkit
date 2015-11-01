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

#if 0
#include "httpd.h"
#include "httpdespfs.h"
#include "cgiwifi.h"
#include "espfs.h"
#include "captdns.h"
#include "webpages-espfs.h"
#endif

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
	extern void ieee80211_mesh_quick_init();
	ieee80211_mesh_quick_init();

	os_printf("ESPNOW ENABLE 6M TX RATE\r\n");
	wifi_enable_6m_rate(true);
    user_SwitchReact();
}

/*
HttpdBuiltInUrl builtInUrls[]={
	{"*", cgiRedirectApClientToHostname, "lightswitch.local"},
	{"/", cgiRedirect, "/wifi.tpl"},
	{"/wifiscan.cgi", cgiWiFiScan, NULL},
	{"/wifi.tpl", cgiEspFsTemplate, tplWlan},
	{"/connect.cgi", cgiWiFiConnect, NULL},
	{"/connstatus.cgi", cgiWiFiConnStatus, NULL},
	{"/setmode.cgi", cgiWiFiSetMode, NULL},

	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL} //end marker
};
*/


uint32 user_GetBatteryVoltageMv() {
	int r=system_get_vdd33();
	int mv=(r*1000)/1024;
	os_printf("BATTERY VAL: %d, raw %d\n", mv, r);
	return mv;
}

void user_init(void)
{
#if LIGHT_DEVICE
#elif LIGHT_SWITCH
    uart_init(115200,115200);
    wifi_set_opmode(STATION_MODE);
	wifi_set_channel(1);

	//Initialize DNS server for captive portal
	//captdnsInit();
	//Initialize espfs containing static webpages
	//espFsInit((void*)(webpages_espfs_start));
	//Initialize webserver
	//httpdInit(builtInUrls, 80);

    //SEND ACTION COMMAND ACCORDING TO GPIO STATUS
    system_init_done_cb(light_switch_action);

#endif
}




