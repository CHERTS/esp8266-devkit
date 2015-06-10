/*
 *
 *  ESP8266 firmware to receive lighting data over TPM2, Art-Net and drive a WS2801 addressable LED strip.
 *
 *  Created on: Nov 15, 2014
 *  Author: frans-willem
 *
 *  Changed on 13.01.2015
 *  Author: Mikhail Grigorev aka CHERTS <sleuthhound@gmail.com>
 *
 *  Official repo: https://github.com/Frans-Willem/EspLightNode
 *
 *	ws2801 connected to GPIO0 and GPIO2
 *
 *  Art-Net Protocol Specification: http://www.artisticlicence.com/WebSiteMaster/User%20Guides/art-net.pdf
 *
 */

#include "tpm2net.h"
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "driver/uart.h"
#include "ws2801.h"
#include "artnet.h"
#include "tpm2net.h"
#include "httpd.h"
#include "config.h"

static volatile os_timer_t client_timer;

static void ICACHE_FLASH_ATTR wait_for_ip(uint8 flag)
{
    LOCAL struct ip_info ipconfig;
    os_timer_disarm(&client_timer);

    switch(wifi_station_get_connect_status())
	{
    	case STATION_GOT_IP:
            wifi_get_ip_info(STATION_IF, &ipconfig);
            if( ipconfig.ip.addr != 0) {
    			#ifdef PLATFORM_DEBUG
            	ets_uart_printf("WiFi connected\r\n");
    			#endif
            	//Start UDP server
    			#ifdef ENABLE_TPM2
            	tpm2net_init();
    			#endif
    			#ifdef ENABLE_ARTNET
            	artnet_init();
    			#endif
            	httpd_init();
            } else {
                os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
                os_timer_arm(&client_timer, 500, 0);
            }
			break;
		case STATION_WRONG_PASSWORD:
			#ifdef PLATFORM_DEBUG
			ets_uart_printf("WiFi connecting error, wrong password\r\n");
			#endif
            os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
            os_timer_arm(&client_timer, 1000, 0);
			break;
		case STATION_NO_AP_FOUND:
			#ifdef PLATFORM_DEBUG
			ets_uart_printf("WiFi connecting error, ap not found\r\n");
			#endif
            os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
            os_timer_arm(&client_timer, 500, 0);
			break;
		case STATION_CONNECT_FAIL:
			#ifdef PLATFORM_DEBUG
			ets_uart_printf("WiFi connecting fail\r\n");
			#endif
            os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
            os_timer_arm(&client_timer, 1000, 0);
			break;
		default:
			#ifdef PLATFORM_DEBUG
			ets_uart_printf("WiFi connecting...\r\n");
			#endif
            os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
            os_timer_arm(&client_timer, 1000, 0);
	}
}

static void ICACHE_FLASH_ATTR system_is_done(void){
	//Bringing up WLAN
	wifi_station_connect();
	wifi_station_dhcpc_start();
	if(wifi_station_get_auto_connect() == 0)
		wifi_station_set_auto_connect(1);
	//Wait for connection
	os_timer_disarm(&client_timer);
	os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
	os_timer_arm(&client_timer, 100, 0);
}

void ICACHE_FLASH_ATTR setup_wifi_st_mode(void)
{
	wifi_set_opmode((wifi_get_opmode()|STATION_MODE)&STATIONAP_MODE);
	struct station_config stconfig;
	wifi_station_disconnect();
	wifi_station_dhcpc_stop();
	if(wifi_station_get_config(&stconfig))
	{
		os_memset(stconfig.ssid, 0, sizeof(stconfig.ssid));
		os_memset(stconfig.password, 0, sizeof(stconfig.password));
		os_sprintf(stconfig.ssid, "%s", WIFI_SSID);
		os_sprintf(stconfig.password, "%s", WIFI_PASSWORD);
		if(!wifi_station_set_config(&stconfig))
		{
			#ifdef PLATFORM_DEBUG
			ets_uart_printf("ESP8266 not set station config!\r\n");
			#endif
		}
	}
	if(wifi_get_phy_mode() != PHY_MODE_11N)
		wifi_set_phy_mode(PHY_MODE_11N);
	#ifdef PLATFORM_DEBUG
	ets_uart_printf("ESP8266 in STA mode configured.\r\n");
	#endif
}

void user_rf_pre_init(void)
{
}

//Init function
void ICACHE_FLASH_ATTR user_init()
{
	// 115200
	UARTInit();
	os_delay_us(1000);

	config_load();
	setup_wifi_st_mode();
    ws2801_init();

    //Wait for system to be done.
    system_init_done_cb(&system_is_done);
    //system_os_task()
}
