/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2015/1/23, v1.0 create this file.
*******************************************************************************/

#include "osapi.h"
#include "at_custom.h"
#include "user_interface.h"
#include "mem.h"
#include "espconn.h"


static struct espconn *at_espconn_demo_espconn_ptr = NULL;

#define AT_ESPCONN_DEMO_BUFFER_SIZE 	(2920)
static uint8 at_espconn_demo_buffer[AT_ESPCONN_DEMO_BUFFER_SIZE];
static uint32 at_espconn_demo_data_len = 0;
static bool at_espconn_demo_flag = FALSE;

static void ICACHE_FLASH_ATTR
at_espconn_demo_recon_cb(void *arg, sint8 errType)
{
	struct espconn *espconn_ptr = (struct espconn *)arg;

	os_printf("at demo espconn reconnect\r\n");
	at_espconn_demo_flag = FALSE;
    espconn_connect(espconn_ptr);
}


// notify at module that espconn has received data
static void ICACHE_FLASH_ATTR
at_espconn_demo_recv(void *arg, char *pusrdata, unsigned short len)
{
	at_fake_uart_rx(pusrdata,len);
}

static void ICACHE_FLASH_ATTR
at_espconn_demo_send_cb(void *arg)
{
	at_espconn_demo_flag = TRUE;
	if(at_espconn_demo_data_len) {
		espconn_send(at_espconn_demo_espconn_ptr,at_espconn_demo_buffer,at_espconn_demo_data_len);
		at_espconn_demo_data_len = 0;
	}
}
static void ICACHE_FLASH_ATTR
at_espconn_demo_discon_cb(void *arg)
{
  struct espconn *espconn_ptr = (struct espconn *)arg;

  os_printf("at demo espconn disconnected\r\n");
  at_espconn_demo_flag = FALSE;
  espconn_connect(espconn_ptr);
}

static void ICACHE_FLASH_ATTR
at_espconn_demo_connect_cb(void *arg)
{
	os_printf("at demo espconn connected\r\n");
	espconn_set_opt((struct espconn*)arg,ESPCONN_COPY);
	at_espconn_demo_flag = TRUE;
	at_espconn_demo_data_len = 0;
}

static void ICACHE_FLASH_ATTR at_espconn_demo_response(const uint8*data,uint32 length)
{
	if((data == NULL) || (length == 0)) {
		return;
	}

	if(at_espconn_demo_flag) {
		espconn_send(at_espconn_demo_espconn_ptr,(uint8*)data,length);
		at_espconn_demo_flag = FALSE;
	} else {
		if(length <= (AT_ESPCONN_DEMO_BUFFER_SIZE - at_espconn_demo_data_len)) {
			os_memcpy(at_espconn_demo_buffer + at_espconn_demo_data_len,data,length);
			at_espconn_demo_data_len += length;
		} else {
			os_printf("at espconn buffer full\r\n");
		}
	}
}


static void ICACHE_FLASH_ATTR at_espconn_demo_init(void)
{
  uint32 ip = 0;
  at_espconn_demo_espconn_ptr = (struct espconn *)os_zalloc(sizeof(struct espconn));
  at_espconn_demo_espconn_ptr->type = ESPCONN_TCP;
  at_espconn_demo_espconn_ptr->state = ESPCONN_NONE;
  at_espconn_demo_espconn_ptr->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  at_espconn_demo_espconn_ptr->proto.tcp->local_port = espconn_port();
  at_espconn_demo_espconn_ptr->proto.tcp->remote_port = 8999;

  ip = ipaddr_addr("192.168.1.120");
  os_memcpy(at_espconn_demo_espconn_ptr->proto.tcp->remote_ip,&ip,sizeof(ip));
  espconn_regist_connectcb(at_espconn_demo_espconn_ptr, at_espconn_demo_connect_cb);
  espconn_regist_reconcb(at_espconn_demo_espconn_ptr, at_espconn_demo_recon_cb);
  espconn_regist_disconcb(at_espconn_demo_espconn_ptr, at_espconn_demo_discon_cb);
  espconn_regist_recvcb(at_espconn_demo_espconn_ptr, at_espconn_demo_recv);
  espconn_regist_sentcb(at_espconn_demo_espconn_ptr, at_espconn_demo_send_cb);
  
  espconn_connect(at_espconn_demo_espconn_ptr);
  
  at_fake_uart_enable(TRUE,at_espconn_demo_response);
}

static void ICACHE_FLASH_ATTR
at_exeCmdTest(uint8_t id)
{
	at_response_ok();
	at_espconn_demo_init();
}

extern void at_exeCmdCiupdate(uint8_t id);
at_funcationType at_custom_cmd[] = {
    {"+TEST", 5, NULL, NULL, NULL, at_exeCmdTest},
#ifdef AT_UPGRADE_SUPPORT
    {"+CIUPDATE", 9,       NULL,            NULL,            NULL, at_exeCmdCiupdate}
#endif
};


void ICACHE_FLASH_ATTR user_init(void)
{
    char buf[64] = {0};
    at_customLinkMax = 5;
    at_init();
    os_sprintf(buf,"compile time:%s %s",__DATE__,__TIME__);
    at_set_custom_info(buf);
    at_port_print("\r\nready\r\n");
    at_cmd_array_regist(&at_custom_cmd[0], sizeof(at_custom_cmd)/sizeof(at_custom_cmd[0]));
	at_port_print("\r\n***==================================***");
	at_port_print("\r\n***  Welcome to at espconn demo!!!   ***");
	at_port_print("\r\n*** Please create a TCP Server on PC,***");
	at_port_print("\r\n*** then enter command AT+TEST.      ***");
	at_port_print("\r\n***==================================***\r\n");
}
