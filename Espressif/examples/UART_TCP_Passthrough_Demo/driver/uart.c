/******************************************************************************
 * Copyright 2013-2014 Espressif Systems
 *
 * FileName: uart.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2015/9/24, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "driver/uart.h"
#include "osapi.h"
#include "driver/uart_register.h"
#include "mem.h"
#include "user_interface.h"

#include "espconn.h"

#define FUNC_U1TXD_BK                   2
#define FUNC_U0CTS                             4
extern UartDevice    UartDev;


static struct UartBuffer *pTxBuffer = NULL;
static struct UartBuffer *pRxBuffer = NULL;

LOCAL void uart0_rx_intr_handler(void *para);

extern struct espconn *get_trans_conn(void);
void uart1_sendStr_no_wait(const char *str);
struct UartBuffer * Uart_Buf_Init(uint32 buf_size);
void set_tcp_block(void);
void clr_tcp_block(void);
void rx_buff_enq(void);
void tx_start_uart_buffer(uint8 uart_no);
void uart_rx_intr_disable(uint8 uart_no);
void uart_rx_intr_enable(uint8 uart_no);
/******************************************************************************
 * FunctionName : uart_config
 * Description  : Internal used function
 *                UART0 used for data TX/RX, RX buffer size is 0x100, interrupt enabled
 *                UART1 just used for debug output
 * Parameters   : uart_no, use UART0 or UART1 defined ahead
 * Returns      : NONE
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
uart_config(uint8 uart_no)
{
    if (uart_no == UART1) {
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
    } else {
        /* rcv_buff size if 0x100 */
        ETS_UART_INTR_ATTACH(uart0_rx_intr_handler,  &(UartDev.rcv_buff));
        PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_U0RTS);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_U0CTS);
        PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTCK_U);
    }

    uart_div_modify(uart_no, (uint16)(UART_CLK_FREQ / (uint32)(UartDev.baut_rate)));

    WRITE_PERI_REG(UART_CONF0(uart_no), (uint32)UartDev.exist_parity
            | (uint32)UartDev.parity
            | (((uint8)UartDev.stop_bits) << UART_STOP_BIT_NUM_S)
            | (((uint8)UartDev.data_bits) << UART_BIT_NUM_S));

    //clear rx and tx fifo,not ready
    SET_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);

    if (uart_no == UART0) {
        //set rx fifo trigger
        WRITE_PERI_REG(UART_CONF1(uart_no),
                ((100 & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S) |
                ((110 & UART_RX_FLOW_THRHD) << UART_RX_FLOW_THRHD_S) |
                UART_RX_FLOW_EN |
                (0x02 & UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S |
                UART_RX_TOUT_EN |

                ((0x10 & UART_TXFIFO_EMPTY_THRHD) << UART_TXFIFO_EMPTY_THRHD_S)); //wjl
                //SET_PERI_REG_MASK( UART_CONF0(uart_no),UART_TX_FLOW_EN);  //add this sentense to add a tx flow control via MTCK( CTS )

        SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_TOUT_INT_ENA |
                UART_FRM_ERR_INT_ENA);
    } else {
        WRITE_PERI_REG(UART_CONF1(uart_no),
                ((UartDev.rcv_buff.TrigLvl & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S));
    }

    //clear all interrupt
    WRITE_PERI_REG(UART_INT_CLR(uart_no), 0xffff);
    //enable rx_interrupt
    SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_FULL_INT_ENA | UART_RXFIFO_OVF_INT_ENA);
}

/******************************************************************************
 * FunctionName : uart1_tx_one_char
 * Description  : Internal used function
 *                Use uart1 interface to transfer one char
 * Parameters   : uint8 TxChar - character to tx
 * Returns      : OK
 *******************************************************************************/
STATUS
uart_tx_one_char(uint8 uart, uint8 TxChar)
{
    for(;;) {
        uint32 fifo_cnt = READ_PERI_REG(UART_STATUS(uart)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);

        if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126) {
            break;
        }
    }

    WRITE_PERI_REG(UART_FIFO(uart) , TxChar);
    return OK;
}

/******************************************************************************
 * FunctionName : uart1_write_char
 * Description  : Internal used function
 *                Do some special deal while tx char is '\r' or '\n'
 * Parameters   : char c - character to tx
 * Returns      : NONE
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
uart1_write_char(char c)
{
    if (c == '\n') {
        (void)uart_tx_one_char(UART1, '\r');
        (void)uart_tx_one_char(UART1, '\n');
    } else if (c == '\r') {
    } else {
        (void)uart_tx_one_char(UART1, (uint8)c);
    }
}

/******************************************************************************
 * FunctionName : uart0_rx_intr_handler
 * Description  : Internal used function
 *                UART0 interrupt handler, add self handle code inside
 * Parameters   : void *para - point to ETS_UART_INTR_ATTACH's arg
 * Returns      : NONE
 *******************************************************************************/
LOCAL void
uart0_rx_intr_handler(void *para)
{
    uint8 uart_no = UART0;
    if (UART_FRM_ERR_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_FRM_ERR_INT_ST)) {
        uart1_sendStr_no_wait("FRM_ERR\r\n");
        WRITE_PERI_REG(UART_INT_CLR(uart_no), UART_FRM_ERR_INT_CLR);
    }

    if (UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_FULL_INT_ST)) {
        uart_rx_intr_disable(uart_no);
        rx_buff_enq();
    } else if (UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_TOUT_INT_ST)) {
        uart_rx_intr_disable(uart_no);
        rx_buff_enq();
    } else if (UART_TXFIFO_EMPTY_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_TXFIFO_EMPTY_INT_ST)) {
        CLEAR_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_TXFIFO_EMPTY_INT_ENA);
        tx_start_uart_buffer(uart_no);
        WRITE_PERI_REG(UART_INT_CLR(uart_no), UART_TXFIFO_EMPTY_INT_CLR);
   } else if (UART_RXFIFO_OVF_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_OVF_INT_ST)) {
        uart1_sendStr_no_wait("RX OVF!\r\n");
        WRITE_PERI_REG(UART_INT_CLR(uart_no), UART_RXFIFO_OVF_INT_CLR);
    }
}

/******************************************************************************
 * FunctionName : uart_init
 * Description  : user interface for init uart
 * Parameters   : UartBautRate uart0_br - uart0 bautrate
 *                UartBautRate uart1_br - uart1 bautrate
 * Returns      : NONE
 *******************************************************************************/
void ICACHE_FLASH_ATTR
uart_init(UartBautRate uart0_br)
{
    UartDev.baut_rate = uart0_br;
    uart_config(UART0);
    UartDev.baut_rate = BIT_RATE_115200;
    uart_config(UART1);
    ETS_UART_INTR_ENABLE();

    os_install_putc1(uart1_write_char);

    pTxBuffer = Uart_Buf_Init(UART_TX_BUFFER_SIZE);
    pRxBuffer = Uart_Buf_Init(UART_RX_BUFFER_SIZE);
}

/******************************************************************************
 * FunctionName : uart_tx_one_char_no_wait
 * Description  : uart tx a single char without waiting for fifo
 * Parameters   : uint8 uart - uart port
 *                uint8 TxChar - char to tx
 * Returns      : STATUS
 *******************************************************************************/
STATUS
uart_tx_one_char_no_wait(uint8 uart, uint8 TxChar)
{
    uint8 fifo_cnt = ((READ_PERI_REG(UART_STATUS(uart)) >> UART_TXFIFO_CNT_S)& UART_TXFIFO_CNT);

    if (fifo_cnt < 126) {
        WRITE_PERI_REG(UART_FIFO(uart) , TxChar);
    }

    return OK;
}

/******************************************************************************
 * FunctionName : uart1_sendStr_no_wait
 * Description  : uart tx a string without waiting for every char, used for print debug info which can be lost
 * Parameters   : const char *str - string to be sent
 * Returns      : NONE
 *******************************************************************************/
void
uart1_sendStr_no_wait(const char *str)
{
    if(str == NULL) {
        return;
    }

    while (*str) {
        (void)uart_tx_one_char_no_wait(UART1, (uint8)*str);
        str++;
    }
}
/******************************************************************************
 * FunctionName : Uart_Buf_Init
 * Description  : tx buffer enqueue: fill a first linked buffer
 * Parameters   : char *pdata - data point  to be enqueue
 * Returns      : NONE
 *******************************************************************************/
struct UartBuffer *ICACHE_FLASH_ATTR
Uart_Buf_Init(uint32 buf_size)
{
    uint32 heap_size = system_get_free_heap_size();

    if(buf_size > 65535) { // now not support
        DBG1("no buf for uart\n\r");
        return NULL;
    }
    if (heap_size <= buf_size) {
        DBG1("no buf for uart\n\r");
        return NULL;
    } else {
        DBG("test heap size: %d\n\r", heap_size);
        struct UartBuffer *pBuff = (struct UartBuffer *)os_malloc((uint32)sizeof(struct UartBuffer));
        pBuff->UartBuffSize = (uint16)buf_size; // THIS OK
        pBuff->pUartBuff = (uint8 *)os_malloc(pBuff->UartBuffSize);
        pBuff->pInPos = pBuff->pUartBuff;
        pBuff->pOutPos = pBuff->pUartBuff;
        pBuff->Space = pBuff->UartBuffSize;
        pBuff->BuffState = OK;
        pBuff->nextBuff = NULL;
        //        pBuff->TcpControl = RUN;
        return pBuff;
    }
}

LOCAL void
Uart_Buf_Cpy(struct UartBuffer *pCur, const char *pdata , uint16 data_len)
{
    if ((pCur == NULL) || (pdata == NULL) || (data_len == 0)) {
        return ;
    }

    uint16 tail_len = (uint16)(pCur->pUartBuff + pCur->UartBuffSize - pCur->pInPos); // THIS OK

    if (tail_len >= data_len) { //do not need to loop back  the queue
        os_memcpy(pCur->pInPos , pdata , data_len);
        pCur->pInPos += (data_len);
        pCur->pInPos = (pCur->pUartBuff + (pCur->pInPos - pCur->pUartBuff) % pCur->UartBuffSize);
        pCur->Space -= data_len;
    } else {
        os_memcpy(pCur->pInPos, pdata, tail_len);
        pCur->pInPos += (tail_len);
        pCur->pInPos = (pCur->pUartBuff + (pCur->pInPos - pCur->pUartBuff) % pCur->UartBuffSize);
        pCur->Space -= tail_len;
        os_memcpy(pCur->pInPos, pdata + tail_len , data_len - tail_len);
        pCur->pInPos += (data_len - tail_len);
        pCur->pInPos = (pCur->pUartBuff + (pCur->pInPos - pCur->pUartBuff) % pCur->UartBuffSize);
        pCur->Space -= (data_len - tail_len);
    }

}

/******************************************************************************
 * FunctionName : uart_buf_free
 * Description  : deinit of the tx buffer
 * Parameters   : struct UartBuffer* pTxBuff - tx buffer struct pointer
 * Returns      : NONE
 *******************************************************************************/
void ICACHE_FLASH_ATTR
uart_buf_free(struct UartBuffer *pBuff)
{
    if(pBuff != NULL) {
        if(pBuff->pUartBuff != NULL) {
            os_free(pBuff->pUartBuff);
        }
        os_free(pBuff);
    }
}

uint16 ICACHE_FLASH_ATTR
rx_buff_get_data_len(void)
{
	uint16 buf_len = pRxBuffer->UartBuffSize - pRxBuffer->Space;
	return buf_len;
}

uint16 ICACHE_FLASH_ATTR
rx_buff_deq(char *pdata, uint16 data_len)
{
    uint16 len_tmp = 0;

    uint16 buf_len = pRxBuffer->UartBuffSize - pRxBuffer->Space;
    uint16 tail_len = (uint16)(pRxBuffer->pUartBuff + pRxBuffer->UartBuffSize - pRxBuffer->pOutPos); // THIS OK
    len_tmp = ((data_len > buf_len) ? buf_len : data_len);

    if (pRxBuffer->pOutPos <= pRxBuffer->pInPos) {
        if(pdata != NULL) {
            os_memcpy(pdata, pRxBuffer->pOutPos, len_tmp);
        }
        pRxBuffer->pOutPos += len_tmp;
        pRxBuffer->Space += len_tmp;
    } else {
        if (len_tmp > tail_len) {
            if(pdata != NULL) {
                os_memcpy(pdata, pRxBuffer->pOutPos, tail_len);
            }
            pRxBuffer->pOutPos += tail_len;
            pRxBuffer->pOutPos = (pRxBuffer->pUartBuff + (pRxBuffer->pOutPos - pRxBuffer->pUartBuff) % pRxBuffer->UartBuffSize);
            pRxBuffer->Space += tail_len;

            if(pdata != NULL) {
                os_memcpy(pdata + tail_len , pRxBuffer->pOutPos, len_tmp - tail_len);
            }
            pRxBuffer->pOutPos += (len_tmp - tail_len);
            pRxBuffer->pOutPos = (pRxBuffer->pUartBuff + (pRxBuffer->pOutPos - pRxBuffer->pUartBuff) % pRxBuffer->UartBuffSize);
            pRxBuffer->Space += (len_tmp - tail_len);
        } else {
            if(pdata != NULL) {
                os_memcpy(pdata, pRxBuffer->pOutPos, len_tmp);
            }
            pRxBuffer->pOutPos += len_tmp;
            pRxBuffer->pOutPos = (pRxBuffer->pUartBuff + (pRxBuffer->pOutPos - pRxBuffer->pUartBuff) % pRxBuffer->UartBuffSize);
            pRxBuffer->Space += len_tmp;
        }
    }

    // os_printf("recv:%d\r\n",pRxBuffer->Space);
    return len_tmp;
}

void
rx_buff_enq(void)
{
    uint8 fifo_len = 0, buf_idx = 0,loop;
    ETSParam par = 0;
    uint8 fifo_data;
    uint8* tail = (pRxBuffer->pUartBuff + pRxBuffer->UartBuffSize);
    fifo_len = (READ_PERI_REG(UART_STATUS(UART0)) >> UART_RXFIFO_CNT_S)&UART_RXFIFO_CNT;

    if(fifo_len > pRxBuffer->Space) {
        buf_idx = pRxBuffer->Space;
    } else {
        buf_idx = fifo_len;
    }
        
    loop = buf_idx;
    while (loop--) {
        *(pRxBuffer->pInPos++) = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
        if (pRxBuffer->pInPos == tail) {
            pRxBuffer->pInPos = pRxBuffer->pUartBuff;
        }
    }
        
    fifo_len -= buf_idx;
        
    while (fifo_len--) {
        fifo_data = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF; // discard data
    }

    pRxBuffer->Space -= buf_idx ;
    par = buf_idx;
    if(system_os_post(TRANS_TASK_PROI, (ETSSignal)TRANS_RECV_DATA_FROM_UART, par) != TRUE) {
        os_printf("post fail!!!\n\r");
    }
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR | UART_RXFIFO_TOUT_INT_CLR);
    uart_rx_intr_enable(UART0);
}

void ICACHE_FLASH_ATTR
tx_buff_enq(const char *pdata, uint16 data_len, bool force)
{
    CLEAR_PERI_REG_MASK(UART_INT_ENA(UART0), UART_TXFIFO_EMPTY_INT_ENA);
    //DBG2("len:%d\n\r",data_len);
    if(pdata != NULL) {
        if (pTxBuffer == NULL) {
            DBG1("\n\rnull, create buffer struct\n\r");
            pTxBuffer = Uart_Buf_Init(UART_TX_BUFFER_SIZE);

            if (pTxBuffer != NULL) {
                Uart_Buf_Cpy(pTxBuffer ,  pdata,  data_len);
            } else {
                DBG1("uart tx MALLOC no buf \n\r");
            }
        } else {
            if (data_len <= pTxBuffer->Space) {
                Uart_Buf_Cpy(pTxBuffer ,  pdata,  data_len);
            } else if (force) {
                for(;;) {
                    tx_start_uart_buffer(UART0);
                    CLEAR_PERI_REG_MASK(UART_INT_ENA(UART0), UART_TXFIFO_EMPTY_INT_ENA);
                    if (data_len <= pTxBuffer->Space) {
                        Uart_Buf_Cpy(pTxBuffer ,  pdata,  data_len);
                        break;
                    }
                    ets_delay_us(70);
                    WRITE_PERI_REG(0X60000914, 0x73);
                };
            } else {
                DBG1("UART TX BUF FULL!!!!\n\r");
            }
        }

        if ((pTxBuffer != NULL) && (pTxBuffer->Space <= URAT_TX_LOWER_SIZE)) {
            set_tcp_block();
        }
    }

    SET_PERI_REG_MASK(UART_CONF1(UART0), (UART_TX_EMPTY_THRESH_VAL & UART_TXFIFO_EMPTY_THRHD) << UART_TXFIFO_EMPTY_THRHD_S);
    SET_PERI_REG_MASK(UART_INT_ENA(UART0), UART_TXFIFO_EMPTY_INT_ENA);
}

//--------------------------------
LOCAL void tx_fifo_insert(struct UartBuffer *pTxBuff, uint8 data_len,  uint8 uart_no)
{
    uint8 i;

    if(pTxBuff == NULL) {
        return;
    }

    for (i = 0; i < data_len; i++) {
        WRITE_PERI_REG(UART_FIFO(uart_no) , *(pTxBuff->pOutPos++));

        if (pTxBuff->pOutPos == (pTxBuff->pUartBuff + pTxBuff->UartBuffSize)) {
            pTxBuff->pOutPos = pTxBuff->pUartBuff;
        }
    }

    pTxBuff->pOutPos = (pTxBuff->pUartBuff + (pTxBuff->pOutPos - pTxBuff->pUartBuff) % pTxBuff->UartBuffSize);
    pTxBuff->Space += data_len;
}

/******************************************************************************
 * FunctionName : tx_start_uart_buffer
 * Description  : get data from the tx buffer and fill the uart tx fifo, co-work with the uart fifo empty interrupt
 * Parameters   : uint8 uart_no - uart port num
 * Returns      : NONE
 *******************************************************************************/
void tx_start_uart_buffer(uint8 uart_no)
{
    uint8 tx_fifo_len = (READ_PERI_REG(UART_STATUS(uart_no)) >> UART_TXFIFO_CNT_S)&UART_TXFIFO_CNT;
    uint8 fifo_remain = UART_FIFO_LEN - tx_fifo_len ;
    uint8 len_tmp;
    uint32 data_len;

    if (pTxBuffer) {
        data_len = (pTxBuffer->UartBuffSize - pTxBuffer->Space);

        if (data_len > fifo_remain) {
            len_tmp = fifo_remain;
            tx_fifo_insert(pTxBuffer, len_tmp, uart_no);
            SET_PERI_REG_MASK(UART_INT_ENA(UART0), UART_TXFIFO_EMPTY_INT_ENA);
        } else {
            len_tmp = (uint8)data_len; // THIS OK
            tx_fifo_insert(pTxBuffer, len_tmp, uart_no);

        }

        if (pTxBuffer->Space >= URAT_TX_UPPER_SIZE) {
            (void)system_os_post(TRANS_TASK_PROI,(ETSSignal)TRANS_SEND_DATA_TO_UART_OVER,(ETSParam)0);
        }
    }
}

void uart_rx_intr_disable(uint8 uart_no)
{
    CLEAR_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_FULL_INT_ENA | UART_RXFIFO_TOUT_INT_ENA);
}

void uart_rx_intr_enable(uint8 uart_no)
{
    SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_FULL_INT_ENA | UART_RXFIFO_TOUT_INT_ENA);
}


void ICACHE_FLASH_ATTR
set_tcp_block(void)
{
    struct espconn * trans_conn = (struct espconn *)get_trans_conn();
    if(trans_conn == NULL) {

    } else if (trans_conn->type == ESPCONN_TCP) {
        DBG1("TCP BLOCK\n\r");
        (void)espconn_recv_hold(trans_conn);
        DBG2("b space: %d\n\r", pTxBuffer->Space);
    } else {

    }
}

void ICACHE_FLASH_ATTR
clr_tcp_block(void)
{
    struct espconn * trans_conn = (struct espconn *)get_trans_conn();
    if(trans_conn == NULL) {

    } else if (trans_conn->type == ESPCONN_TCP) {
        DBG1("TCP recover\n\r");
        (void)espconn_recv_unhold(trans_conn);
        DBG2("r space: %d\n\r", pTxBuffer->Space);
    } else {

    }

}
//========================================================
