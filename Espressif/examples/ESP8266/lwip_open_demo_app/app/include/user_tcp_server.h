#ifndef __USER_TCP_SERVER__
#define __USER_TCP_SERVER__

#include "user_config.h"

#ifndef __ESPCONN_H__
typedef sint8 err_t;
enum espconn_state {
    ESPCONN_NONE,
    ESPCONN_WAIT,  // ожидает закрытия
    ESPCONN_LISTEN, // соединение открыто, ждет rx
    ESPCONN_CONNECT, // соединение открыто, было rx  или tx
    ESPCONN_WRITE,
    ESPCONN_READ,
    ESPCONN_CLOSE // соединение закрывается
};
#endif

#ifndef TCP_SRV_MAX_CONNECTIONS
 #define TCP_SRV_MAX_CONNECTIONS 7
#endif

#ifndef TCP_SRV_SERVER_PORT
 #define TCP_SRV_SERVER_PORT 80
#endif

#ifndef DEBUGSOO
 #define DEBUGSOO 3
#endif

#ifndef TCP_SRV_RECV_WAIT
 #define TCP_SRV_RECV_WAIT  5
#endif

#ifndef TCP_SRV_END_WAIT
 #define TCP_SRV_END_WAIT  5
#endif

#define TCP_SRV_CLOSE_WAIT 120 // 120/4 = 30 сек

#define TCP_SRV_MIN_HEAP_SIZE 14528  // самый минимум от 6Kb,
// надо иметь от TCP WIN * 2 + дескрипторы в памяти (1560*4*2=12480) + 2Kb = 14528

//--------------------------------------------------------------------------
// Структура соединения
//
typedef struct _tcpsrv_conn_flags {
	uint16 throttle_data_reception: 1; //
	uint16 close_recv_sent: 1; //
}tcpsrv_conn_flags;

typedef struct _TCP_SERV_CONN {
	struct _tcpsrv_conn_flags flag;  // флаги
	uint16 state;    	// pcb state
	struct _TCP_SERV_CFG *pcfg;      // указатель на базу
	uint16 recv_check;   // счет секунд в tcpsrv_pool
	uint16 remote_port;  // номер порта клиента
	union {              // ip номер клиента
	  uint8  b[4];
	  uint32 dw;
	}remote_ip;
	struct _TCP_SERV_CONN *next; // указатель на следующую структуру
	struct tcp_pcb *pcb; // pcb
	os_timer_t ptimer;  // use tcpsrv_close_cb
	uint32 cntr; 	// ипользуется sent data
	uint8 *ptrbuf; // buf ипользуется sent data
	uint16 unrecved_bytes; // используется при ручном управлении TCP WIN / This can be used to throttle data reception
} TCP_SERV_CONN;


//--------------------------------------------------------------------------
// Вызываемые функции
//
typedef void (*func_disconect_calback)(TCP_SERV_CONN *ts_conn); // соединение закрыто
typedef err_t (*func_listen)(TCP_SERV_CONN *hconn); // новый клиент
typedef err_t (*func_received_data)(TCP_SERV_CONN *ts_conn, uint8 *pusrdata, uint16 length); // принято length байт, лежат в буфере по pusrdata
typedef err_t (*func_sent_callback)(TCP_SERV_CONN *ts_conn); // блок данных передан

typedef struct _tcpsrv_cfg_flags {
	uint16 none: 1; // это пока
}tcpsrv_cfg_flags;

//--------------------------------------------------------------------------
// Структура конфигурации tcp сервера
//
typedef struct _TCP_SERV_CFG {
        tcpsrv_cfg_flags flg;      // флаги конфигурации (пока не задействован)
        uint16 port;      	 // номер порта, при = 0 заменяется на 80.
        uint16 max_conn;  	 // максимальное кол-во одновременных соединений, при = 0 заменяется на 5.
        uint16 conn_count;       // кол-во текущих соединений, при инициализации прописывает 0
        uint16 min_heap; 	 // минимальный размер heap при открытии нового соединения, при = 0 заменяется на 8192.
        uint16 time_wait_rec;    // время (сек) ожидания запроса от клиента, до авто-закрытия соединения, при = 0 заменяется на 5 сек.
        uint16 time_wait_cls;    // время (сек) до авто-закрытия соединения после приема или передачи, при = 0 заменяется на 5 сек.
        TCP_SERV_CONN * conn_links;  // указатель на цепочку активных соединений, при инициализации или отсуствии активных соединений = NULL
        struct tcp_pcb *pcb;     // начальный pcb [LISTEN]
        func_disconect_calback func_discon_cb; // функция вызываемая после закрытия соединения, если = NULL - не вызывается
        func_listen func_listen; // функция вызываемая при присоединении клиента, если = NULL - не вызывается
        func_sent_callback func_sent_cb; // функция вызываемая после передачи данных или наличию места в ip стеке для следушей передачи данных
        func_received_data func_recv; // функция вызываемая при приеме данных
        struct _TCP_SERV_CFG *next; // следующий экземпляр
}TCP_SERV_CFG;

//--------------------------------------------------------------------------
// Данные
//
extern TCP_SERV_CFG *phcfg; // указатель на цепочку TCP_SERV_CFG (стартовавших серверов)

//--------------------------------------------------------------------------
// Функции
//
err_t tcpsrv_sent_data(TCP_SERV_CONN * ts_conn, uint8 *psent, uint16 length) ICACHE_FLASH_ATTR; // передать length байт
void tcpsrv_disconnect(TCP_SERV_CONN * ts_conn) ICACHE_FLASH_ATTR;
void tcpsrv_print_remote_info(TCP_SERV_CONN *ts_conn) ICACHE_FLASH_ATTR; // выводит remote_ip:remote_port [conn_count] os_printf("srv x.x.x.x:x [n] ")
TCP_SERV_CFG * hcfg_find_port(uint16 portn) ICACHE_FLASH_ATTR; // поиск конфига по номеру порта
void tcpsrv_unrecved_win(TCP_SERV_CONN *ts_conn) ICACHE_FLASH_ATTR; // Восстановить размер TCP WIN, если используется ручное управление размером окна TCP

TCP_SERV_CFG *tcpsrv_init(uint16 portn);
bool tcpsrv_start(TCP_SERV_CFG *p);
bool tcpsrv_close(TCP_SERV_CFG *p);

#endif
