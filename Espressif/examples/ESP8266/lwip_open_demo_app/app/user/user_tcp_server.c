/*
 * Реализация простейшего TCP-сервера
 * Автор: pvvx
 *
 * Simple TCP-server
 * Author: pvvx
 *
 */

#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "user_interface.h"

#include "lwip/netif.h"
#include "lwip/inet.h"
#include "netif/etharp.h"
#include "lwip/tcp.h"
#include "lwip/ip.h"
#include "lwip/init.h"
#include "lwip/tcp_impl.h"
#include "lwip/memp.h"

#include "lwip/mem.h"
#include "lwip/app/espconn.h"
#include "lwip/app/espconn_tcp.h"
#include "lwip/app/espconn_udp.h"

#include "user_tcp_server.h"

TCP_SERV_CFG *phcfg = NULL;

// пред.описание...
static void ICACHE_FLASH_ATTR tcpsrv_list_delete(TCP_SERV_CONN * ts_conn);
static void ICACHE_FLASH_ATTR tcpsrv_disconnect_successful(TCP_SERV_CONN * ts_conn);
static void ICACHE_FLASH_ATTR tcpsrv_close_cb(TCP_SERV_CONN * ts_conn);
static void ICACHE_FLASH_ATTR tcpsrv_server_close(TCP_SERV_CONN * ts_conn);
static err_t ICACHE_FLASH_ATTR tcpsrv_server_poll(void *arg, struct tcp_pcb *pcb);
static void ICACHE_FLASH_ATTR tcpsrv_server_err(void *arg, err_t err);
TCP_SERV_CFG * ICACHE_FLASH_ATTR hcfg_find_port(uint16 portn);

/******************************************************************************
 * FunctionName : tcpsrv_print_remote_info
 * Description  : выводит remote_ip:remote_port [conn_count] ets_uart_printf("srv x.x.x.x:x [n] ")
 * Parameters   : TCP_SERV_CONN * ts_conn
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR tcpsrv_print_remote_info(TCP_SERV_CONN *ts_conn)
{
	ets_uart_printf("srv[%u] " IPSTR ":%d [%d] ", ts_conn->pcfg->port, ts_conn->remote_ip.b[0], ts_conn->remote_ip.b[1], ts_conn->remote_ip.b[2], ts_conn->remote_ip.b[3], ts_conn->remote_port, ts_conn->pcfg->conn_count);
}

void ICACHE_FLASH_ATTR tcpsrv_disconnect_calback_default(TCP_SERV_CONN *ts_conn)
{
#if DEBUG_LEVEL > 1
	tcpsrv_print_remote_info(ts_conn);
	ets_uart_printf("disconnect\n");
#endif
}

err_t ICACHE_FLASH_ATTR tcpsrv_listen_default(TCP_SERV_CONN *ts_conn)
{
#if DEBUG_LEVEL > 1
	tcpsrv_print_remote_info(ts_conn);
	ets_uart_printf("listen\n");
#endif
	return ERR_OK;
}

err_t ICACHE_FLASH_ATTR tcpsrv_sent_callback_default(TCP_SERV_CONN *ts_conn)
{
#if DEBUG_LEVEL > 1
	tcpsrv_print_remote_info(ts_conn);
	ets_uart_printf("sent_cb\n");
#endif
	return ERR_OK;
}

err_t ICACHE_FLASH_ATTR tcpsrv_received_data_default(TCP_SERV_CONN *ts_conn, uint8 *data_ptr, uint16 data_cntr)
{
#if DEBUG_LEVEL > 1
	tcpsrv_print_remote_info(ts_conn);
	ets_uart_printf("received %d bytes\n", data_cntr);
#endif
	return ERR_OK;
}

/******************************************************************************
 * FunctionName : tcpsrv_unrecved_win
 * Description  : Update the TCP window.
 * This can be used to throttle data reception (e.g. when received data is
 * programmed to flash and data is received faster than programmed).
 * Parameters   : TCP_SERV_CONN * ts_conn
 * Returns      : none
 * После включения throttle, будет принято до 5840 (MAX WIN) + 1460 (MSS) байт?
*******************************************************************************/
void tcpsrv_unrecved_win(TCP_SERV_CONN *ts_conn)
{
	if((ts_conn->flag.throttle_data_reception)&&(ts_conn->unrecved_bytes != 0)) {
		// update the TCP window
#if DEBUG_LEVEL > 3
		ets_uart_printf("recved_bytes=%d\n", ts_conn->unrecved_bytes);
#endif
		tcp_recved(ts_conn->pcb, ts_conn->unrecved_bytes);
		ts_conn->unrecved_bytes = 0;
	}
}

/******************************************************************************
 * FunctionName : find_pcb
 * Description  : поиск pcb в списках lwip
 * Parameters   : TCP_SERV_CONN * ts_conn
 * Returns      : *pcb or NULL
*******************************************************************************/
struct tcp_pcb * ICACHE_FLASH_ATTR find_tcpsrv_pcb(TCP_SERV_CONN * ts_conn)
{
	struct tcp_pcb *pcb;
	uint16 remote_port = ts_conn->remote_port;
	uint16 local_port = ts_conn->pcfg->port;
	uint32 ip = ts_conn->remote_ip.dw;
	for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) {
	  if((pcb->remote_port == remote_port)&&(pcb->local_port == local_port)&&(pcb->remote_ip.addr == ip)) return pcb;
	}
	for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) {
	  if((pcb->remote_port == remote_port)&&(pcb->local_port == local_port)&&(pcb->remote_ip.addr == ip)) return pcb;
	}
	return NULL;
}

/******************************************************************************
 * FunctionName : tcpsrv_disconnect
 * Description  : disconnect
 * Parameters   :
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR tcpsrv_disconnect(TCP_SERV_CONN * ts_conn)
{
    if (ts_conn == NULL || ts_conn->state == ESPCONN_WAIT) return;
    ts_conn->pcb = find_tcpsrv_pcb(ts_conn);
    if(ts_conn->pcb != NULL) tcpsrv_server_close(ts_conn);
}

/******************************************************************************
 * FunctionName : tcpsrv_sent_data
 * Description  : sent data for server
 * Parameters   : void *arg -- client or server to send
 *                uint8* psent -- Data to send
 *                uint16 length -- Length of data to send
 * Returns      : none
*******************************************************************************/
err_t ICACHE_FLASH_ATTR tcpsrv_sent_data(TCP_SERV_CONN * ts_conn, uint8 *psent, uint16 length)
{
    if (ts_conn == NULL || psent == NULL || length == 0) {
        return ERR_ARG;
    }
    //if(ts_conn->state == ESPCONN_WAIT) return;
    struct tcp_pcb *pcb = ts_conn->pcb;
    err_t err;
    u16_t len = length;

    if (tcp_sndbuf(pcb) < length)  len = tcp_sndbuf(pcb);
    if(len) {
    	u16_t mss2 = (tcp_mss(pcb)<< 1); // <<2 ???!!!
    	if(len > mss2) len = mss2;
    	do {
    		err = tcp_write(pcb, psent, len, 0);
    		if (err == ERR_MEM) len /= 2;
    	} while (err == ERR_MEM && len > 1);
    	if (err == ERR_OK) {
    		ts_conn->ptrbuf = psent + len;
    		ts_conn->cntr = length - len;
    		err = tcp_output(pcb);
    	}
    	else ts_conn->state = ESPCONN_CLOSE;
    }
    else return tcp_output(pcb);
    return err;
}

/******************************************************************************
 * FunctionName : tcpsrv_server_sent
 * Description  : Data has been sent and acknowledged by the remote host.
 * This means that more data can be sent.
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pcb -- The connection pcb for which data has been acknowledged
 *                len -- The amount of bytes acknowledged
 * Returns      : ERR_OK: try to send some data by calling tcp_output
 *                ERR_ABRT: if you have called tcp_abort from within the function!
*******************************************************************************/
static err_t ICACHE_FLASH_ATTR tcpsrv_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
	sint8 ret_err = ERR_OK;
	TCP_SERV_CONN * ts_conn = arg;
    if (ts_conn == NULL || pcb == NULL) {
        return ERR_ARG;
    }
    //if(ts_conn->state == ESPCONN_WAIT) return ret_err;
    ts_conn->pcb = pcb;
	ts_conn->recv_check = 0;
    if (ts_conn->cntr == 0) {
    	ts_conn->state = ESPCONN_CONNECT;
    	if((ts_conn->flag.close_recv_sent==0)&&(ts_conn->pcfg->func_sent_cb != NULL)) ret_err = ts_conn->pcfg->func_sent_cb(ts_conn);
    }
    else ret_err = tcpsrv_sent_data(ts_conn, ts_conn->ptrbuf, ts_conn->cntr);
    return ret_err;
}

/******************************************************************************
 * FunctionName : tcpsrv_server_recv
 * Description  : Data has been received on this pcb.
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pcb -- The connection pcb which received data
 *                p -- The received data (or NULL when the connection has been closed!)
 *                err -- An error code if there has been an error receiving
 * Returns      : ERR_ABRT: if you have called tcp_abort from within the function!
*******************************************************************************/
static err_t ICACHE_FLASH_ATTR tcpsrv_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	// Sets the callback function that will be called when new data arrives on the connection associated with pcb.
	// The callback function will be passed a NULL pbuf to indicate that the remote host has closed the connection.
	TCP_SERV_CONN * ts_conn = arg;
	sint8 ret_err = err;
    	if (ts_conn == NULL) return ERR_ARG;

    	ts_conn->pcb = pcb;
    	if (p == NULL || err != ERR_OK) { // the remote host has closed the connection.
    		tcpsrv_server_close(ts_conn);
    		return ERR_OK;
    	}
        if (ts_conn->flag.throttle_data_reception) { // throttle data reception?
        	ts_conn->unrecved_bytes += p->tot_len;
        }
        else tcp_recved(pcb, p->tot_len); // сообщает стеку, что можно посылать ACK и принимать новые данные.

        if(ts_conn->state != ESPCONN_WAIT) {
    		ts_conn->state = ESPCONN_CONNECT;
    		ts_conn->recv_check = 0;
    		u8_t *data_ptr = (u8_t *)os_zalloc(p->tot_len + 1);
            if(data_ptr == NULL) {
#if DEBUG_LEVEL > 2
            	tcpsrv_print_remote_info(ts_conn);
    	        ets_uart_printf("recv err_mem heap=%d\n", system_get_free_heap_size());
#endif
            	pbuf_free(p);
        		return ERR_MEM;
            }
    		u32_t data_cntr = pbuf_copy_partial(p, data_ptr, p->tot_len, 0);
        	pbuf_free(p);
        	if (data_cntr != 0) {
        		if((ts_conn->flag.close_recv_sent == 0)&&(ts_conn->pcfg->func_recv != NULL)) ret_err = ts_conn->pcfg->func_recv(ts_conn, data_ptr, data_cntr);
        	}
        	os_free(data_ptr);
    	}
    	return err;
}

/******************************************************************************
 * FunctionName : tcpsrv_disconnect
 * Description  : disconnect with host
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
static void ICACHE_FLASH_ATTR tcpsrv_disconnect_successful(TCP_SERV_CONN * ts_conn)
{
	sint8 dis_err = 0;
    if (ts_conn == NULL) return;
    struct tcp_pcb *pcb = ts_conn->pcb;
    if(pcb != NULL) {
    	tcp_arg(pcb, NULL);
    	tcp_err(pcb, NULL);
    }
  	ts_conn->state = ESPCONN_CLOSE;
	if(ts_conn->pcfg->func_discon_cb != NULL) ts_conn->pcfg->func_discon_cb(ts_conn);
	// remove the node from the server's connection list
	tcpsrv_list_delete(ts_conn);
}

/******************************************************************************
 * FunctionName : tcpsrv_closed
 * Description  : The connection has been successfully closed.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
static void ICACHE_FLASH_ATTR tcpsrv_close_cb(TCP_SERV_CONN * ts_conn)
{
    if (ts_conn == NULL) {
        return;
    }
#if DEBUG_LEVEL > 3
    ets_uart_printf("close_cb %p ", ts_conn->pcb);
#endif
    struct tcp_pcb *pcb = find_tcpsrv_pcb(ts_conn); // ts_conn->pcb; Часто уже закрыта
#if DEBUG_LEVEL > 3
    ets_uart_printf("%p\n", pcb);
#endif
    if(pcb == NULL || pcb->state == CLOSED || pcb->state == TIME_WAIT) {
    	ts_conn->pcb = pcb;
		ts_conn->state = ESPCONN_CLOSE;
		/*remove the node from the server's active connection list*/
		tcpsrv_disconnect_successful(ts_conn);
    }
    else {
        if (++ts_conn->recv_check > TCP_SRV_CLOSE_WAIT) { // ts_conn->pcfg->time_wait_cls * 2 + 4) {
#if DEBUG_LEVEL > 2
        	tcpsrv_print_remote_info(ts_conn);
        	ets_uart_printf("tcp_abandon!\n");
#endif
            tcp_poll(pcb, NULL, 0);
            tcp_abandon(pcb, 0);
            ts_conn->recv_check = 0;
        	ts_conn->pcb = pcb;
    		ts_conn->state = ESPCONN_CLOSE;
    		/*remove the node from the server's active connection list*/
    		tcpsrv_disconnect_successful(ts_conn);
        }
        else os_timer_arm(&ts_conn->ptimer, TCP_FAST_INTERVAL, 0); // 250 ms
    }
}

/******************************************************************************
 * FunctionName : tcpsrv_server_close
 * Description  : The connection shall be actively closed.
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pcb -- the pcb to close
 * Returns      : none
*******************************************************************************/
static void ICACHE_FLASH_ATTR tcpsrv_server_close(TCP_SERV_CONN * ts_conn)
{
    	err_t err;
    	struct tcp_pcb *pcb = ts_conn->pcb;
//    	if(ts_conn->state != ESPCONN_WAIT) {
    		ts_conn->state = ESPCONN_WAIT;
    		ts_conn->recv_check = 0;
    		os_timer_disarm(&ts_conn->ptimer);
    		tcp_recv(pcb, NULL);
    		err = tcp_close(pcb);
    		// The function may return ERR_MEM if no memory was available for closing the connection.
    		// If so, the application should wait and try again either by using the acknowledgment callback or the polling functionality.
    		// If the close succeeds, the function returns ERR_OK.
    		os_timer_setfn(&ts_conn->ptimer, tcpsrv_close_cb, ts_conn);
    		os_timer_arm(&ts_conn->ptimer, TCP_FAST_INTERVAL, 0);  // 250 ms

    		if (err != ERR_OK) {
#if DEBUG_LEVEL > 2
    			tcpsrv_print_remote_info(ts_conn);
    			ets_uart_printf("+ncls+\n", pcb);
#endif
    			// closing failed, try again later
    			tcp_arg(pcb, ts_conn);
    			tcp_recv(pcb, tcpsrv_server_recv);  // восстановить работу? ждем FIN от remote?
/*                      // Set up the various callback functions
                        tcp_err(pcb, tcpsrv_server_err);
                        tcp_sent(pcb, tcpsrv_server_sent);
                        tcp_recv(pcb, tcpsrv_server_recv);
                        tcp_poll(pcb, tcpsrv_server_poll, 4); // every 1 seconds */
    		} else {
    			/* closing succeeded */
    			tcp_poll(pcb, NULL, 0);
    			tcp_sent(pcb, NULL);
    		}
//    	}
}

/******************************************************************************
 * FunctionName : espconn_server_poll
 * Description  : The poll function is called every 3nd second.
 * If there has been no data sent (which resets the retries) in 3 seconds, close.
 * If the last portion of a file has not been sent in 3 seconds, close.
 *
 * This could be increased, but we don't want to waste resources for bad connections.
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pcb -- The connection pcb for which data has been acknowledged
 * Returns      : ERR_OK: try to send some data by calling tcp_output
 *                ERR_ABRT: if you have called tcp_abort from within the function!
*******************************************************************************/
static err_t ICACHE_FLASH_ATTR tcpsrv_server_poll(void *arg, struct tcp_pcb *pcb)
{
	TCP_SERV_CONN * ts_conn = arg;
    if (ts_conn == NULL) {
        tcp_abandon(pcb, 0);
        tcp_poll(pcb, NULL, 0);
        return ERR_OK;
    }
#if DEBUG_LEVEL > 3
	tcpsrv_print_remote_info(ts_conn);
    ets_uart_printf("poll %d #%d p:%u\n", ts_conn->recv_check, pcb->state, ts_conn->remote_port);
#endif
	if(ts_conn->state != ESPCONN_WAIT) {
		ts_conn->pcb = pcb;
		if (pcb->state == ESTABLISHED) {
			if((ts_conn->state == ESPCONN_LISTEN)&&(++ts_conn->recv_check > ts_conn->pcfg->time_wait_rec)) tcpsrv_server_close(ts_conn);
			if((ts_conn->state == ESPCONN_CONNECT)&&(++ts_conn->recv_check > ts_conn->pcfg->time_wait_cls)) tcpsrv_server_close(ts_conn);
		}
		else tcpsrv_server_close(ts_conn);
	}
	else tcp_poll(pcb, NULL, 0);
    return ERR_OK;
}

/******************************************************************************
 * FunctionName : tcpsrv_list_delete
 * Description  : remove the node from the connection list
 * Parameters   : ts_conn
 * Returns      : none
*******************************************************************************/
static void ICACHE_FLASH_ATTR tcpsrv_list_delete(TCP_SERV_CONN * ts_conn)
{
	TCP_SERV_CONN ** p = &ts_conn->pcfg->conn_links;
	TCP_SERV_CONN *tcpsrv_cmp = ts_conn->pcfg->conn_links;
	if(ts_conn == NULL) return;
	while(tcpsrv_cmp != NULL) {
		if(tcpsrv_cmp == ts_conn) {
			*p = ts_conn->next;
			ts_conn->pcfg->conn_count--;
			return; // break;
		}
		p = &tcpsrv_cmp->next;
		tcpsrv_cmp = tcpsrv_cmp->next;
	}
}

/******************************************************************************
 * FunctionName : tcpsrv_server_err
 * Description  : The pcb had an error and is already deallocated.
 *		The argument might still be valid (if != NULL).
 * Parameters   : arg -- Additional argument to pass to the callback function
 *		err -- Error code to indicate why the pcb has been closed
 * Returns      : none
*******************************************************************************/
#if DEBUG_LEVEL > 1
const char *srvContenErr[] =
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
//  см esp_iot_sdk\libs\lwip\api\err.c , зависит от версии lwip
};
#endif

static void ICACHE_FLASH_ATTR tcpsrv_server_err(void *arg, err_t err)
{
  TCP_SERV_CONN * ts_conn = arg;
  struct tcp_pcb *pcb = NULL;
	if (ts_conn != NULL) {
#if DEBUG_LEVEL > 1
		tcpsrv_print_remote_info(ts_conn);
		ets_uart_printf("error %d (%s)\n", err, ((err>-16)&&(err<1))? srvContenErr[-err] : "?" );
#endif
		if(ts_conn->state != ESPCONN_WAIT) {
		   ts_conn->state = ESPCONN_CLOSE;
		   /*remove the node from the server's connection list*/
	       tcpsrv_list_delete(ts_conn);
		}
    }
}

/******************************************************************************
 * FunctionName : tcpsrv_tcp_accept
 * Description  : A new incoming connection has been accepted.
 * Parameters   : arg -- Additional argument to pass to the callback function
 *		pcb -- The connection pcb which is accepted
 *		err -- An unused error code, always ERR_OK currently
 * Returns      : acception result
*******************************************************************************/
static err_t ICACHE_FLASH_ATTR tcpsrv_server_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
  struct tcp_pcb_listen *lpcb = (struct tcp_pcb_listen*)arg;

  TCP_SERV_CFG *p = hcfg_find_port(pcb->local_port);
  if(p == NULL) return ERR_ARG;

  if(system_get_free_heap_size() < p->min_heap) {
#if DEBUG_LEVEL > 1
	  ets_uart_printf("srv[%u] new listen - lov heap size!\n", p->port);
#endif
	  return ERR_MEM;
  }
  if(p->conn_count >= p->max_conn) {
#if DEBUG_LEVEL > 1
	  ets_uart_printf("srv[%u] new listen - max connection!\n", p->port);
#endif
	  return ERR_CONN;
  }
  TCP_SERV_CONN * ts_conn = (TCP_SERV_CONN *)os_zalloc(sizeof(TCP_SERV_CONN));
  if(ts_conn == NULL) {
#if DEBUG_LEVEL > 1
	  ets_uart_printf("srv[%u] new listen - out of mem!\n", ts_conn->pcfg->port);
#endif
	  return ERR_MEM;
  }
  ts_conn->pcfg = p;
  tcp_accepted(lpcb); // Decrease the listen backlog counter
//  tcp_setprio(pcb, TCP_PRIO_MIN); // Set priority ?
// init/copy data ts_conn
  ts_conn->pcb = pcb;
  ts_conn->remote_port = pcb->remote_port;
  //  ts_conn->remote_ip.dw = pcb->remote_ip.addr;
  ts_conn->remote_ip.b[0] = ip4_addr1_16(&pcb->remote_ip);
  ts_conn->remote_ip.b[1] = ip4_addr2_16(&pcb->remote_ip);
  ts_conn->remote_ip.b[2] = ip4_addr3_16(&pcb->remote_ip);
  ts_conn->remote_ip.b[3] = ip4_addr4_16(&pcb->remote_ip);
  ts_conn->state = ESPCONN_LISTEN;
  *(uint16 *)&ts_conn->flag = 0;
  ts_conn->recv_check = 0;
  // insert new ts_conn
  ts_conn->next = ts_conn->pcfg->conn_links;
  ts_conn->pcfg->conn_links = ts_conn;
  ts_conn->pcfg->conn_count++;

  // Tell TCP that this is the structure we wish to be passed for our callbacks.
  tcp_arg(pcb, ts_conn);
  // Set up the various callback functions
  tcp_err(pcb, tcpsrv_server_err);
  tcp_sent(pcb, tcpsrv_server_sent);
  tcp_recv(pcb, tcpsrv_server_recv);
  tcp_poll(pcb, tcpsrv_server_poll, 4); /* every 1 seconds */
  if(p->func_listen != NULL) return p->func_listen(ts_conn);
  return ERR_OK;
}
/******************************************************************************
 * FunctionName : hcfg_find_port
 * Description  : поиск сонфига по порту
 * Parameters   : номер порта
 * Returns      : указатель на TCP_SERV_CFG или NULL
*******************************************************************************/
TCP_SERV_CFG * ICACHE_FLASH_ATTR hcfg_find_port(uint16 portn)
{
	TCP_SERV_CFG * p;
	for(p  = phcfg; p != NULL; p = p->next) if(p->port == portn) return p;
  	return NULL;
}

/******************************************************************************
 tcpsrv_init
*******************************************************************************/
TCP_SERV_CFG * tcpsrv_init(uint16 portn)
{
  if(portn == 0) portn = 80;
#if DEBUG_LEVEL > 0
  ets_uart_printf("TCP Server init on port %u - ", portn);
#endif
  TCP_SERV_CFG * p;
  for(p  = phcfg; p != NULL; p = p->next) {
    if(p->port == portn) {
#if DEBUG_LEVEL > 0
    	ets_uart_printf("already initialized!\n");
#endif
    	return NULL;
    }
  }
  p = (TCP_SERV_CFG *)os_zalloc(sizeof(TCP_SERV_CFG));
  if(p == NULL) {
    ets_uart_printf("out of memory!\n");
    return NULL;
  }
  p->port = portn;
  p->max_conn = TCP_SRV_MAX_CONNECTIONS;
  p->conn_count = 0;
  p->min_heap = TCP_SRV_MIN_HEAP_SIZE;
  p->time_wait_rec = TCP_SRV_RECV_WAIT;
  p->time_wait_cls = TCP_SRV_END_WAIT;
// p->phcfg->conn_links = NULL; // zalloc
// p->pcb = NULL; // zalloc
  p->func_discon_cb = tcpsrv_disconnect_calback_default;
  p->func_listen = tcpsrv_listen_default;
  p->func_sent_cb = tcpsrv_sent_callback_default;
  p->func_recv = tcpsrv_received_data_default;
#if DEBUG_LEVEL > 0
  ets_uart_printf("Ok\n");
#endif
  return p;
}

/******************************************************************************
 tcpsrv_start
*******************************************************************************/
bool tcpsrv_start(TCP_SERV_CFG *p)
{
#if DEBUG_LEVEL > 0
	ets_uart_printf("TCP Server start - ");
#endif
	if(p == NULL) {
#if DEBUG_LEVEL > 0
		ets_uart_printf("NULL pointer!\n");
#endif
		return false;
	}
	if(p->pcb != NULL){
#if DEBUG_LEVEL > 0
    	ets_uart_printf("already running!\n");
#endif
    	return false;
	}
	p->pcb = tcp_new();
	if(p->pcb != NULL) {
		err_t err = tcp_bind(p->pcb, IP_ADDR_ANY, p->port); // Binds pcb to a local IP address and port number.
		if(err == ERR_OK) { // If another connection is bound to the same port, the function will return ERR_USE, otherwise ERR_OK is returned.
			p->pcb = tcp_listen(p->pcb); // Commands pcb to start listening for incoming connections.
			// When an incoming connection is accepted, the function specified with the tcp_accept() function
			// will be called. pcb must have been bound to a local port with the tcp_bind() function.
			// The tcp_listen() function returns a new connection identifier, and the one passed as an
			// argument to the function will be deallocated. The reason for this behavior is that less
			// memory is needed for a connection that is listening, so tcp_listen() will reclaim the memory
			// needed for the original connection and allocate a new smaller memory block for the listening connection.
			// tcp_listen() may return NULL if no memory was available for the listening connection.
			// If so, the memory associated with pcb will not be deallocated.
			if(p->pcb != NULL) {
				tcp_arg(p->pcb, p->pcb);
				// insert new tcpsrv_config
				p->next = phcfg;
				phcfg = p;
				// initialize callback arg and accept callback
				tcp_accept(p->pcb, tcpsrv_server_accept);
#if DEBUG_LEVEL > 0
				ets_uart_printf("Ok\n");
#endif
				return true;
			}
		}
		tcp_abandon(p->pcb, 0);
		p->pcb = NULL;
	}
#if DEBUG_LEVEL > 0
	ets_uart_printf("failed!\n");
#endif
	return false;
}

/******************************************************************************
 tcpsrv_close
*******************************************************************************/
bool tcpsrv_close(TCP_SERV_CFG *p)
{
#if DEBUG_LEVEL > 0
   ets_uart_printf("TCP Server closed - ");
#endif
   if(p != NULL) {
#if DEBUG_LEVEL > 0
	   ets_uart_printf("NULL pointer!\n");
#endif
	   return false;
   }
   TCP_SERV_CONN * ts_conn = p->conn_links;
   while(p->conn_links != NULL) {
     ts_conn->pcb = find_tcpsrv_pcb(ts_conn);
     if(ts_conn->pcb != NULL) {
       tcp_abort(ts_conn->pcb);
     }
     ts_conn->state = ESPCONN_CLOSE;
     if(phcfg->func_discon_cb != NULL) phcfg->func_discon_cb(ts_conn);
     tcpsrv_list_delete(ts_conn);
     ts_conn = p->conn_links;
   }
   tcp_abandon(p->pcb, 0);
	TCP_SERV_CFG **pwr = &phcfg;
	TCP_SERV_CFG *pcmp = phcfg;
	while(pcmp != NULL) {
		if(pcmp == p) {
			*pwr = p->next;
			os_free(p);
#if DEBUG_LEVEL > 0
			ets_uart_printf("Ok\n");
#endif
			return true; // break;
		}
		pwr = &pcmp->next;
		pcmp = pcmp->next;
	}
#if DEBUG_LEVEL > 0
   ets_uart_printf("no find!\n");
#endif
   return false;
}
