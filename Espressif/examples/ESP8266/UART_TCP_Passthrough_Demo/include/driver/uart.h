/******************************************************************************
 * Copyright 2013-2014 Espressif Systems
 *
 * FileName: uart.h
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2015/9/24, v1.0 create this file.
*******************************************************************************/
#ifndef UART_APP_H
#define UART_APP_H

#include "uart_register.h"
#include "eagle_soc.h"
#include "c_types.h"

#define UART0   0
#define UART1   1

typedef enum {
    FIVE_BITS = 0x0,
    SIX_BITS = 0x1,
    SEVEN_BITS = 0x2,
    EIGHT_BITS = 0x3
} UartBitsNum4Char;

typedef enum {
    ONE_STOP_BIT             = 0x1,
    ONE_HALF_STOP_BIT        = 0x2,
    TWO_STOP_BIT             = 0x3
} UartStopBitsNum;

typedef enum {
    NONE_BITS = 0x2,
    ODD_BITS   = 1,
    EVEN_BITS = 0
} UartParityMode;

typedef enum {
    STICK_PARITY_DIS   = 0,
    STICK_PARITY_EN    = 1
} UartExistParity;

typedef enum {
    UART_None_Inverse = 0x0,
    UART_Rxd_Inverse = UART_RXD_INV,
    UART_CTS_Inverse = UART_CTS_INV,
    UART_Txd_Inverse = UART_TXD_INV,
    UART_RTS_Inverse = UART_RTS_INV,
} UART_LineLevelInverse;


typedef enum {
    BIT_RATE_300 = 300,
    BIT_RATE_600 = 600,
    BIT_RATE_1200 = 1200,
    BIT_RATE_2400 = 2400,
    BIT_RATE_4800 = 4800,
    BIT_RATE_9600   = 9600,
    BIT_RATE_19200  = 19200,
    BIT_RATE_38400  = 38400,
    BIT_RATE_57600  = 57600,
    BIT_RATE_74880  = 74880,
    BIT_RATE_115200 = 115200,
    BIT_RATE_230400 = 230400,
    BIT_RATE_460800 = 460800,
    BIT_RATE_921600 = 921600,
    BIT_RATE_1843200 = 1843200,
    BIT_RATE_3686400 = 3686400,
} UartBautRate;

typedef enum {
    NONE_CTRL,
    HARDWARE_CTRL,
    XON_XOFF_CTRL
} UartFlowCtrl;

typedef enum {
    USART_HardwareFlowControl_None = 0x0,
    USART_HardwareFlowControl_RTS = 0x1,
    USART_HardwareFlowControl_CTS = 0x2,
    USART_HardwareFlowControl_CTS_RTS = 0x3
} UART_HwFlowCtrl;

typedef enum {
    EMPTY,
    UNDER_WRITE,
    WRITE_OVER
} RcvMsgBuffState;

typedef struct {
    uint32     RcvBuffSize;
    uint8     *pRcvMsgBuff;
    uint8     *pWritePos;
    uint8     *pReadPos;
    uint8      TrigLvl; //JLU: may need to pad
    RcvMsgBuffState  BuffState;
} RcvMsgBuff;

typedef struct {
    uint32   TrxBuffSize;
    uint8   *pTrxBuff;
} TrxMsgBuff;

typedef enum {
    BAUD_RATE_DET,
    WAIT_SYNC_FRM,
    SRCH_MSG_HEAD,
    RCV_MSG_BODY,
    RCV_ESC_CHAR,
} RcvMsgState;

typedef struct {
    UartBautRate 	     baut_rate;
    UartBitsNum4Char  data_bits;
    UartExistParity      exist_parity;
    UartParityMode 	    parity;    // chip size in byte
    UartStopBitsNum   stop_bits;
    UartFlowCtrl         flow_ctrl;
    RcvMsgBuff          rcv_buff;
    TrxMsgBuff           trx_buff;
    RcvMsgState        rcv_state;
    int                      received;
    int                      buff_uart_no;  //indicate which uart use tx/rx buffer
} UartDevice;

void uart_init(UartBautRate uart0_br);
///////////////////////////////////////
#define UART_FIFO_LEN  128  //define the tx fifo length
//
#define UART_TX_EMPTY_THRESH_VAL 0x10
#define UART_TX_BUFFER_SIZE 8192
#define URAT_TX_LOWER_SIZE 1024*7
#define URAT_TX_UPPER_SIZE  8000*1

#define UART_RX_BUFFER_SIZE  (10*1024) //201

//#define UART_TX_DEBUG
#define DBG(...)
#define DBG1(...) //uart1_sendStr_no_wait
#define DBG2(...) //os_printf

struct UartBuffer {
    uint16     UartBuffSize;
    uint8     *pUartBuff;
    uint8     *pInPos;
    uint8     *pOutPos;
    STATUS  BuffState;
    uint16    Space;  //remanent space of the buffer
//    uint8  TcpControl;
    struct  UartBuffer       *nextBuff;
};

struct UartRxBuff {
    uint32     UartRxBuffSize;
    uint8     *pUartRxBuff;
    uint8     *pWritePos;
    uint8     *pReadPos;
    STATUS RxBuffState;
    uint32    Space;  //remanent space of the buffer
} ;

void tx_buff_enq(const char *pdata, uint16 data_len, bool force);
uint16 rx_buff_deq(char *pdata, uint16 data_len);
//==============================================
#define FUNC_UART0_CTS 4
#define UART_LINE_INV_MASK  (0x3f<<19)

#endif

