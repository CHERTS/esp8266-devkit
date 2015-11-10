#include "driver/slc_register.h"
#include "driver/sdio_slv.h"
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
//#include "gpio.h"
#include "user_interface.h"
#include "mem.h"

uint8 rx_buffer[1024],tx_buffer[1024];
os_event_t * sdioQueue;
struct sdio_queue rx_que,tx_que;

void sdio_slave_init(void)
{
	uint32 regval = 0;
	union sdio_slave_status sdio_sta;
	////reset orginal link
 	SET_PERI_REG_MASK(SLC_CONF0, SLC_RXLINK_RST|SLC_TXLINK_RST);
	CLEAR_PERI_REG_MASK(SLC_CONF0, SLC_RXLINK_RST|SLC_TXLINK_RST);

	os_printf("RX&TX link reset!\n");

	//set sdio mode
	SET_PERI_REG_MASK(SLC_RX_DSCR_CONF, SLC_RX_EOF_MODE | SLC_RX_FILL_MODE);
	//clear to host interrupt io signal for preventing from random initial signal.	   
	WRITE_PERI_REG(SLC_HOST_INTR_CLR, 0xffffffff);
	//enable 2 events to trigger the to host intr io
	SET_PERI_REG_MASK(SLC_HOST_INTR_ENA , SLC_HOST_TOHOST_BIT0_INT_ENA);
	////initialize rx queue information 
	rx_que.blocksize=1024;
	rx_que.datalen=0;
	rx_que.eof=1;
	rx_que.owner=1;
	rx_que.sub_sof=0;
	rx_que.unused=0;
	rx_que.buf_ptr=(uint32)rx_buffer;
	rx_que.next_link_ptr=0;
	////initialize tx queue information
	tx_que.blocksize=1024;
	tx_que.datalen=0;
	tx_que.eof=0;
	tx_que.owner=1;
	tx_que.sub_sof=0;
	tx_que.unused=0;
	tx_que.buf_ptr=(uint32)tx_buffer;
	tx_que.next_link_ptr=0;

	///////link tx&rx queue information address to sdio hardware
	CLEAR_PERI_REG_MASK(SLC_RX_LINK,SLC_RXLINK_DESCADDR_MASK);
 	regval= ((uint32)&rx_que);
	SET_PERI_REG_MASK(SLC_RX_LINK, regval&SLC_RXLINK_DESCADDR_MASK);
	CLEAR_PERI_REG_MASK(SLC_TX_LINK,SLC_TXLINK_DESCADDR_MASK);
	regval= ((uint32)&tx_que);
	SET_PERI_REG_MASK(SLC_TX_LINK, regval&SLC_TXLINK_DESCADDR_MASK);
	
	/////config sdio_status reg
	sdio_sta.elm_value.comm_cnt=7;
	sdio_sta.elm_value.intr_no=INIT_STAGE;
	sdio_sta.elm_value.wr_busy=0;
	sdio_sta.elm_value.rd_empty=1;
	sdio_sta.elm_value.rx_length=0;
	sdio_sta.elm_value.res=0;
	SET_PERI_REG_MASK(SLC_TX_LINK, SLC_TXLINK_START);
	WRITE_PERI_REG(SLC_HOST_CONF_W2, sdio_sta.word_value);


	/////attach isr func to sdio interrupt
	ETS_SLC_INTR_ATTACH(sdio_slave_isr, NULL);
	/////enable sdio operation intr
	WRITE_PERI_REG(SLC_INT_ENA,  SLC_INTEREST_EVENT);
	/////clear sdio initial random active intr signal 
	WRITE_PERI_REG(SLC_INT_CLR, 0xffffffff);
	/////enable sdio intr in cpu
	ETS_SLC_INTR_ENABLE();
}

void sdio_slave_isr(void *para)
{
	uint32 slc_intr_status,postval;
    	static uint8 state =0;
	uint16 rx_len,i;
	uint32* pword;
	union sdio_slave_status sdio_sta;
	
	slc_intr_status = READ_PERI_REG(SLC_INT_STATUS);

        if (slc_intr_status == 0) {
            /* No interested interrupts pending */
		return;
        }
	//clear all intrs
	WRITE_PERI_REG(SLC_INT_CLR, slc_intr_status);
	
	//process every intr

	//TO HOST DONE
	if (slc_intr_status & SLC_RX_EOF_INT_ENA) {
		//following code must be called after a data pack has been read
		rx_buff_read_done();
		TRIG_TOHOST_INT();
        }

        //FROM HOST DONE
        if (slc_intr_status & SLC_TX_EOF_INT_ENA) {
		//call the following function after host cpu data transmission finished 
		tx_buff_write_done();

		/************handle data**************/
		//get length
		rx_len=tx_que.datalen;

		//test code, check tx data
		for(i=0;i<rx_len;i++){
			if(tx_buffer[i]!=((uint8)i)){
				postval=(i)|(tx_buffer[i]<<16);
				system_os_post(USER_TASK_PRIO_1,SDIO_DATA_ERROR,postval);
			}
		}
		/*************************/
		
		//enable host cpu to send another data pack,should be called after tx data disposed
		tx_buff_handle_done();

		//***load rx data buffer***///
		for(i=0;i<rx_len;i++){
			rx_buffer[i]=tx_buffer[i]+tx_buffer[i]+tx_buffer[i];
		}
		//*********************//
		
		//enable host cpu to receive a new data pack, must be called after rx buffer reloaded
		rx_buff_load_done(rx_len);
		//trig to host intr io signal
		TRIG_TOHOST_INT();	
        }

        //TO HOST underflow
        if(slc_intr_status & SLC_RX_UDF_INT_ENA) {

        }

        //FROM HOST overflow
        if(slc_intr_status & SLC_TX_DSCR_ERR_INT_ENA) {

        }

}

void ICACHE_FLASH_ATTR
sdio_err_task(os_event_t *e)
{
	uint8 data;
    	switch(e->sig){
		case SDIO_DATA_ERROR:
			os_printf("Data ERR,data:%08x\n",e->par);
			break;

	       default:
	            break;
	}
}

void ICACHE_FLASH_ATTR
    sdio_task_init(void)
{
    sdioQueue = (os_event_t*)os_malloc(sizeof(os_event_t)*SDIO_QUEUE_LEN);
    system_os_task(sdio_err_task,USER_TASK_PRIO_1,sdioQueue,SDIO_QUEUE_LEN);
}

void rx_buff_read_done(void)
{
	union sdio_slave_status sdio_sta;
	/////modify sdio status reg
	sdio_sta.word_value=READ_PERI_REG(SLC_HOST_CONF_W2);
	sdio_sta.elm_value.comm_cnt++;
	sdio_sta.elm_value.rd_empty=1;
	sdio_sta.elm_value.rx_length=0;
	WRITE_PERI_REG(SLC_HOST_CONF_W2, sdio_sta.word_value);	//update sdio status register

}

void tx_buff_write_done(void)
{
	union sdio_slave_status sdio_sta;
	/////modify sdio status reg
	sdio_sta.word_value=READ_PERI_REG(SLC_HOST_CONF_W2);
	sdio_sta.elm_value.comm_cnt++;
	sdio_sta.elm_value.wr_busy=1;
	WRITE_PERI_REG(SLC_HOST_CONF_W2, sdio_sta.word_value);	//update sdio status register
}

void tx_buff_handle_done(void)
{
	union sdio_slave_status sdio_sta;
	/////config tx queue information
	tx_que.blocksize=1024;
	tx_que.datalen=0;
	tx_que.eof=0;
	tx_que.owner=1;
	/////modify sdio status reg
	sdio_sta.word_value=READ_PERI_REG(SLC_HOST_CONF_W2);
	sdio_sta.elm_value.wr_busy=0;
	sdio_sta.elm_value.intr_no=TX_AVAILIBLE;	

	SET_PERI_REG_MASK(SLC_TX_LINK, SLC_TXLINK_START);		//tx buffer is ready for being written
	WRITE_PERI_REG(SLC_HOST_CONF_W2, sdio_sta.word_value);	//update sdio status register
	//*******************************************************************//

}
void rx_buff_load_done(uint16 rx_len)
{
	union sdio_slave_status sdio_sta;
	/////config rx queue information
	rx_que.blocksize=1024;
	rx_que.datalen=rx_len;
	rx_que.eof=1;
	rx_que.owner=1;
	/////modify sdio status reg
	sdio_sta.word_value=READ_PERI_REG(SLC_HOST_CONF_W2);
	sdio_sta.elm_value.rd_empty=0;
	sdio_sta.elm_value.intr_no=RX_AVAILIBLE;
	sdio_sta.elm_value.rx_length=rx_len;	
	
	SET_PERI_REG_MASK(SLC_RX_LINK, SLC_RXLINK_START);		//rx buffer is ready for being read
	WRITE_PERI_REG(SLC_HOST_CONF_W2, sdio_sta.word_value);	//update sdio status register
}
