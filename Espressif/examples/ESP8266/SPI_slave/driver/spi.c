
#include "driver/spi.h"
#include "driver/spi_overlap.h"

#define CACHE_FLASH_CTRL_REG 0x3ff0000C
#define CACHE_FLUSH_START_BIT BIT0
#define CACHE_EMPTY_FLAG_BIT BIT1
/******************************************************************************
 * FunctionName : cache_flush
 * Description  : clear all the cpu cache data for stability test.
*******************************************************************************/
#if 0
void cache_flush(void)
{
   while(READ_PERI_REG(CACHE_FLASH_CTRL_REG)&CACHE_EMPTY_FLAG_BIT) {
      CLEAR_PERI_REG_MASK(CACHE_FLASH_CTRL_REG, CACHE_FLUSH_START_BIT);
      SET_PERI_REG_MASK(CACHE_FLASH_CTRL_REG, CACHE_FLUSH_START_BIT);
   }
   while(!(READ_PERI_REG(CACHE_FLASH_CTRL_REG)&CACHE_EMPTY_FLAG_BIT));
   	
   CLEAR_PERI_REG_MASK(CACHE_FLASH_CTRL_REG, CACHE_FLUSH_START_BIT);
}
#endif
/******************************************************************************
 * FunctionName : spi_master_init
 * Description  : SPI master initial function for common byte units transmission
 * Parameters   : uint8 spi_no - SPI module number, Only "SPI" and "HSPI" are valid
*******************************************************************************/
void ICACHE_FLASH_ATTR
    spi_master_init(uint8 spi_no)
{
	uint32 regvalue; 

	if(spi_no>1) {	

		os_printf("invalid spi number!\n\r");

		return;

	} //handle invalid input number

	 else if(spi_no==SPI){
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CLK_U, 1);//configure io to spi mode
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CMD_U, 1);//configure io to spi mode	
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA0_U, 1);//configure io to spi mode	
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA1_U, 1);//configure io to spi mode	
    }else if(spi_no==HSPI){
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2);//configure io to Hspi mode
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2);//configure io to Hspi mode	
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2);//configure io to Hspi mode	
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2);//configure io to Hspi mode	
    }

	
	//SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_CS_SETUP|SPI_CS_HOLD);
	//CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_FLASH_MODE|SPI_USR_COMMAND);

	SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_CS_SETUP|SPI_CS_HOLD|SPI_USR_COMMAND);
	CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_FLASH_MODE);

	WRITE_PERI_REG(SPI_CLOCK(spi_no), 
					((3&SPI_CLKCNT_N)<<SPI_CLKCNT_N_S)|
					((1&SPI_CLKCNT_H)<<SPI_CLKCNT_H_S)|
					((3&SPI_CLKCNT_L)<<SPI_CLKCNT_L_S)); //clear bit 31,set SPI clock div

}
/******************************************************************************
 * FunctionName : spi_lcd_9bit_write
 * Description  : SPI 9bits transmission function for driving LCD TM035PDZV36
 * Parameters   : 	uint8 spi_no - SPI module number, Only "SPI" and "HSPI" are valid
 *				uint8 high_bit - first high bit of the data, 0 is for "0",the other value 1-255 is for "1"
 *				uint8 low_8bit- the rest 8bits of the data.
*******************************************************************************/
void ICACHE_FLASH_ATTR
    spi_lcd_9bit_write(uint8 spi_no,uint8 high_bit,uint8 low_8bit)
{
	uint32 regvalue;
	uint8 bytetemp;
	if(spi_no>1) 		return; //handle invalid input number
	
	if(high_bit)		bytetemp=(low_8bit>>1)|0x80;
	else				bytetemp=(low_8bit>>1)&0x7f;
	
	regvalue= ((8&SPI_USR_COMMAND_BITLEN)<<SPI_USR_COMMAND_BITLEN_S)|((uint32)bytetemp);		//configure transmission variable,9bit transmission length and first 8 command bit 
	if(low_8bit&0x01) 	regvalue|=BIT15;        //write the 9th bit
	while(READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR);		//waiting for spi module available
	WRITE_PERI_REG(SPI_USER2(spi_no), regvalue);				//write  command and command length into spi reg
	SET_PERI_REG_MASK(SPI_CMD(spi_no), SPI_USR);		//transmission start
//	while(READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR);	
}


 /******************************************************************************
  * FunctionName : spi_mast_byte_write
  * Description  : SPI master 1 byte transmission function
  * Parameters	 :	 uint8 spi_no - SPI module number, Only "SPI" and "HSPI" are valid
  * 			 uint8 data- transmitted data
 *******************************************************************************/
 void //ICACHE_FLASH_ATTR
	 spi_mast_byte_write(uint8 spi_no,uint8 data)
  {
	 uint32 regvalue;
 
	 if(spi_no>1)		 return; //handle invalid input number
 
	 while(READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR);
	 CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MOSI|SPI_USR_MISO);

	 SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_COMMAND);
 
	 //SPI_FLASH_USER2 bit28-31 is cmd length,cmd bit length is value(0-15)+1,
	 // bit15-0 is cmd value.
	 WRITE_PERI_REG(SPI_USER2(spi_no), 
					 ((7&SPI_USR_COMMAND_BITLEN)<<SPI_USR_COMMAND_BITLEN_S)|((uint32)data));
	 SET_PERI_REG_MASK(SPI_CMD(spi_no), SPI_USR);
	 while(READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR);  

  }  
 
 /******************************************************************************
  * FunctionName : spi_byte_write_espslave
  * Description  : SPI master 1 byte transmission function for esp8266 slave,
  * 		 transmit 1byte data to esp8266 slave buffer needs 16bit transmission ,
  * 		 first byte is command 0x04 to write slave buffer, second byte is data
  * Parameters	 :	 uint8 spi_no - SPI module number, Only "SPI" and "HSPI" are valid
  * 			 uint8 data- transmitted data
 *******************************************************************************/
 void ICACHE_FLASH_ATTR
	 spi_byte_write_espslave(uint8 spi_no,uint8 data)
  {
	 uint32 regvalue;
 
	 if(spi_no>1)		 return; //handle invalid input number
 
	 while(READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR);
	 SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MOSI|SPI_USR_COMMAND);
	 CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MISO|SPI_USR_ADDR|SPI_USR_DUMMY);
 
	 //SPI_FLASH_USER2 bit28-31 is cmd length,cmd bit length is value(0-15)+1,
	 // bit15-0 is cmd value.
	 //0x70000000 is for 8bits cmd, 0x04 is eps8266 slave write cmd value
	 WRITE_PERI_REG(SPI_USER2(spi_no), 
					 ((7&SPI_USR_COMMAND_BITLEN)<<SPI_USR_COMMAND_BITLEN_S)|4);
	 
	 //in register SPI_FLASH_USER1, bit 8-16 stores MOSI bit length value
	 //The value shall be (bit_num-1).
	 //for example, we WRITE 1 byte data which has length of 8 bits,
	 //therefore the MOSI bit length value of 7 should be written into the related register bits.
	WRITE_PERI_REG(SPI_USER1(spi_no),((7&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S));
	 

	 WRITE_PERI_REG(SPI_W0(spi_no), (uint32)(data));
	 
	 SET_PERI_REG_MASK(SPI_CMD(spi_no), SPI_USR);

	 while(READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR); 
  }
 /******************************************************************************
  * FunctionName : spi_byte_read_espslave
  * Description  : SPI master 1 byte read function for esp8266 slave,
  * 		 read 1byte data from esp8266 slave buffer needs 16bit transmission ,
  * 		 first byte is command 0x06 to read slave buffer, second byte is recieved data
  * Parameters	 :	 uint8 spi_no - SPI module number, Only "SPI" and "HSPI" are valid
  * 			 uint8* data- recieved data address
 *******************************************************************************/
   void ICACHE_FLASH_ATTR
	   spi_byte_read_espslave(uint8 spi_no,uint8 *data)
  {
	 uint32 regvalue;
 
	 if(spi_no>1)		 return; //handle invalid input number
 
	 while(READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR);
 
	 SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MISO|SPI_USR_COMMAND);
	 CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MOSI|SPI_USR_ADDR|SPI_USR_DUMMY);
		 //SPI_FLASH_USER2 bit28-31 is cmd length,cmd bit length is value(0-15)+1,
	 // bit15-0 is cmd value.
	 //0x70000000 is for 8bits cmd, 0x06 is eps8266 slave read cmd value
	 WRITE_PERI_REG(SPI_USER2(spi_no), 
					 ((7&SPI_USR_COMMAND_BITLEN)<<SPI_USR_COMMAND_BITLEN_S)|6);

    //in register SPI_FLASH_USER1, bit 8-16 stores MOSI bit length value
	//The value shall be (bit_num-1).
	//for example, we READ 1 byte data which has length of 8 bits,
	//therefore the MISO bit length value of 7 should be written into the related register bits.
	WRITE_PERI_REG(SPI_USER1(spi_no),((7&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S));
			 
	 SET_PERI_REG_MASK(SPI_CMD(spi_no), SPI_USR);
	 
	 while(READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR);
	 
	 *data=(uint8)(READ_PERI_REG(SPI_W0(spi_no))&0xff);
  }
 

 
 
  /******************************************************************************
  * FunctionName :	   spi_WR_espslave(uint8 spi_no)
  * Description  : SPI master byte units write and read
  * Parameters	 : uint8 spi_no - SPI module number, Only "SPI" and "HSPI" are valid
 *******************************************************************************/

 void ICACHE_FLASH_ATTR
	 spi_WR_espslave(uint8 spi_no)
{
   uint32 regvalue;

   if(spi_no>1) {	

		os_printf("invalid spi number!\n\r");

		return;

	} //handle invalid input number

   while(READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR);

   SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MOSI|SPI_USR_DOUTDIN);
   
   CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);
   
    //in register SPI_FLASH_USER1, bit 17-25 stores MOSI bit length value
    //The value shall be (bit_num-1).
    //for example, we output 64-byte data which has length of 512 bits,
    //therefore the MOSI bit length value of 511(0x1FF) should be written into the related register bits.
	WRITE_PERI_REG(SPI_USER1(spi_no),((0x1ff&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S));
	
   SET_PERI_REG_MASK(SPI_CMD(spi_no), SPI_USR);
   
   while(READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR);
   
}


void ICACHE_FLASH_ATTR
    set_data()
{
        WRITE_PERI_REG(SPI_W0(HSPI),0x05040302);
        WRITE_PERI_REG(SPI_W1(HSPI),0x09080706);
        WRITE_PERI_REG(SPI_W2(HSPI),0x0d0c0b0a);
        WRITE_PERI_REG(SPI_W3(HSPI),0x11100f0e);
        WRITE_PERI_REG(SPI_W4(HSPI),0x15141312);
        WRITE_PERI_REG(SPI_W5(HSPI),0x19181716);
        WRITE_PERI_REG(SPI_W6(HSPI),0x1d1c1b1a);
        WRITE_PERI_REG(SPI_W7(HSPI),0x21201f1e);
        WRITE_PERI_REG(SPI_W8(HSPI),0x05040302);
        WRITE_PERI_REG(SPI_W9(HSPI),0x09080706);
        WRITE_PERI_REG(SPI_W10(HSPI),0x0d0c0b0a);
        WRITE_PERI_REG(SPI_W11(HSPI),0x11100f0e);
        WRITE_PERI_REG(SPI_W12(HSPI),0x15141312);
        WRITE_PERI_REG(SPI_W13(HSPI),0x19181716);
        WRITE_PERI_REG(SPI_W14(HSPI),0x1d1c1b1a);
        WRITE_PERI_REG(SPI_W15(HSPI),0x21201f1e);

    }


//-----------------------------------------------------------------------------------------------------------------------------
#ifdef SPI_SLAVE_DEBUG
 /******************************************************************************
 * FunctionName : spi_slave_init
 * Description  : SPI slave mode initial funtion, including mode setting,
 * 			IO setting, transmission interrupt opening, interrupt function registration
 * Parameters   : 	uint8 spi_no - SPI module number, Only "SPI" and "HSPI" are valid
*******************************************************************************/
void spi_slave_init(uint8 spi_no)
{
    uint32 regvalue; 
    if(spi_no>1)
        return; //handle invalid input number

    //clear bit9,bit8 of reg PERIPHS_IO_MUX
    //bit9 should be cleared when HSPI clock doesn't equal CPU clock
    //bit8 should be cleared when SPI clock doesn't equal CPU clock
    ////WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105); //clear bit9//TEST
    if(spi_no==SPI){
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CLK_U, 1);//configure io to spi mode
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CMD_U, 1);//configure io to spi mode	
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA0_U, 1);//configure io to spi mode	
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA1_U, 1);//configure io to spi mode	
    }else if(spi_no==HSPI){
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2);//configure io to Hspi mode
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2);//configure io to Hspi mode	
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2);//configure io to Hspi mode	
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2);//configure io to Hspi mode	
    }

    //regvalue=READ_PERI_REG(SPI_FLASH_SLAVE(spi_no));
    //slave mode,slave use buffers which are register "SPI_FLASH_C0~C15", enable trans done isr
    //set bit 30 bit 29 bit9,bit9 is trans done isr mask
    SET_PERI_REG_MASK(	SPI_SLAVE(spi_no), 
    						SPI_SLAVE_MODE|SPI_SLV_WR_RD_BUF_EN|
                                         	SPI_SLV_WR_BUF_DONE_EN|SPI_SLV_RD_BUF_DONE_EN|
                                         	SPI_SLV_WR_STA_DONE_EN|SPI_SLV_RD_STA_DONE_EN|
                                         	SPI_TRANS_DONE_EN);
    //disable general trans intr 
    //CLEAR_PERI_REG_MASK(SPI_SLAVE(spi_no),SPI_TRANS_DONE_EN);

    CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_FLASH_MODE);//disable flash operation mode
    SET_PERI_REG_MASK(SPI_USER(spi_no),SPI_USR_MISO_HIGHPART);//SLAVE SEND DATA BUFFER IN C8-C15 


//////**************RUN WHEN SLAVE RECIEVE*******************///////
   //tow lines below is to configure spi timing.
    SET_PERI_REG_MASK(SPI_CTRL2(spi_no),(0x2&SPI_MOSI_DELAY_NUM)<<SPI_MOSI_DELAY_NUM_S) ;//delay num
    os_printf("SPI_CTRL2 is %08x\n",READ_PERI_REG(SPI_CTRL2(spi_no)));
    WRITE_PERI_REG(SPI_CLOCK(spi_no), 0);


    
/////***************************************************//////	

    //set 8 bit slave command length, because slave must have at least one bit addr, 
    //8 bit slave+8bit addr, so master device first 2 bytes can be regarded as a command 
    //and the  following bytes are datas, 
    //32 bytes input wil be stored in SPI_FLASH_C0-C7
    //32 bytes output data should be set to SPI_FLASH_C8-C15
    WRITE_PERI_REG(SPI_USER2(spi_no), (0x7&SPI_USR_COMMAND_BITLEN)<<SPI_USR_COMMAND_BITLEN_S); //0x70000000


    //set 8 bit slave recieve buffer length, the buffer is SPI_FLASH_C0-C7
    //set 8 bit slave status register, which is the low 8 bit of register "SPI_FLASH_STATUS"
    SET_PERI_REG_MASK(SPI_SLAVE1(spi_no),  ((0xff&SPI_SLV_BUF_BITLEN)<< SPI_SLV_BUF_BITLEN_S)|
                                                                                        ((0x7&SPI_SLV_STATUS_BITLEN)<<SPI_SLV_STATUS_BITLEN_S)|
                                                                                       ((0x7&SPI_SLV_WR_ADDR_BITLEN)<<SPI_SLV_WR_ADDR_BITLEN_S)|
                                                                                       ((0x7&SPI_SLV_RD_ADDR_BITLEN)<<SPI_SLV_RD_ADDR_BITLEN_S));
    CLEAR_PERI_REG_MASK(SPI_SLAVE1(spi_no),  BIT25);//CHOOSE ACTIVE STATUS REG
    SET_PERI_REG_MASK(SPI_PIN(spi_no),BIT19);//BIT19   

    //maybe enable slave transmission liston 
    SET_PERI_REG_MASK(SPI_CMD(spi_no),SPI_USR);
    //register level2 isr function, which contains spi, hspi and i2s events
    #if TWO_INTR_LINE_PROTOCOL
    ETS_SPI_INTR_ATTACH(spi_slave_isr_handler,NULL);
   #elif ONE_INTR_LINE_WITH_STATUS
   ETS_SPI_INTR_ATTACH(spi_slave_isr_sta,NULL);
   #endif
    //enable level2 isr, which contains spi, hspi and i2s events
    ETS_SPI_INTR_ENABLE(); 
}
/******************************************************************************
 * FunctionName : spi_slave_isr_handler
 * Description  : SPI interrupt function, SPI HSPI and I2S interrupt can trig this function
 			   some basic operation like clear isr flag has been done, 
 			   and it is availible	for adding user coder in the funtion
 * Parameters  : void *para- function parameter address, which has been registered in function spi_slave_init
*******************************************************************************/
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
static uint8 spi_data[32] = {0};
static uint8 idx = 0;
static uint8 spi_flg = 0;
#define SPI_MISO
#define SPI_QUEUE_LEN 8
os_event_t * spiQueue;
#define MOSI  0
#define MISO  1
#define STATUS_R_IN_WR 2
#define STATUS_W  3
#define TR_DONE_ALONE  4
#define WR_RD 5
#define DATA_ERROR 6
#define STATUS_R_IN_RD 7
//init the two intr line of slave
//gpio0: wr_ready ,and  
//gpio2: rd_ready , controlled by slave
void ICACHE_FLASH_ATTR
    gpio_init()
{

    	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
#if	TWO_INTR_LINE_PROTOCOL
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
    	GPIO_OUTPUT_SET(0, 1);
    	GPIO_OUTPUT_SET(2, 0);
	GPIO_OUTPUT_SET(4, 1);
#endif
}



void spi_slave_isr_handler(void *para)
{
	uint32 regvalue,calvalue;
    	static uint8 state =0;
	uint32 recv_data,send_data;

	if(READ_PERI_REG(0x3ff00020)&BIT4){		
        //following 3 lines is to clear isr signal
        	CLEAR_PERI_REG_MASK(SPI_SLAVE(SPI), 0x3ff);
    	}else if(READ_PERI_REG(0x3ff00020)&BIT7){ //bit7 is for hspi isr,
        	regvalue=READ_PERI_REG(SPI_SLAVE(HSPI));
         	CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI),  
								SPI_TRANS_DONE_EN|
								SPI_SLV_WR_STA_DONE_EN|
								SPI_SLV_RD_STA_DONE_EN|
								SPI_SLV_WR_BUF_DONE_EN|
								SPI_SLV_RD_BUF_DONE_EN);
        	SET_PERI_REG_MASK(SPI_SLAVE(HSPI), SPI_SYNC_RESET);
        	CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI),  
								SPI_TRANS_DONE|
								SPI_SLV_WR_STA_DONE|
								SPI_SLV_RD_STA_DONE|
								SPI_SLV_WR_BUF_DONE|
								SPI_SLV_RD_BUF_DONE); 
		SET_PERI_REG_MASK(SPI_SLAVE(HSPI),  
								SPI_TRANS_DONE_EN|
								SPI_SLV_WR_STA_DONE_EN|
								SPI_SLV_RD_STA_DONE_EN|
								SPI_SLV_WR_BUF_DONE_EN|
								SPI_SLV_RD_BUF_DONE_EN);

		if((regvalue&SPI_SLV_WR_BUF_DONE)&&(regvalue&SPI_SLV_RD_BUF_DONE)){
			system_os_post(USER_TASK_PRIO_1,WR_RD,regvalue);
		}

		else if(regvalue&SPI_SLV_WR_BUF_DONE){ 
            		GPIO_OUTPUT_SET(0, 0);
			state=0;
            		idx=0;
            		while(idx<8){
            			recv_data=READ_PERI_REG(SPI_W0(HSPI)+(idx<<2));
            			spi_data[idx<<2] = recv_data&0xff;
            			spi_data[(idx<<2)+1] = (recv_data>>8)&0xff;
            			spi_data[(idx<<2)+2] = (recv_data>>16)&0xff;
            			spi_data[(idx<<2)+3] = (recv_data>>24)&0xff;
            			idx++;
			}

			for(idx=0;idx<8;idx++)
			{
				send_data=READ_PERI_REG(SPI_W8(HSPI)+(idx<<2));
				calvalue=send_data&0xff;
				calvalue=(calvalue*calvalue*calvalue)%255;
				if(spi_data[idx<<2]!=calvalue){		
					GPIO_OUTPUT_SET(4, 0);
					system_os_post(USER_TASK_PRIO_1,DATA_ERROR,regvalue);
				}
				calvalue=(send_data>>8)&0xff;
				calvalue=(calvalue*calvalue*calvalue)%255;
				if(spi_data[(idx<<2)+1] !=calvalue){	
					GPIO_OUTPUT_SET(4, 0);
					system_os_post(USER_TASK_PRIO_1,DATA_ERROR,regvalue);
				}
				calvalue=(send_data>>16)&0xff;
				calvalue=(calvalue*calvalue*calvalue)%255;
				if(spi_data[(idx<<2)+2] !=calvalue){	
					GPIO_OUTPUT_SET(4, 0);	
					system_os_post(USER_TASK_PRIO_1,DATA_ERROR,regvalue);
				}
				calvalue=(send_data>>24)&0xff;
				calvalue=(calvalue*calvalue*calvalue)%255;
				if(spi_data[(idx<<2)+3] !=calvalue){	
					GPIO_OUTPUT_SET(4, 0);	
					system_os_post(USER_TASK_PRIO_1,DATA_ERROR,regvalue);
				}
			}

			for(idx=0;idx<8;idx++)
			{
				WRITE_PERI_REG(SPI_W8(HSPI)+(idx<<2), READ_PERI_REG(SPI_W0(HSPI)+(idx<<2)));
			}
			GPIO_OUTPUT_SET(2, 1);
            		GPIO_OUTPUT_SET(0, 1);
		 	GPIO_OUTPUT_SET(4, 1);
            		SET_PERI_REG_MASK(SPI_SLAVE(HSPI),  SPI_SLV_WR_BUF_DONE_EN);
					
        	}

		else if(regvalue&SPI_SLV_RD_BUF_DONE){
           		GPIO_OUTPUT_SET(2, 0);
			state=1;
        	}

		if(regvalue&SPI_SLV_RD_STA_DONE){
			GPIO_OUTPUT_SET(4, 0);
			if(state){	
				system_os_post(USER_TASK_PRIO_1,STATUS_R_IN_WR,regvalue);
				state=0;
			}
			else{
				system_os_post(USER_TASK_PRIO_1,STATUS_R_IN_RD,regvalue);
				state=1;
			}		
			GPIO_OUTPUT_SET(4, 1);
		}

		if(regvalue&SPI_SLV_WR_STA_DONE){
			GPIO_OUTPUT_SET(4, 0);	
			system_os_post(USER_TASK_PRIO_1,STATUS_W,regvalue);
			GPIO_OUTPUT_SET(4, 1);	
		}

		if((regvalue&SPI_TRANS_DONE)&&((regvalue&0xf)==0)){
	//		GPIO_OUTPUT_SET(4, 0);
			system_os_post(USER_TASK_PRIO_1,TR_DONE_ALONE,regvalue);
	//		GPIO_OUTPUT_SET(4, 1);	
		}
    
    }

		else if(READ_PERI_REG(0x3ff00020)&BIT9){ //bit7 is for i2s isr,

    }
}

void spi_slave_isr_sta(void *para)
{
	uint32 regvalue,calvalue;
    	static uint8 state =0;
	uint32 recv_data,send_data;

	union spi_slave_status spi_sta;
	
	if(READ_PERI_REG(0x3ff00020)&BIT4){		
        //following 3 lines is to clear isr signal
        	CLEAR_PERI_REG_MASK(SPI_SLAVE(SPI), 0x3ff);
    	}else if(READ_PERI_REG(0x3ff00020)&BIT7){ //bit7 is for hspi isr,
        	regvalue=READ_PERI_REG(SPI_SLAVE(HSPI));
         	CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI),  
								SPI_TRANS_DONE_EN|
								SPI_SLV_WR_STA_DONE_EN|
								SPI_SLV_RD_STA_DONE_EN|
								SPI_SLV_WR_BUF_DONE_EN|
								SPI_SLV_RD_BUF_DONE_EN);
        	SET_PERI_REG_MASK(SPI_SLAVE(HSPI), SPI_SYNC_RESET);
        	CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI),  
								SPI_TRANS_DONE|
								SPI_SLV_WR_STA_DONE|
								SPI_SLV_RD_STA_DONE|
								SPI_SLV_WR_BUF_DONE|
								SPI_SLV_RD_BUF_DONE); 
		SET_PERI_REG_MASK(SPI_SLAVE(HSPI),  
								SPI_TRANS_DONE_EN|
								SPI_SLV_WR_STA_DONE_EN|
								SPI_SLV_RD_STA_DONE_EN|
								SPI_SLV_WR_BUF_DONE_EN|
								SPI_SLV_RD_BUF_DONE_EN);

		 if(regvalue&SPI_SLV_WR_BUF_DONE){ 
		 	spi_sta.byte_value=READ_PERI_REG(SPI_RD_STATUS(HSPI))&0xff;
			spi_sta.elm_value.wr_busy=1;
			spi_sta.elm_value.comm_cnt++;
			WRITE_PERI_REG(SPI_RD_STATUS(HSPI), (uint32)spi_sta.byte_value);
			
			state=0;
            		idx=0;
            		while(idx<8){
            			recv_data=READ_PERI_REG(SPI_W0(HSPI)+(idx<<2));
            			spi_data[idx<<2] = recv_data&0xff;
            			spi_data[(idx<<2)+1] = (recv_data>>8)&0xff;
            			spi_data[(idx<<2)+2] = (recv_data>>16)&0xff;
            			spi_data[(idx<<2)+3] = (recv_data>>24)&0xff;
            			idx++;
			}

			for(idx=0;idx<8;idx++)
			{
				send_data=READ_PERI_REG(SPI_W8(HSPI)+(idx<<2));
				calvalue=send_data&0xff;
				calvalue=(calvalue*calvalue*calvalue)%255;
				if(spi_data[idx<<2]!=calvalue){		
					GPIO_OUTPUT_SET(4, 0);
					system_os_post(USER_TASK_PRIO_1,DATA_ERROR,regvalue);
				}
				calvalue=(send_data>>8)&0xff;
				calvalue=(calvalue*calvalue*calvalue)%255;
				if(spi_data[(idx<<2)+1] !=calvalue){	
					GPIO_OUTPUT_SET(4, 0);
					system_os_post(USER_TASK_PRIO_1,DATA_ERROR,regvalue);
				}
				calvalue=(send_data>>16)&0xff;
				calvalue=(calvalue*calvalue*calvalue)%255;
				if(spi_data[(idx<<2)+2] !=calvalue){	
					GPIO_OUTPUT_SET(4, 0);	
					system_os_post(USER_TASK_PRIO_1,DATA_ERROR,regvalue);
				}
				calvalue=(send_data>>24)&0xff;
				calvalue=(calvalue*calvalue*calvalue)%255;
				if(spi_data[(idx<<2)+3] !=calvalue){	
					GPIO_OUTPUT_SET(4, 0);	
					system_os_post(USER_TASK_PRIO_1,DATA_ERROR,regvalue);
				}
			}
			spi_sta.byte_value=READ_PERI_REG(SPI_RD_STATUS(HSPI))&0xff;
			spi_sta.elm_value.wr_busy=0;
			WRITE_PERI_REG(SPI_RD_STATUS(HSPI), (uint32)spi_sta.byte_value);

			for(idx=0;idx<8;idx++)
			{
				WRITE_PERI_REG(SPI_W8(HSPI)+(idx<<2), READ_PERI_REG(SPI_W0(HSPI)+(idx<<2)));
			}
			spi_sta.byte_value=READ_PERI_REG(SPI_RD_STATUS(HSPI))&0xff;
			spi_sta.elm_value.rd_empty=0;
			WRITE_PERI_REG(SPI_RD_STATUS(HSPI), (uint32)spi_sta.byte_value);
			GPIO_OUTPUT_SET(0, 1);

        	}else if(regvalue&SPI_SLV_RD_BUF_DONE){
           		spi_sta.byte_value=READ_PERI_REG(SPI_RD_STATUS(HSPI))&0xff;
			spi_sta.elm_value.comm_cnt++;
			spi_sta.elm_value.rd_empty=1;
			WRITE_PERI_REG(SPI_RD_STATUS(HSPI), (uint32)spi_sta.byte_value);
			GPIO_OUTPUT_SET(0, 1);
			state=1;
        	}

		if(regvalue&SPI_SLV_RD_STA_DONE){
			GPIO_OUTPUT_SET(0,0);
		}

		if(regvalue&SPI_SLV_WR_STA_DONE){

		}

		if((regvalue&SPI_TRANS_DONE)&&((regvalue&0xf)==0)){

		}
    
    }

		else if(READ_PERI_REG(0x3ff00020)&BIT9){ //bit7 is for i2s isr,

    }
}


void ICACHE_FLASH_ATTR
    set_miso_data()
{
    if(GPIO_INPUT_GET(2)==0){
        WRITE_PERI_REG(SPI_W8(HSPI),0x05040302);
        WRITE_PERI_REG(SPI_W9(HSPI),0x09080706);
        WRITE_PERI_REG(SPI_W10(HSPI),0x0d0c0b0a);
        WRITE_PERI_REG(SPI_W11(HSPI),0x11100f0e);

        WRITE_PERI_REG(SPI_W12(HSPI),0x15141312);
        WRITE_PERI_REG(SPI_W13(HSPI),0x19181716);
        WRITE_PERI_REG(SPI_W14(HSPI),0x1d1c1b1a);
        WRITE_PERI_REG(SPI_W15(HSPI),0x21201f1e);
        GPIO_OUTPUT_SET(2, 1);
    }
}



void ICACHE_FLASH_ATTR
    disp_spi_data()
{
    uint8 i = 0;
    for(i=0;i<32;i++){
        os_printf("data %d : 0x%02x\n\r",i,spi_data[i]);
    }
}


void ICACHE_FLASH_ATTR
    spi_task(os_event_t *e)
{
    uint8 data;
    switch(e->sig){
       case MOSI:
            	disp_spi_data();
            	break;
	case STATUS_R_IN_WR :
		os_printf("SR ERR in WRPR,Reg:%08x \n",e->par);
		break;
	case STATUS_W:
		os_printf("SW ERR,Reg:%08x\n",e->par);
		break;	
	case TR_DONE_ALONE:
		os_printf("TD ALO ERR,Reg:%08x\n",e->par);
		break;	
	case WR_RD:
		os_printf("WR&RD ERR,Reg:%08x\n",e->par);
		break;	
	case DATA_ERROR:
		os_printf("Data ERR,Reg:%08x\n",e->par);
		break;
	case STATUS_R_IN_RD :
		os_printf("SR ERR in RDPR,Reg:%08x\n",e->par);
		break;	
        default:
            break;
    }
}

void ICACHE_FLASH_ATTR
    spi_task_init(void)
{
    spiQueue = (os_event_t*)os_malloc(sizeof(os_event_t)*SPI_QUEUE_LEN);
    system_os_task(spi_task,USER_TASK_PRIO_1,spiQueue,SPI_QUEUE_LEN);
}

os_timer_t spi_timer_test;

void ICACHE_FLASH_ATTR
    spi_test_init()
{
    os_printf("spi init\n\r");
    spi_slave_init(HSPI);
    os_printf("gpio init\n\r");
    gpio_init();
    os_printf("spi task init \n\r");
    spi_task_init();
#ifdef SPI_MISO
    os_printf("spi miso init\n\r");
    set_miso_data();
#endif

}
#endif


