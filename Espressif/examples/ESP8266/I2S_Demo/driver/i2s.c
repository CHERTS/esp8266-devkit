#include "driver/spi.h"
#include "driver/spi_overlap.h"
#include "driver/i2s_reg.h"
#include "driver/slc_register.h"
#include "driver/sdio_slv.h"
#include "driver/i2s.h"
#include "osapi.h"
#include "os_type.h"
#include "gpio.h"
#include "pin_mux_register.h"


#define i2c_bbpll                                 0x67
#define i2c_bbpll_en_audio_clock_out            4
#define i2c_bbpll_en_audio_clock_out_msb        7
#define i2c_bbpll_en_audio_clock_out_lsb        7
#define i2c_bbpll_hostid                           4

#define i2c_writeReg_Mask(block, host_id, reg_add, Msb, Lsb, indata)  rom_i2c_writeReg_Mask(block, host_id, reg_add, Msb, Lsb, indata)
#define i2c_readReg_Mask(block, host_id, reg_add, Msb, Lsb)  rom_i2c_readReg_Mask(block, host_id, reg_add, Msb, Lsb)
#define i2c_writeReg_Mask_def(block, reg_add, indata) \
      i2c_writeReg_Mask(block, block##_hostid,  reg_add,  reg_add##_msb,  reg_add##_lsb,  indata)
#define i2c_readReg_Mask_def(block, reg_add) \
      i2c_readReg_Mask(block, block##_hostid,  reg_add,  reg_add##_msb,  reg_add##_lsb)

#define IIS_RX_BUF_LEN  512  //unit Byte
#define IIS_TX_BUF_LEN  512  //unit Byte
#define n  2

#define RX_NUM 128 //unit word
//#define 

uint32 i2s_rx_buff1[IIS_RX_BUF_LEN/4];
uint32 i2s_rx_buff2[IIS_RX_BUF_LEN/4];
uint32 i2s_tx_buff1[IIS_TX_BUF_LEN/4];
uint32 i2s_tx_buff2[IIS_TX_BUF_LEN/4];
uint32 i2s_tx_buff3[IIS_TX_BUF_LEN/4];

uint32 i2s_tx_test[IIS_TX_BUF_LEN*n];



int rx_buff1_cnt=0;
int rx_buff2_cnt=0;
int tx_cnt=0;


uint32 triang_tab1[IIS_RX_BUF_LEN/2];
uint32 triang_tab2[IIS_RX_BUF_LEN/2];

int8 tab_idx=0;

struct sdio_queue i2s_rx_queue1, i2s_rx_queue2,i2s_tx_queue1,i2s_tx_queue2,i2s_tx_queue3;

//create fake audio data
void generate_data()
{

	uint16 i;
	uint32 val;
	val = 0x100;

   //generate data	

		for(i=0;i<256;i++){
			triang_tab1[i]=val;
			val+=0x100;
			
		}
		for(;i<512;i++){
		triang_tab2[i-256]=val;
		val+=0x100;
			
		}

}

//load data into buffer
void load_buffer1_1(uint32* buffer,uint32 buff_len)
{
	uint32 i;
	uint32* pbuff=buffer;

	for(i=0;i<buff_len;i++){
		*pbuff=triang_tab1[i];
		pbuff++;
	}
}

void load_buffer1_2(uint32* buffer,uint32 buff_len)
{
	uint32 i;
	uint32* pbuff=buffer;

	for(i=0;i<buff_len;i++){
		*pbuff=triang_tab1[buff_len+i];
		pbuff++;
	}
}

void load_buffer2_1(uint32* buffer,uint32 buff_len)
{
	uint32 i;
	uint32* pbuff=buffer;

	for(i=0;i<buff_len;i++){
		*pbuff=triang_tab2[i];
		pbuff++;
	}
}

void load_buffer2_2(uint32* buffer,uint32 buff_len)
{
	uint32 i;
	uint32* pbuff=buffer;

	for(i=0;i<buff_len;i++){
		*pbuff=triang_tab2[buff_len+i];
		pbuff++;
	}
}

//create DMA buffer descriptors, unit of either size or length here is byte. 
//More details in I2S documents.
void creat_one_link(uint8 own, uint8 eof,uint8 sub_sof, uint16 size, uint16 length,
                       uint32* buf_ptr, struct sdio_queue* nxt_ptr, struct sdio_queue* i2s_queue)
 {
       unsigned int link_data0;
       unsigned int link_data1;
       unsigned int link_data2;
       unsigned int start_addr;

	i2s_queue->owner=own;
	i2s_queue->eof= eof;
	i2s_queue->sub_sof=sub_sof;
	i2s_queue->datalen=length;
	i2s_queue->blocksize=size;
	i2s_queue->buf_ptr=(uint32)buf_ptr;
	i2s_queue->next_link_ptr=(uint32)nxt_ptr;
	i2s_queue->unused=0;
}


//Initialize the I2S module
//More Registor details in I2S documents.
void i2s_init()
{


    //CONFIG I2S RX PIN FUNC
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_I2SI_DATA);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_I2SI_BCK);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_I2SI_WS);
	
	//CONFIG I2S TX PIN FUNC
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_I2SO_DATA);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_I2SO_WS);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_I2SO_BCK);

    //Enable a 160MHz clock to i2s subsystem
	i2c_writeReg_Mask_def(i2c_bbpll, i2c_bbpll_en_audio_clock_out, 1);

	//reset I2S
	CLEAR_PERI_REG_MASK(I2SCONF,I2S_I2S_RESET_MASK);
	SET_PERI_REG_MASK(I2SCONF,I2S_I2S_RESET_MASK);
	CLEAR_PERI_REG_MASK(I2SCONF,I2S_I2S_RESET_MASK);

	
	//Enable FIFO in i2s module
	SET_PERI_REG_MASK(I2S_FIFO_CONF, I2S_I2S_DSCR_EN);
	
	//set I2S_FIFO
	//set rx,tx data size, both are "24-bit full data discountinue" here
	SET_PERI_REG_MASK(I2S_FIFO_CONF, (I2S_I2S_RX_FIFO_MOD<<I2S_I2S_RX_FIFO_MOD_S)|(I2S_I2S_TX_FIFO_MOD<<I2S_I2S_TX_FIFO_MOD_S));

	//set I2S_CHAN 
	//set rx,tx channel mode, both are "two channel" here
	SET_PERI_REG_MASK(I2SCONF_CHAN, (I2S_TX_CHAN_MOD<<I2S_TX_CHAN_MOD_S)|(I2S_RX_CHAN_MOD<<I2S_RX_CHAN_MOD_S));

	//set RX eof num
	WRITE_PERI_REG(I2SRXEOF_NUM, RX_NUM);

	
	//trans master&rece slave mode,
	//MSB_shift,right_first,MSB_right,
	//use I2S clock divider to produce a 32KHz Sample Rate
	CLEAR_PERI_REG_MASK(I2SCONF, I2S_TRANS_SLAVE_MOD|
						(I2S_BITS_MOD<<I2S_BITS_MOD_S)|
						(I2S_BCK_DIV_NUM <<I2S_BCK_DIV_NUM_S)|
                                    	(I2S_CLKM_DIV_NUM<<I2S_CLKM_DIV_NUM_S));
	
	SET_PERI_REG_MASK(I2SCONF, I2S_RIGHT_FIRST|I2S_MSB_RIGHT|I2S_RECE_SLAVE_MOD|
						I2S_RECE_MSB_SHIFT|I2S_TRANS_MSB_SHIFT|
						(( 26&I2S_BCK_DIV_NUM )<<I2S_BCK_DIV_NUM_S)|
						((4&I2S_CLKM_DIV_NUM)<<I2S_CLKM_DIV_NUM_S)|
						(8<<I2S_BITS_MOD_S));
   /*
   
    //trans slave&rece master mode,
    //MSB_shift,right_first,MSB_right,
    //use I2S clock divider to produce a 32KHz Sample Rate
	CLEAR_PERI_REG_MASK(I2SCONF, I2S_RECE_SLAVE_MOD|
						(I2S_BITS_MOD<<I2S_BITS_MOD_S)|
						(I2S_BCK_DIV_NUM <<I2S_BCK_DIV_NUM_S)|
                                    	(I2S_CLKM_DIV_NUM<<I2S_CLKM_DIV_NUM_S));
	
	SET_PERI_REG_MASK(I2SCONF, I2S_RIGHT_FIRST|I2S_MSB_RIGHT|I2S_TRANS_SLAVE_MOD|
						I2S_RECE_MSB_SHIFT|I2S_TRANS_MSB_SHIFT|
						(( 26&I2S_BCK_DIV_NUM )<<I2S_BCK_DIV_NUM_S)|
						((4&I2S_CLKM_DIV_NUM)<<I2S_CLKM_DIV_NUM_S)|
						(8<<I2S_BITS_MOD_S));

	*/


	
	//clear int
	SET_PERI_REG_MASK(I2SINT_CLR,   I2S_I2S_TX_REMPTY_INT_CLR|I2S_I2S_TX_WFULL_INT_CLR|I2S_I2S_RX_REMPTY_INT_CLR|
	I2S_I2S_RX_WFULL_INT_CLR|I2S_I2S_PUT_DATA_INT_CLR|I2S_I2S_TAKE_DATA_INT_CLR);
	CLEAR_PERI_REG_MASK(I2SINT_CLR,   I2S_I2S_TX_REMPTY_INT_CLR|I2S_I2S_TX_WFULL_INT_CLR|I2S_I2S_RX_REMPTY_INT_CLR|
	I2S_I2S_RX_WFULL_INT_CLR|I2S_I2S_PUT_DATA_INT_CLR|I2S_I2S_TAKE_DATA_INT_CLR);

	//enable int
	SET_PERI_REG_MASK(I2SINT_ENA,   I2S_I2S_TX_REMPTY_INT_ENA|I2S_I2S_TX_WFULL_INT_ENA|I2S_I2S_RX_REMPTY_INT_ENA|
	I2S_I2S_RX_WFULL_INT_ENA|I2S_I2S_TX_PUT_DATA_INT_ENA|I2S_I2S_RX_TAKE_DATA_INT_ENA);

	

	//Start transmitter and receiver
	SET_PERI_REG_MASK(I2SCONF,I2S_I2S_TX_START|I2S_I2S_RX_START);

}

//Initialize the SLC module for DMA function
void slc_init()
{
    //Reset DMA
	SET_PERI_REG_MASK(SLC_CONF0, SLC_RXLINK_RST|SLC_TXLINK_RST);
	CLEAR_PERI_REG_MASK(SLC_CONF0, SLC_RXLINK_RST|SLC_TXLINK_RST);

	//Enable and configure DMA
	CLEAR_PERI_REG_MASK(SLC_CONF0, (SLC_MODE<<SLC_MODE_S));
	SET_PERI_REG_MASK(SLC_CONF0,(1<<SLC_MODE_S));
	SET_PERI_REG_MASK(SLC_RX_DSCR_CONF,SLC_INFOR_NO_REPLACE|SLC_TOKEN_NO_REPLACE);//|0xfe
	CLEAR_PERI_REG_MASK(SLC_RX_DSCR_CONF, SLC_RX_FILL_EN|SLC_RX_EOF_MODE | SLC_RX_FILL_MODE);
	
	
	ETS_SLC_INTR_ATTACH(slc_isr, NULL);
	/////enable sdio operation intr
	WRITE_PERI_REG(SLC_INT_ENA, SLC_INTEREST_EVENT);
	/////clear sdio initial random active intr signal 
	WRITE_PERI_REG(SLC_INT_CLR, 0xffffffff);
	/////enable sdio intr in cpu
	ETS_SLC_INTR_ENABLE();
}

 
//interrupt
//write new buff data into RX_DMA at the transmitter side
//or load buff data out from TX_DMA at the receiver side
void slc_isr(void *para)
{
	uint32 slc_intr_status;

	slc_intr_status = READ_PERI_REG(SLC_INT_STATUS);

	

        if (slc_intr_status == 0) {
           //No interested interrupts pending  
		return;
        }
	
	//clear all intrs
	WRITE_PERI_REG(SLC_INT_CLR, 0xffffffff);
	
	//process every intr

	//Transimitter side
	if (slc_intr_status & SLC_RX_EOF_INT_ST) {

		//find the DMA which sends the interrupt signal
        if(READ_PERI_REG(SLC_RX_EOF_DES_ADDR)==(((uint32)&i2s_rx_queue1))){
	
            //replace data in the buff
			if ((rx_buff1_cnt%2)==0) {			
			load_buffer2_1(i2s_rx_buff1,IIS_RX_BUF_LEN/4);
				}
			else if ((rx_buff1_cnt%2)==1) {			
			load_buffer1_1(i2s_rx_buff1,IIS_RX_BUF_LEN/4);
				}
			rx_buff1_cnt++;
		}

        else if(READ_PERI_REG(SLC_RX_EOF_DES_ADDR)==(((uint32)&i2s_rx_queue2))){

				

			if ((rx_buff2_cnt%2)==0) {	
					
			load_buffer2_2(i2s_rx_buff2,IIS_RX_BUF_LEN/4);
				}
			else if ((rx_buff2_cnt%2)==1) {			
			load_buffer1_2(i2s_rx_buff2,IIS_RX_BUF_LEN/4);
				}
 			rx_buff2_cnt++;
		}

		}

	//Receiver side
	if (slc_intr_status & SLC_TX_EOF_INT_ST) {
	
        		
        //find the DMA which sends the interrupt signal
		if(READ_PERI_REG(SLC_TX_EOF_DES_ADDR)==(((uint32)&i2s_tx_queue1))){

		//load out data in the buff	
		if(tx_cnt < 4*n){
		os_memcpy((uint8*)i2s_tx_test+IIS_TX_BUF_LEN*tx_cnt,(uint8*)i2s_tx_buff1,IIS_TX_BUF_LEN);
			}

		//reset DMA discrpiter
		i2s_tx_queue1.next_link_ptr = (uint32)(&i2s_tx_queue2);
		i2s_tx_queue1.eof=0;
		i2s_tx_queue1.owner=1;
		i2s_tx_queue1.datalen = 0;
				}
		
		else if(READ_PERI_REG(SLC_TX_EOF_DES_ADDR)==(((uint32)&i2s_tx_queue2))){
		
		if(tx_cnt < 4*n){
		os_memcpy((uint8*)i2s_tx_test+IIS_TX_BUF_LEN*tx_cnt,(uint8*)i2s_tx_buff2,IIS_TX_BUF_LEN);
			}


		i2s_tx_queue2.next_link_ptr = (uint32)(&i2s_tx_queue3);
		i2s_tx_queue2.eof=0;
		i2s_tx_queue2.owner=1;
		i2s_tx_queue2.datalen = 0;
				}

		
		else if(READ_PERI_REG(SLC_TX_EOF_DES_ADDR)==(((uint32)&i2s_tx_queue3))){
		

		if(tx_cnt < 4*n){
		os_memcpy((uint8*)i2s_tx_test+IIS_TX_BUF_LEN*tx_cnt,(uint8*)i2s_tx_buff3,IIS_TX_BUF_LEN);
			}

		i2s_tx_queue3.next_link_ptr = (uint32)(&i2s_tx_queue1);
		i2s_tx_queue3.eof=0;
		i2s_tx_queue3.owner=1;
		i2s_tx_queue3.datalen =0;
				}

		
        tx_cnt++;
		}

		
	
}


void ICACHE_FLASH_ATTR
	i2s_test(void)
{
	uint32 i;

	generate_data();
	load_buffer1_1(i2s_rx_buff1,IIS_RX_BUF_LEN/4);
	load_buffer1_2(i2s_rx_buff2,IIS_RX_BUF_LEN/4);
	
	slc_init();

	//To receive data from the I2S module, counter-intuitively we use the TXLINK part, not the RXLINK part.
	//Vice versa.
	//Note:At the transimitter side,the size of the DMAs can not be smaller than 128*4 bytes which are the
	//size of the I2S FIFO.
	//Note:At the receiver side,the number of the DMAs can not be smaller than 3 which is limited by the 
	//hardware.
	creat_one_link(1,0,0,IIS_TX_BUF_LEN,0,i2s_tx_buff1,&i2s_tx_queue2,&i2s_tx_queue1); 
	creat_one_link(1,0,0,IIS_TX_BUF_LEN,0,i2s_tx_buff2,&i2s_tx_queue3,&i2s_tx_queue2); 
 	creat_one_link(1,0,0,IIS_TX_BUF_LEN,0,i2s_tx_buff3,&i2s_tx_queue1,&i2s_tx_queue3); 

	creat_one_link(1,1,0,IIS_RX_BUF_LEN,IIS_RX_BUF_LEN,i2s_rx_buff1,&i2s_rx_queue2,&i2s_rx_queue1); 
	creat_one_link(1,1,0,IIS_RX_BUF_LEN,IIS_RX_BUF_LEN,i2s_rx_buff2,&i2s_rx_queue1,&i2s_rx_queue2); 

	os_printf("================DMA descripter built==============\r\n");

	os_printf("==========debug RX descripter for I2S transmitter========\r\n");
	os_printf("i2s_rx_queue1_address: %08x\r\n",&i2s_rx_queue1);
	os_printf("i2s_rx_queue1_word0:%08x\n\r",READ_PERI_REG(&i2s_rx_queue1));
	os_printf("i2s_rx_queue1_buf_ptr: %08x\r\n",i2s_rx_queue1.buf_ptr);
	os_printf("i2s_rx_queue1_next_link_ptr: %08x\r\n",i2s_rx_queue1.next_link_ptr);
	os_printf("\r\n");
	os_printf("i2s_rx_queue2: %08x\r\n",&i2s_rx_queue2);
	os_printf("i2s_rx_queue2_word0:%08x\n\r",READ_PERI_REG(&i2s_rx_queue2));
	os_printf("i2s_rx_queue2_buf_ptr: %08x\r\n",i2s_rx_queue2.buf_ptr);
	os_printf("i2s_rx_queue2_next_link_ptr: %08x\r\n",i2s_rx_queue2.next_link_ptr);
	os_printf("--------------------------------------------------\r\n");

	os_printf("==========debug TX descripter for I2S receiver===========\r\n");
	os_printf("i2s_tx_queue1_address: %08x\r\n",&i2s_tx_queue1);
	os_printf("i2s_tx_queue1_word0:%08x\n\r",READ_PERI_REG(&i2s_tx_queue1));
	os_printf("i2s_tx_queue1_buf_ptr: %08x\r\n",i2s_tx_queue1.buf_ptr);
	os_printf("i2s_tx_queue1_next_link_ptr: %08x\r\n",i2s_tx_queue1.next_link_ptr);
	os_printf("\r\n");
	os_printf("i2s_tx_queue2_address: %08x\r\n",&i2s_tx_queue2);
	os_printf("i2s_tx_queue2_word0:%08x\n\r",READ_PERI_REG(&i2s_tx_queue2));
	os_printf("i2s_tx_queue2_buf_ptr: %08x\r\n",i2s_tx_queue2.buf_ptr);
	os_printf("i2s_tx_queue2_next_link_ptr: %08x\r\n",i2s_tx_queue2.next_link_ptr);
	os_printf("\r\n");
	os_printf("i2s_tx_queue3_address: %08x\r\n",&i2s_tx_queue3);
	os_printf("i2s_tx_queue3_word0:%08x\n\r",READ_PERI_REG(&i2s_tx_queue3));
	os_printf("i2s_tx_queue3_buf_ptr: %08x\r\n",i2s_tx_queue3.buf_ptr);
	os_printf("i2s_tx_queue3_next_link_ptr: %08x\r\n",i2s_tx_queue3.next_link_ptr);
	os_printf("--------------------------------------------------\r\n");

	//config rx&tx link to hardware
	CONF_RXLINK_ADDR(&i2s_rx_queue1);
	CONF_TXLINK_ADDR(&i2s_tx_queue1);	

	os_printf("=============Descripter linked=================\r\n");

	os_printf("SLC_RX_LINK:%08x\n\r",READ_PERI_REG(SLC_RX_LINK));
	os_printf("SLC_TX_LINK:%08x\n\r",READ_PERI_REG(SLC_TX_LINK));
	os_printf("--------------------------------------------------\r\n");
	//os_printf("SLC_RXLINK_DESCADDR_MASK:%08x\n\r",READ_PERI_REG(SLC_RX_LINK)&0x000FFFFF);
  
	
 	//config rx control, start  
	START_RXLINK();
 	START_TXLINK();

	os_printf("=================set SLC start=================\r\n");
	os_printf("SLC_RX_LINK:%08x\n\r",READ_PERI_REG(SLC_RX_LINK));
	os_printf("SLC_TX_LINK:%08x\n\r",READ_PERI_REG(SLC_TX_LINK));
	os_printf("--------------------------------------------------\r\n");


	
	i2s_init();	//start


	os_printf("\n\r");
	os_printf("\n\r");
	
	//As mentioned about the use of TXLINK & RXLINK part above, 
	//rx_buff contains data sented and tx_buff contains data reveived.
	//Delay exists at the receiver side, and under this test condition the delay is 2 words.
	//However, delay may change depends on different I2S mode(like master_slave, data length, FIFO size etc).
	 for(i=0;i<IIS_TX_BUF_LEN*2;i+=32){
		os_printf("the %dth number of data received: %08x   \n\r",i+1,i2s_tx_test[i]);
	    }
	 os_printf("--------------------------------------------------\r\n");


	 os_printf("the %dth number of data received: %08x	 \n\r",1,i2s_tx_test[0]);
	 os_printf("the %dth number of data received: %08x   \n\r",2,i2s_tx_test[1]);
	 os_printf("the %dth number of data received: %08x   \n\r",3,i2s_tx_test[2]);
	 os_printf("the %dth number of data received: %08x   \n\r",4,i2s_tx_test[3]);
	 os_printf("the %dth number of data received: %08x   \n\r",5,i2s_tx_test[4]);
	 
	 os_printf("--------------------------------------------------\r\n");

	 os_printf("the %dth number of data received: %08x   \n\r",512,i2s_tx_test[511]);
	 os_printf("the %dth number of data received: %08x   \n\r",513,i2s_tx_test[512]);
	 os_printf("the %dth number of data received: %08x   \n\r",514,i2s_tx_test[513]);
	 os_printf("the %dth number of data received: %08x   \n\r",515,i2s_tx_test[514]);
	 os_printf("the %dth number of data received: %08x   \n\r",516,i2s_tx_test[515]);
   

}
