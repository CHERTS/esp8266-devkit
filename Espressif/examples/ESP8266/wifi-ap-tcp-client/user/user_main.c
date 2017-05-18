#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espconn.h"
#include "mem.h"
#include "gpio.h"
#include "user_config.h"

extern int ets_uart_printf(const char *fmt, ...);
static char macaddr[6];
static ETSTimer BtnTimer;
uint16_t GPIO_Time_Active = 0;

static void ICACHE_FLASH_ATTR
at_tcpclient_sent_cb(void *arg) {
	#ifdef PLATFORM_DEBUG
	ets_uart_printf("Send callback\r\n");
	#endif
	struct espconn *pespconn = (struct espconn *)arg;
	espconn_disconnect(pespconn);
}

static void ICACHE_FLASH_ATTR
at_tcpclient_discon_cb(void *arg) {
	struct espconn *pespconn = (struct espconn *)arg;
	os_free(pespconn->proto.tcp);
	os_free(pespconn);
	#ifdef PLATFORM_DEBUG
	ets_uart_printf("Disconnect callback\r\n");
	#endif
}

static void ICACHE_FLASH_ATTR
at_tcpclient_connect_cb(void *arg)
{
	struct espconn *pespconn = (struct espconn *)arg;
	#ifdef PLATFORM_DEBUG
	ets_uart_printf("TCP client connect\r\n");
	#endif
	espconn_regist_sentcb(pespconn, at_tcpclient_sent_cb);
	//espconn_regist_recvcb(pespconn, at_tcpclient_recv);
	espconn_regist_disconcb(pespconn, at_tcpclient_discon_cb);
	char payload[128];
	os_sprintf(payload, MACSTR ",%s\r\n", MAC2STR(macaddr), "ESP8266");
	#ifdef PLATFORM_DEBUG
	ets_uart_printf(payload);
	#endif
	espconn_sent(pespconn, payload, strlen(payload));
}

static void ICACHE_FLASH_ATTR
senddata()
{
	char info[150];
	char tcpserverip[15];
	struct espconn *pCon = (struct espconn *)os_zalloc(sizeof(struct espconn));
	if (pCon == NULL)
	{
		#ifdef PLATFORM_DEBUG
		ets_uart_printf("TCP connect failed\r\n");
		#endif
		return;
	}
	pCon->type = ESPCONN_TCP;
	pCon->state = ESPCONN_NONE;
	os_sprintf(tcpserverip, "%s", TCPSERVERIP);
	uint32_t ip = ipaddr_addr(tcpserverip);
	pCon->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	pCon->proto.tcp->local_port = espconn_port();
	pCon->proto.tcp->remote_port = TCPSERVERPORT;
	os_memcpy(pCon->proto.tcp->remote_ip, &ip, 4);
	espconn_regist_connectcb(pCon, at_tcpclient_connect_cb);
	//espconn_regist_reconcb(pCon, at_tcpclient_recon_cb);
	#ifdef PLATFORM_DEBUG
	os_sprintf(info,"Start espconn_connect to " IPSTR ":%d\r\n",
		   IP2STR(pCon->proto.tcp->remote_ip),
		   pCon->proto.tcp->remote_port);
	ets_uart_printf(info);
	#endif
	espconn_connect(pCon);
}

static void ICACHE_FLASH_ATTR BtnTimerCb(void *arg)
{
	if (!GPIO_INPUT_GET(BTNGPIO))
	{
		GPIO_Time_Active++;
	} else {
		if (GPIO_Time_Active != 0)
		{
			#ifdef PLATFORM_DEBUG
			ets_uart_printf("Start sending data...\r\n");
			#endif
			senddata();
		}
		GPIO_Time_Active = 0;
	}
}

void BtnInit() {
	// Select pin function
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	// Enable pull up R
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);  
	// Set GPIO0 as input mode
	gpio_output_set(0, 0, 0, BIT0);
	os_timer_disarm(&BtnTimer);
	os_timer_setfn(&BtnTimer, BtnTimerCb, NULL);
	os_timer_arm(&BtnTimer, 500, 1);
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

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{
}

void ICACHE_FLASH_ATTR user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(100);

	#ifdef PLATFORM_DEBUG
	ets_uart_printf("ESP8266 platform starting...\r\n");
	#endif
    
	struct softap_config apConfig;
	struct ip_info ipinfo;
	char ssid[33];
	char password[33];
	char macaddress[17];
	char info[150];

	if(wifi_get_opmode() != SOFTAP_MODE)
	{
		#ifdef PLATFORM_DEBUG
		ets_uart_printf("ESP8266 not in SOFTAP mode, restarting in SOFTAP mode...\r\n");
		#endif
		wifi_set_opmode(SOFTAP_MODE);
		//after esp_iot_sdk_v0.9.2, need not to restart
		//system_restart();
	}

	IP4_ADDR(&ipinfo.ip, 10, 10, 10, 1);
	IP4_ADDR(&ipinfo.gw, 10, 10, 10, 1);
	IP4_ADDR(&ipinfo.netmask, 255, 255, 255, 0);
	wifi_set_ip_info(SOFTAP_IF, &ipinfo);

	wifi_get_macaddr(SOFTAP_IF, macaddr);
	wifi_softap_get_config(&apConfig);
	os_memset(apConfig.ssid, 0, sizeof(apConfig.ssid));
	os_sprintf(ssid, "%s", WIFI_APSSID);
	os_memcpy(apConfig.ssid, ssid, os_strlen(ssid));
	if (wifi_get_opmode() == SOFTAP_MODE) 
	{
		#ifdef WIFI_APWPA
		os_memset(apConfig.password, 0, sizeof(apConfig.password));
		os_sprintf(password, "%s", WIFI_APPASSWORD);
		os_memcpy(apConfig.password, password, os_strlen(password));
		apConfig.authmode = AUTH_WPA_WPA2_PSK;
		#else
		apConfig.authmode = AUTH_OPEN;
		#endif
		apConfig.channel = 7;
		apConfig.max_connection = 255;
		apConfig.ssid_hidden = 0;
		wifi_softap_set_config(&apConfig);
	}

	#ifdef PLATFORM_DEBUG
	if (wifi_get_opmode() == SOFTAP_MODE) 
	{
		wifi_softap_get_config(&apConfig);
		os_sprintf(macaddress, MACSTR, MAC2STR(macaddr));
		os_sprintf(info,"OPMODE: %u, SSID: %s, PASSWORD: %s, CHANNEL: %d, AUTHMODE: %d, MACADDRESS: %s\r\n",
					wifi_get_opmode(),
					apConfig.ssid,
					apConfig.password,
					apConfig.channel,
					apConfig.authmode,
					macaddress);
		ets_uart_printf(info);
	}
	#endif

	BtnInit();

	#ifdef PLATFORM_DEBUG
	ets_uart_printf("ESP8266 platform started!\r\n");
	#endif
}
