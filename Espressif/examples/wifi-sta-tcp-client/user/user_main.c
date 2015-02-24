#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "user_config.h"
#ifndef LWIP_DEBUG
#include "espconn.h"
#endif
#ifdef LWIP_DEBUG
#include "lwip/netif.h"
#include "lwip/inet.h"
#include "netif/etharp.h"
#include "lwip/tcp.h"
#include "lwip/ip.h"
#include "lwip/init.h"
#include "lwip/tcp_impl.h"
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip/udp.h"
#include "lwip/app/espconn.h"
#include "lwip/app/espconn_tcp.h"
#include "lwip/app/espconn_udp.h"
#endif

typedef enum {
	WIFI_CONNECTING,
	WIFI_CONNECTING_ERROR,
	WIFI_CONNECTED,
	TCP_DISCONNECTED,
	TCP_CONNECTING,
	TCP_CONNECTING_ERROR,
	TCP_CONNECTED,
	TCP_SENDING_DATA_ERROR,
	TCP_SENT_DATA
} tConnState;

struct espconn Conn;
esp_tcp ConnTcp;
extern int ets_uart_printf(const char *fmt, ...);
int (*console_printf)(const char *fmt, ...) = ets_uart_printf;
static char macaddr[6];
static ETSTimer WiFiLinker;
static tConnState connState = WIFI_CONNECTING;
static unsigned char tcpReconCount;

static void platform_reconnect(struct espconn *);
static void wifi_check_ip(void *arg);

const char *WiFiMode[] =
{
		"NULL",		// 0x00
		"STATION",	// 0x01
		"SOFTAP", 	// 0x02
		"STATIONAP"	// 0x03
};

#ifdef PLATFORM_DEBUG
// enum espconn state, see file /include/lwip/api/err.c
const char *sEspconnErr[] =
{
		"Ok",                    // ERR_OK          0
		"Out of memory error",   // ERR_MEM        -1
		"Buffer error",          // ERR_BUF        -2
		"Timeout",               // ERR_TIMEOUT    -3
		"Routing problem",       // ERR_RTE        -4
		"Operation in progress", // ERR_INPROGRESS -5
		"Illegal value",         // ERR_VAL        -6
		"Operation would block", // ERR_WOULDBLOCK -7
		"Connection aborted",    // ERR_ABRT       -8
		"Connection reset",      // ERR_RST        -9
		"Connection closed",     // ERR_CLSD       -10
		"Not connected",         // ERR_CONN       -11
		"Illegal argument",      // ERR_ARG        -12
		"Address in use",        // ERR_USE        -13
		"Low-level netif error", // ERR_IF         -14
		"Already connected"      // ERR_ISCONN     -15
};
#endif

#ifdef LWIP_DEBUG
// enum tcp_state, see file /include/lwip/tcp.h
const char * const deb_tcp_state_str[] = {
  "CLOSED",      // 0  CLOSED
  "LISTEN",      // 1  LISTEN
  "SYN_SENT",    // 2  SYN_SENT
  "SYN_RCVD",    // 3  SYN_RCVD
  "ESTABLISHED", // 4  ESTABLISHED
  "FIN_WAIT_1",  // 5  FIN_WAIT_1
  "FIN_WAIT_2",  // 6  FIN_WAIT_2
  "CLOSE_WAIT",  // 7  CLOSE_WAIT
  "CLOSING",     // 8  CLOSING
  "LAST_ACK",    // 9  LAST_ACK
  "TIME_WAIT"    // 10 TIME_WAIT
};

void ICACHE_FLASH_ATTR list_lwip_tcp_psc(void)
{
	struct tcp_pcb *pcb;
	console_printf("LWIP_DEBUG: Lwip Active PCB states:\r\n");
	bool prt_none = true;
	for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) {
		console_printf("LWIP_DEBUG: Port %u | %u flg:%02x tmr:%04x %s\r\n", pcb->local_port, pcb->remote_port, pcb->flags, pcb->tmr, deb_tcp_state_str[pcb->state]);
		prt_none = false;
	}
	if(prt_none) console_printf("LWIP_DEBUG: none\r\n");
	console_printf("LWIP_DEBUG: Lwip Listen PCB states:\r\n");
	prt_none = true;
	for(pcb = (struct tcp_pcb *)tcp_listen_pcbs.pcbs; pcb != NULL; pcb = pcb->next) {
		console_printf("LWIP_DEBUG: Port %u | %u flg:%02x tmr:%04x %s\r\n", pcb->local_port, pcb->remote_port, pcb->flags, pcb->tmr, deb_tcp_state_str[pcb->state]);
		prt_none = false;
	}
	if(prt_none) console_printf("LWIP_DEBUG: none\r\n");
	console_printf("LWIP_DEBUG: Lwip TIME-WAIT PCB states:\r\n");
	prt_none = true;
	for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) {
		console_printf("LWIP_DEBUG: Port %u | %u flg:%02x tmr:%04x %s\r\n", pcb->local_port, pcb->remote_port, pcb->flags, pcb->tmr, deb_tcp_state_str[pcb->state]);
		prt_none = false;
	}
	if(prt_none) console_printf("LWIP_DEBUG: none\r\n");
	return;
}

static char *msg_espconn_state[] =
{
		"NONE",
		"WAIT",
		"LISTEN",
		"CONNECT",
		"WRITE",
		"READ",
		"CLOSE"
};

void ICACHE_FLASH_ATTR list_espconn_tcp(struct espconn *esp_conn)
{
	remot_info *pinfo;
	if(espconn_get_connection_info(esp_conn, &pinfo , ESPCONN_NORM)==ESPCONN_OK) // ESPCONN_SSL, ESPCONN_UDP
	{
		if(esp_conn->link_cnt) {
			console_printf("LWIP_DEBUG: EspConn %u TCP connections:\r\n", esp_conn->link_cnt);
			int	i = 0;
			while(i < esp_conn->link_cnt) {
				console_printf("LWIP_DEBUG: %d) " IPSTR ":%u %s\r\n", i,  IP2STR(pinfo[i].remote_ip), pinfo[i].remote_port, msg_espconn_state[pinfo[i].state] );
				i++;
			};
		}
		else console_printf("LWIP_DEBUG: EspConn not info TCP connection(!)\r\n"); // очередная беда в espconn :(
	}
	else console_printf("LWIP_DEBUG: Error: espconn_get_connection_info!\r\n");
}
#endif

static void ICACHE_FLASH_ATTR tcpclient_sent_cb(void *arg)
{
	struct espconn *pespconn = arg;
	#ifdef PLATFORM_DEBUG
	ets_uart_printf("tcpclient_sent_cb\r\n");
	#endif
	#ifdef LWIP_DEBUG
	list_espconn_tcp(pespconn);
	list_lwip_tcp_psc();
	#endif
	espconn_disconnect(pespconn);
}

static void ICACHE_FLASH_ATTR tcpclient_discon_cb(void *arg)
{
	struct espconn *pespconn = arg;
	connState = TCP_DISCONNECTED;
	#ifdef PLATFORM_DEBUG
	ets_uart_printf("tcpclient_discon_cb\r\n");
	#endif
	if (pespconn == NULL)
	{
		#ifdef PLATFORM_DEBUG
		ets_uart_printf("tcpclient_discon_cb - conn is NULL!\r\n");
		#endif
		return;
	}
	#ifdef PLATFORM_DEBUG
	ets_uart_printf("Will reconnect in 2s...\r\n");
	#endif
	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)platform_reconnect, pespconn);
	os_timer_arm(&WiFiLinker, 2000, 0);
}

static void ICACHE_FLASH_ATTR tcpclient_connect_cb(void *arg)
{
	struct espconn *pespconn = arg;
	tcpReconCount = 0;
	char payload[128];
	#ifdef PLATFORM_DEBUG
	ets_uart_printf("tcpclient_connect_cb\r\n");
	#endif
	espconn_regist_sentcb(pespconn, tcpclient_sent_cb);
	connState = TCP_CONNECTED;
	os_sprintf(payload, MACSTR ",%s", MAC2STR(macaddr), "ESP8266");
	sint8 espsent_status = espconn_sent(pespconn, payload, strlen(payload));
	if(espsent_status == ESPCONN_OK) {
		connState = TCP_SENT_DATA;
		#ifdef PLATFORM_DEBUG
		console_printf("Data sent, payload: %s\r\n", payload);
		#endif
	} else {
		connState = TCP_SENDING_DATA_ERROR;
		#ifdef PLATFORM_DEBUG
		console_printf("Error while sending data.\r\n");
		#endif
	}
	#ifdef LWIP_DEBUG
	list_espconn_tcp(pespconn);
	list_lwip_tcp_psc();
	#endif
}

static void ICACHE_FLASH_ATTR tcpclient_recon_cb(void *arg, sint8 err)
{
	struct espconn *pespconn = arg;
	connState = TCP_DISCONNECTED;

	#ifdef PLATFORM_DEBUG
	console_printf("tcpclient_recon_cb\r\n");
	if (err != ESPCONN_OK)
		console_printf("Connection error: %d - %s\r\n", err, ((err>-16)&&(err<1))? sEspconnErr[-err] : "?");
    #endif
	#ifdef LWIP_DEBUG
	list_espconn_tcp(pespconn);
	list_lwip_tcp_psc();
	#endif

    if (++tcpReconCount >= 5)
    {
		connState = TCP_CONNECTING_ERROR;
		tcpReconCount = 0;
		#ifdef PLATFORM_DEBUG
		console_printf("tcpclient_recon_cb, 5 failed TCP attempts!\r\n");
		console_printf("Will reconnect in 10s...\r\n");
		#endif
		os_timer_disarm(&WiFiLinker);
		os_timer_setfn(&WiFiLinker, (os_timer_func_t *)platform_reconnect, pespconn);
		os_timer_arm(&WiFiLinker, 10000, 0);
    }
    else
    {
		#ifdef PLATFORM_DEBUG
		console_printf("Will reconnect in 2s...\r\n");
		#endif
		os_timer_disarm(&WiFiLinker);
		os_timer_setfn(&WiFiLinker, (os_timer_func_t *)platform_reconnect, pespconn);
		os_timer_arm(&WiFiLinker, 2000, 0);
	}
}

static void ICACHE_FLASH_ATTR platform_reconnect(struct espconn *pespconn)
{
	#ifdef PLATFORM_DEBUG
	console_printf("platform_reconnect\r\n");
	#endif
	wifi_check_ip(NULL);
}

static void ICACHE_FLASH_ATTR senddata()
{
	os_timer_disarm(&WiFiLinker);
	char info[150];
	char tcpserverip[15];
	Conn.proto.tcp = &ConnTcp;
	Conn.type = ESPCONN_TCP;
	Conn.state = ESPCONN_NONE;
	os_sprintf(tcpserverip, "%s", TCPSERVERIP);
	uint32_t ip = ipaddr_addr(tcpserverip);
	os_memcpy(Conn.proto.tcp->remote_ip, &ip, 4);
	Conn.proto.tcp->local_port = espconn_port();
	Conn.proto.tcp->remote_port = TCPSERVERPORT;
	espconn_regist_connectcb(&Conn, tcpclient_connect_cb);
	espconn_regist_reconcb(&Conn, tcpclient_recon_cb);
	espconn_regist_disconcb(&Conn, tcpclient_discon_cb);
	#ifdef PLATFORM_DEBUG
	console_printf("Start espconn_connect to " IPSTR ":%d\r\n", IP2STR(Conn.proto.tcp->remote_ip), Conn.proto.tcp->remote_port);
	#endif
	#ifdef LWIP_DEBUG
	console_printf("LWIP_DEBUG: Start espconn_connect local port %u\r\n", Conn.proto.tcp->local_port);
	#endif
	sint8 espcon_status = espconn_connect(&Conn);
	#ifdef PLATFORM_DEBUG
	switch(espcon_status)
	{
		case ESPCONN_OK:
			console_printf("TCP created.\r\n");
			break;
		case ESPCONN_RTE:
			console_printf("Error connection, routing problem.\r\n");
			break;
		case ESPCONN_TIMEOUT:
			console_printf("Error connection, timeout.\r\n");
			break;
		default:
			console_printf("Connection error: %d\r\n", espcon_status);
	}
	#endif
	if(espcon_status != ESPCONN_OK) {
		#ifdef LWIP_DEBUG
		list_espconn_tcp(&Conn);
		list_lwip_tcp_psc();
		#endif
		os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&WiFiLinker, 1000, 0);
	}
}

static void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{
	struct ip_info ipConfig;
	os_timer_disarm(&WiFiLinker);
	switch(wifi_station_get_connect_status())
	{
		case STATION_GOT_IP:
			wifi_get_ip_info(STATION_IF, &ipConfig);
			if(ipConfig.ip.addr != 0) {
				connState = WIFI_CONNECTED;
				#ifdef PLATFORM_DEBUG
				ets_uart_printf("WiFi connected\r\n");
				ets_uart_printf("Start TCP connecting...\r\n");
				#endif
				connState = TCP_CONNECTING;
				senddata();
				return;
			}
			break;
		case STATION_WRONG_PASSWORD:
			connState = WIFI_CONNECTING_ERROR;
			#ifdef PLATFORM_DEBUG
			ets_uart_printf("WiFi connecting error, wrong password\r\n");
			#endif
			break;
		case STATION_NO_AP_FOUND:
			connState = WIFI_CONNECTING_ERROR;
			#ifdef PLATFORM_DEBUG
			ets_uart_printf("WiFi connecting error, ap not found\r\n");
			#endif
			break;
		case STATION_CONNECT_FAIL:
			connState = WIFI_CONNECTING_ERROR;
			#ifdef PLATFORM_DEBUG
			ets_uart_printf("WiFi connecting fail\r\n");
			#endif
			break;
		default:
			connState = WIFI_CONNECTING;
			#ifdef PLATFORM_DEBUG
			ets_uart_printf("WiFi connecting...\r\n");
			#endif
	}
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 1000, 0);
}

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
			#ifdef PLATFORM_DEBUG
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
	#ifdef PLATFORM_DEBUG
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
			#ifdef PLATFORM_DEBUG
			ets_uart_printf("ESP8266 not set station config!\r\n");
			#endif
		}
	}
	wifi_station_connect();
	wifi_station_dhcpc_start();
	wifi_station_set_auto_connect(1);
	#ifdef PLATFORM_DEBUG
	ets_uart_printf("ESP8266 in STA mode configured.\r\n");
	#endif
}

//Init function 
void ICACHE_FLASH_ATTR user_init()
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(1000);

	#ifdef PLATFORM_DEBUG
	ets_uart_printf("ESP8266 platform starting...\r\n");
	#endif
    
	//if(wifi_get_opmode() != USE_WIFI_MODE)
	{
		#ifdef PLATFORM_DEBUG
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

	#ifdef PLATFORM_DEBUG
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

	// Wait for Wi-Fi connection and start TCP connection
	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 1000, 0);

	#ifdef PLATFORM_DEBUG
	ets_uart_printf("ESP8266 platform started!\r\n");
	#endif
	#ifdef LWIP_DEBUG
	list_lwip_tcp_psc();
	#endif
}

