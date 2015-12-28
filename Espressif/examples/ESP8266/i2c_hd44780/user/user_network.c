/*
Tnx to Sprite_TM (source came from his esp8266ircbot)
*/

#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "user_network.h"
#include "user_display.h"
#include "user_config.h"

static char lineBuf[1024];
static int lineBufPos;
LOCAL os_timer_t network_timer;

static void ICACHE_FLASH_ATTR networkParseLine(struct espconn *conn, char *line) {
	char buff[1024];
    uint8 page, y;
    page = line[0];
    y = line[1];
    char* data = line + 2;    
    os_printf("P-L: %x-%x: %s\n\r",page,y,data);
    display_data(page, y, data);
}

static void ICACHE_FLASH_ATTR networkParseChar(struct espconn *conn, char c) {
	lineBuf[lineBufPos++]=c;
	if (lineBufPos>=sizeof(lineBuf)) lineBufPos--;

	if (lineBufPos>2 && lineBuf[lineBufPos-1]=='\n') {
		lineBuf[lineBufPos-1]=0;
		networkParseLine(conn, lineBuf);
		lineBufPos=0;
	}
}

static void ICACHE_FLASH_ATTR networkRecvCb(void *arg, char *data, unsigned short len) {
	struct espconn *conn=(struct espconn *)arg;
	int x;
	for (x=0; x<len; x++) networkParseChar(conn, data[x]);
}

static void ICACHE_FLASH_ATTR networkConnectedCb(void *arg) {
	struct espconn *conn=(struct espconn *)arg;
	espconn_regist_recvcb(conn, networkRecvCb);
	lineBufPos=0;
    os_printf("connected\n\r");
}

static void ICACHE_FLASH_ATTR networkReconCb(void *arg, sint8 err) {
    os_printf("Reconnect\n\r");
	network_init();
}

static void ICACHE_FLASH_ATTR networkDisconCb(void *arg) {
    os_printf("Disconnect\n\r");
	network_init();
}

static void ICACHE_FLASH_ATTR networkServerFoundCb(const char *name, ip_addr_t *ip, void *arg) {
	static esp_tcp tcp;
	struct espconn *conn=(struct espconn *)arg;
	if (ip==NULL) {
        os_printf("Nslookup failed :/ Trying again...\n");
		network_init();
	}

    os_printf("Server at %d.%d.%d.%d\n",
        *((uint8 *)&ip->addr), *((uint8 *)&ip->addr + 1),
        *((uint8 *)&ip->addr + 2), *((uint8 *)&ip->addr + 3));

    char page_buffer[20];
    os_sprintf(page_buffer,"DST: %d.%d.%d.%d",
        *((uint8 *)&ip->addr), *((uint8 *)&ip->addr + 1),
        *((uint8 *)&ip->addr + 2), *((uint8 *)&ip->addr + 3));
    LCD_setCursor(0,1);
    LCD_print(page_buffer);
    LCD_setCursor(11,3);
	conn->type=ESPCONN_TCP;
	conn->state=ESPCONN_NONE;
	conn->proto.tcp=&tcp;
	conn->proto.tcp->local_port=espconn_port();
	conn->proto.tcp->remote_port=12346;
	os_memcpy(conn->proto.tcp->remote_ip, &ip->addr, 4);

	espconn_regist_connectcb(conn, networkConnectedCb);
	espconn_regist_disconcb(conn, networkDisconCb);
	espconn_regist_reconcb(conn, networkReconCb);
	espconn_connect(conn);
}

void ICACHE_FLASH_ATTR
network_start() {
	static struct espconn conn;
	static ip_addr_t ip;
    os_printf("Looking up server...\n");
	espconn_gethostbyname(&conn, "verlichting.client.home.gigafreak.net", &ip, networkServerFoundCb);
}

void ICACHE_FLASH_ATTR
network_check_ip(void)
{
    struct ip_info ipconfig;

    os_timer_disarm(&network_timer);

    wifi_get_ip_info(STATION_IF, &ipconfig);


    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
        char page_buffer[20];
        os_sprintf(page_buffer,"IP: %d.%d.%d.%d",IP2STR(&ipconfig.ip));
        LCD_setCursor(0,0);
        LCD_print(page_buffer);
        LCD_setCursor(11,3);
        network_start();
    }
    else 
    {
        os_printf("No ip found\n\r");
        os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
        os_timer_arm(&network_timer, 1000, 0);
    } 
        
}

void ICACHE_FLASH_ATTR
network_init()
{
    os_timer_disarm(&network_timer);
    os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
    os_timer_arm(&network_timer, 1000, 0);    
}
