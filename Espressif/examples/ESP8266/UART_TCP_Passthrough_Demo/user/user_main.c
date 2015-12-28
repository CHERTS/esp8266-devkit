/******************************************************************************
 * Copyright 2013-2014 Espressif Systems
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2015/9/24, v1.0 create this file.
*******************************************************************************/

#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "driver/uart.h"
#include "mem.h"

#define TRANS_SSID		"TT-DEMO"
#define TRANS_QUEUE_LEN		20
static os_event_t trans_Queue[TRANS_QUEUE_LEN];
static struct espconn *pTcpServer;
static struct espconn *pTcpClient;
#define	TRANS_BUF_SIZE	2920
static uint8 trans_buf[TRANS_BUF_SIZE];
static uint32 trans_buf_count = 0;
static uint8 trans_wait_write = 0;
static uint8 trans_pending_count = 0;
static os_timer_t trans_interval_timer;

#define TRANS_TIMER_INTERVAL    20

struct espconn *ICACHE_FLASH_ATTR
get_trans_conn(void)
{
    return pTcpClient;
}

static void ICACHE_FLASH_ATTR
trans_tcpclient_recv(void *arg, char *pdata, unsigned short len)
{
    tx_buff_enq(pdata, len, TRUE);
}

static void ICACHE_FLASH_ATTR
trans_tcpclient_recon_cb(void *arg, sint8 err)
{
	system_os_post(TRANS_TASK_PROI,TRANS_CLIENT_DISCONNECTED,0);
}

static void ICACHE_FLASH_ATTR
trans_tcpclient_discon_cb(void *arg)
{
	system_os_post(TRANS_TASK_PROI,TRANS_CLIENT_DISCONNECTED,0);
}

static void ICACHE_FLASH_ATTR
trans_tcpclient_sent_cb(void *arg)
{

}

static void ICACHE_FLASH_ATTR
trans_tcpclient_write_cb(void *arg)
{
	os_timer_disarm(&trans_interval_timer);
	trans_buf_count = 0;
	trans_wait_write = 0;
	if(trans_pending_count) {
		system_os_post(TRANS_TASK_PROI, (ETSSignal)TRANS_RECV_DATA_FROM_UART, trans_pending_count);
		trans_pending_count = 0;
	}
	os_timer_arm(&trans_interval_timer, (uint32_t)TRANS_TIMER_INTERVAL, 0);
}

static void ICACHE_FLASH_ATTR
trans_server_listen(void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;

    if(pespconn == NULL) {
        return;
    }
    
    espconn_regist_recvcb(pespconn, trans_tcpclient_recv);
    espconn_regist_reconcb(pespconn, trans_tcpclient_recon_cb);
    espconn_regist_disconcb(pespconn, trans_tcpclient_discon_cb);
    espconn_regist_sentcb(pespconn, trans_tcpclient_sent_cb);///////
    espconn_regist_write_finish(pespconn, trans_tcpclient_write_cb);
    espconn_set_opt(pespconn,ESPCONN_COPY);
    system_os_post(TRANS_TASK_PROI,TRANS_CLIENT_CONNECTED,(ETSParam)pespconn);
    return;
}

void ICACHE_FLASH_ATTR
transmit_data(const void *arg)
{
	os_timer_disarm(&trans_interval_timer);
	if (trans_wait_write == 1) {
		return;
	}

	if(trans_buf_count != 0) {
		if(espconn_sent(pTcpClient, trans_buf, trans_buf_count) != ESPCONN_OK) {
			return;
		}
		trans_wait_write = 1;
	}
	os_timer_arm(&trans_interval_timer, (uint32_t)TRANS_TIMER_INTERVAL, 0);
}

static void ICACHE_FLASH_ATTR ///////
trans_task(os_event_t *events)
{
	static uint8 state = 0;
	int32 ret = 0;
	uint32 data_len = 0;

	if(events == NULL) {
		return;
	}

	switch(events->sig) {
	case TRANS_CLIENT_CONNECTED:    // tcp connected
		os_timer_arm(&trans_interval_timer, (uint32_t)TRANS_TIMER_INTERVAL, 0);
		pTcpClient = (struct espconn *)events->par;
		state = 2;	// Now we can transparently transmit data
		break;
	case TRANS_CLIENT_DISCONNECTED: // tcp disconnected
		os_timer_disarm(&trans_interval_timer);
		pTcpClient = NULL;
		state = 3;	// If we receive data via uart,we just discard it
		trans_pending_count = 0;
		trans_buf_count = 0;
		trans_wait_write = 0;
		break;
	case TRANS_RECV_DATA_FROM_UART: 
		data_len = rx_buff_get_data_len();
		if(state != 2) {
			rx_buff_deq(NULL,data_len);
			os_printf("state %d\r\n",state);
			return;
		}
		os_timer_disarm(&trans_interval_timer);
		uint32 trans_buf_free = TRANS_BUF_SIZE - trans_buf_count;

		if(trans_wait_write == 1) { // now tcp is using buffer,we must wait
			trans_pending_count += data_len;
			return;
		}

		if(trans_buf_free > data_len) {
			rx_buff_deq(&trans_buf[trans_buf_count],data_len);  // get data from uart buffer
			trans_buf_count += data_len;
		} else {
			rx_buff_deq(&trans_buf[trans_buf_count],trans_buf_free);
			trans_buf_count = TRANS_BUF_SIZE;
			trans_pending_count += data_len - trans_buf_free;
			if(espconn_send(pTcpClient,trans_buf,trans_buf_count) != ESPCONN_OK) {  // send data
				os_printf("espconn_send fail\r\n");
				return;
			} else {
				trans_wait_write = 1;
			}
		}

		os_timer_arm(&trans_interval_timer, (uint32_t)TRANS_TIMER_INTERVAL, 0);
		break;
	case TRANS_SEND_DATA_TO_UART_OVER:
		clr_tcp_block();
		break;
	default:
		os_printf("sig %d\r\n",events->sig);
		break;
	}
}

void ICACHE_FLASH_ATTR wifi_event_handler_cb(System_Event_t * event)
{
    uint8 loop = 0;
    static uint8 status = EVENT_MAX;
    if(event == NULL) {
        return;
    }
    switch(event->event) {
    case EVENT_SOFTAPMODE_STACONNECTED:
    	os_printf("EVENT_SOFTAPMODE_STACONNECTED\r\n"); // station connected
        break;
    case EVENT_SOFTAPMODE_STADISCONNECTED:
    	os_printf("EVENT_SOFTAPMODE_STADISCONNECTED\r\n"); // station disconnected
    	break;
    default:
        break;
    }
}

void user_init(void)
{
    struct softap_config apConfig;
    uart_init(500000);
    
    wifi_set_opmode(SOFTAP_MODE);  // softap mode
    
    // set softap config
    os_memset(&apConfig,0x0,sizeof(apConfig));
    apConfig.authmode = AUTH_OPEN;
    apConfig.channel = 0;
    apConfig.max_connection = 1;
    apConfig.ssid_len = os_strlen(TRANS_SSID);
    os_strncpy(apConfig.ssid,TRANS_SSID,os_strlen(TRANS_SSID));
    wifi_softap_set_config(&apConfig);
    
    wifi_set_event_handler_cb(wifi_event_handler_cb);   // monitor wifi state
    system_os_task(trans_task, TRANS_TASK_PROI, trans_Queue, TRANS_QUEUE_LEN);  // create a task that processes transparently transmitting data

    // create tcp server
    pTcpServer = (struct espconn *)os_zalloc((uint32)sizeof(struct espconn));

    if (pTcpServer == NULL) {
    	os_printf("TcpServer Failure\r\n");
    	return;
    }

    pTcpServer->type = ESPCONN_TCP;
    pTcpServer->state = ESPCONN_NONE;
    pTcpServer->proto.tcp = (esp_tcp *)os_zalloc((uint32)sizeof(esp_tcp));
    pTcpServer->proto.tcp->local_port = 12000;      // server port
    espconn_regist_connectcb(pTcpServer, trans_server_listen);
    espconn_accept(pTcpServer);
    espconn_regist_time(pTcpServer, 180, 0);
    
    os_timer_disarm(&trans_interval_timer);
    os_timer_setfn(&trans_interval_timer, (os_timer_func_t *)transmit_data, NULL);
}
