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
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "Httpclient.h"
#include "user_config.h"

static ETSTimer test_timer;//test http



/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_rf_pre_init(void)
{

}
static void ICACHE_FLASH_ATTR test_timer_cb()
{
	os_printf("http://api.openweathermap.org/data/2.5/weather?q=beijing&APPID=fd470504ddf42c64c35a4fa30c2c4120\n");
	http_get("http://api.openweathermap.org/data/2.5/weather?q=beijing&APPID=fd470504ddf42c64c35a4fa30c2c4120", "", http_callback_example);
	// This test will fail. The DHCP request returns directly, but the TCP connection hangs.
	// FIXME: wait for network to be ready before connecting?
	os_timer_arm(&test_timer, 15000, 0);
}

void ICACHE_FLASH_ATTR http_test()
{
	// FIXME: what happens when no Wifi network is available?

	os_timer_disarm(&test_timer);
	os_timer_setfn(&test_timer, test_timer_cb, NULL);
	os_timer_arm(&test_timer, 10000, 0); //
}

void user_init(void)
{
    //TODO
	//smartconfig_set_type(SC_TYPE_ESPTOUCH);
	if(wifi_get_opmode() != STATION_MODE)//sta mode
	{
	    wifi_set_opmode(STATION_MODE);
	}	
	//esptouch_set_timeout(timeout_s);
	//smartconfig_start(smartconfig_done);
	//smartconfig_start(SC_TYPE_ESPTOUCH, smartconfig_done);

	struct station_config sta_conf;

	os_memset(&sta_conf,0,sizeof(sta_conf));
	os_memcpy(sta_conf.ssid, "TP-LINK-FD", sizeof(sta_conf.ssid));
	os_memcpy(sta_conf.password, "aaaaaaaa", sizeof(sta_conf.password));
	wifi_station_set_config(&sta_conf);
	wifi_station_disconnect();
	wifi_station_connect();
	http_test();	
}

