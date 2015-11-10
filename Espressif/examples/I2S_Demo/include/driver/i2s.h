#include "driver\i2s_reg.h"
#include "osapi.h"
#include "os_type.h"
#include "driver/sdio_slv.h"



#define CONF_RXLINK_ADDR(addr)		CLEAR_PERI_REG_MASK(SLC_RX_LINK,SLC_RXLINK_DESCADDR_MASK);\
	SET_PERI_REG_MASK(SLC_RX_LINK, ((uint32)(addr)) & SLC_RXLINK_DESCADDR_MASK)
#define CONF_TXLINK_ADDR(addr) 		CLEAR_PERI_REG_MASK(SLC_TX_LINK,SLC_TXLINK_DESCADDR_MASK);\
	SET_PERI_REG_MASK(SLC_TX_LINK, ((uint32)(addr)) & SLC_TXLINK_DESCADDR_MASK)
	
#define START_RXLINK() 	SET_PERI_REG_MASK(SLC_RX_LINK, SLC_RXLINK_START)
#define START_TXLINK() 	SET_PERI_REG_MASK(SLC_TX_LINK, SLC_TXLINK_START)

void i2s_test(void);

//Initialize the I2S module
void i2s_init();

//create DMA buffer descriptors
void creat_one_link(uint8 own, uint8 eof,uint8 sub_sof, uint16 size, uint16 length,
                       uint32* buf_ptr, struct sdio_queue* nxt_ptr, struct sdio_queue* i2s_queue);

//Initialize the SLC module for DMA function
void slc_init();

void slc_isr(void *para);

//create fake audio data
void generate_data();

//load data into buffer
void load_buffer1_1(uint32 * buffer,uint32 buff_len);
void load_buffer1_2(uint32 * buffer,uint32 buff_len);
void load_buffer2_1(uint32 * buffer,uint32 buff_len);
void load_buffer2_2(uint32 * buffer,uint32 buff_len);

