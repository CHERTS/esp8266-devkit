#include "ets_sys.h"
#include "user_interface.h"
#include "driver/uart.h"

#include "user_tcp_server.h"

extern int ets_uart_printf(const char *fmt, ...);
int (*console_printf)(const char *fmt, ...) = ets_uart_printf;
static char macaddr[6];

const char *WiFiMode[] =
{
		"NULL",		// 0x00
		"STATION",	// 0x01
		"SOFTAP", 	// 0x02
		"STATIONAP"	// 0x03
};

void setup_wifi_ap_mode(void)
{
	wifi_set_opmode((wifi_get_opmode()|STATIONAP_MODE)&USE_WIFI_MODE);
	struct softap_config apconfig;
	if(wifi_softap_get_config(&apconfig))
	{
		wifi_softap_dhcps_stop();
		os_memset(apconfig.ssid, 0, sizeof(apconfig.ssid));
		os_memset(apconfig.password, 0, sizeof(apconfig.password));
		apconfig.ssid_len = os_sprintf(apconfig.ssid, WIFI_AP_NAME);
		os_sprintf(apconfig.password, "%s", WIFI_AP_PASSWORD);
		apconfig.authmode = AUTH_WPA_WPA2_PSK;
		apconfig.ssid_hidden = 0;
		apconfig.channel = 7;
		apconfig.max_connection = 4;
		if(!wifi_softap_set_config(&apconfig))
		{
			#if DEBUG_LEVEL > 0
			ets_uart_printf("ESP8266 not set AP config!\r\n");
			#endif
		};
		struct ip_info ipinfo;
		wifi_get_ip_info(SOFTAP_IF, &ipinfo);
		IP4_ADDR(&ipinfo.ip, 192, 168, 4, 1);
		IP4_ADDR(&ipinfo.gw, 192, 168, 4, 1);
		IP4_ADDR(&ipinfo.netmask, 255, 255, 255, 0);
		wifi_set_ip_info(SOFTAP_IF, &ipinfo);
		wifi_softap_dhcps_start();
	}
	#if DEBUG_LEVEL > 0
	ets_uart_printf("ESP8266 in AP mode configured.\r\n");
	#endif
}

void setup_wifi_st_mode(void)
{
	wifi_set_opmode((wifi_get_opmode()|STATIONAP_MODE)&USE_WIFI_MODE);
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
			#if DEBUG_LEVEL > 0
			ets_uart_printf("ESP8266 not set station config!\r\n");
			#endif
		}
	}
	wifi_station_connect();
	wifi_station_dhcpc_start();
	wifi_station_set_auto_connect(1);
	#if DEBUG_LEVEL > 0
	ets_uart_printf("ESP8266 in STA mode configured.\r\n");
	#endif
}

void init_done(void)
{
	#if DEBUG_LEVEL > 0
	char temp[80];
	// Print system info
	ets_uart_printf("\r\n");
	ets_uart_printf("ESP8266 platform starting...\r\n");
	ets_uart_printf("System info:\r\n");
	os_sprintf(temp, "SDK version: %s\r\n", system_get_sdk_version());
	ets_uart_printf(temp);
	os_sprintf(temp, "Time = %ld\r\n", system_get_time());
	ets_uart_printf(temp);
	os_sprintf(temp, "Chip id = 0x%x\r\n", system_get_chip_id());
	ets_uart_printf(temp);
	os_sprintf(temp, "CPU freq = %d MHz\r\n", system_get_cpu_freq());
	ets_uart_printf(temp);
	os_sprintf(temp, "Free heap size = %d\r\n", system_get_free_heap_size());
	ets_uart_printf(temp);
	#endif

	//if(wifi_get_opmode() != USE_WIFI_MODE)
	{
		#if DEBUG_LEVEL > 0
		console_printf("ESP8266 is %s mode, restarting in %s mode...\r\n", WiFiMode[wifi_get_opmode()], WiFiMode[USE_WIFI_MODE]);
		#endif
		if(USE_WIFI_MODE & SOFTAP_MODE)
			setup_wifi_ap_mode();
		if(USE_WIFI_MODE & STATION_MODE)
			setup_wifi_st_mode();
	}
	if(USE_WIFI_MODE & SOFTAP_MODE)
		wifi_get_macaddr(SOFTAP_IF, macaddr);
	if(USE_WIFI_MODE & STATION_MODE)
		wifi_get_macaddr(STATION_IF, macaddr);
	if(wifi_get_phy_mode() != PHY_MODE_11N)
		wifi_set_phy_mode(PHY_MODE_11N);
	if(wifi_station_get_auto_connect() == 0)
		wifi_station_set_auto_connect(1);

	#if DEBUG_LEVEL > 0
	console_printf("Wi-Fi mode: %s\r\n", WiFiMode[wifi_get_opmode()]);
	if(USE_WIFI_MODE & SOFTAP_MODE)
	{
		struct softap_config apConfig;
		if(wifi_softap_get_config(&apConfig)) {
			console_printf("AP config: SSID: %s, PASSWORD: %s\r\n",
					apConfig.ssid,
					apConfig.password);
		}
	}
	if(USE_WIFI_MODE & STATION_MODE)
	{
		struct station_config stationConfig;
		if(wifi_station_get_config(&stationConfig)) {
			console_printf("STA config: SSID: %s, PASSWORD: %s\r\n",
			stationConfig.ssid,
			stationConfig.password);
		}
	}
	#endif

	// инициализация и запуск tcp сервера на порту 80
	TCP_SERV_CFG *p = tcpsrv_init(80);
	if(p != NULL) {
		// изменим конфиг на наше усмотрение:
		p->max_conn = 3;
		p->time_wait_rec = 30;
		p->time_wait_cls = 30;
		p->min_heap = 16384;
		ets_uart_printf("Max connection %d, time waits %d & %d, min heap size %d\n", p->max_conn, p->time_wait_rec, p->time_wait_cls, p->min_heap );
		// слинкуем с желаемыми процедурами:
		//p->func_discon_cb = NULL;
		//p->func_listen =
		//p->func_sent_cb =
		//p->func_recv =
		if(!tcpsrv_start(p)) tcpsrv_close(p);
	}
	// инициализация и запуск tcp сервера на порту 8080
	p = tcpsrv_init(8080);
	if(p != NULL) {
		// изменим конфиг на наше усмотрение:
		p->max_conn = 99;
		p->time_wait_rec = 30;
		p->time_wait_cls = 30;
		p->min_heap = 16384;
		ets_uart_printf("Max connection %d, time waits %d & %d, min heap size %d\n", p->max_conn, p->time_wait_rec, p->time_wait_cls, p->min_heap );
		if(!tcpsrv_start(p)) tcpsrv_close(p);
	}
	#if DEBUG_LEVEL > 0
	ets_uart_printf("ESP8266 platform started!\r\n");
	#endif
}

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

void ICACHE_FLASH_ATTR user_init(void)
{
	// Configure the UART
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	system_init_done_cb(init_done);
}
