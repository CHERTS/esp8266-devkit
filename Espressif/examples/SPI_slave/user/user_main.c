/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "driver\spi.h"
#include "user_interface.h"
#include "driver/uart.h"

void user_rf_pre_init(void)
{
}

void ICACHE_FLASH_ATTR spi_init()
{
    spi_test_init();
}


void W25QXX_Write_Page(uint8 data,uint32 WriteAddr,uint16 NumByteToWrite)
{
	u16 i=0;  
	spi_mast_byte_write(SPI,0x06);
	SET_PERI_REG_MASK(SPI_PIN(SPI), SPI_CS_DIS);
	spi_mast_byte_write(SPI,0x02);
	spi_mast_byte_write(SPI,(u8)((WriteAddr)>>16));
	spi_mast_byte_write(SPI,(u8)((WriteAddr)>>8));
	spi_mast_byte_write(SPI,(u8)WriteAddr);

	while(i<NumByteToWrite)
	{
		spi_mast_byte_write(HSPI,data);
		i++;
	} 
	
	CLEAR_PERI_REG_MASK(SPI_PIN(SPI), SPI_CS_DIS);
	 
	if (i==NumByteToWrite){
		spi_mast_byte_write(HSPI,data);
	}
} 

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/

void user_init(void)
{
	os_printf("SDK version:%s\n", system_get_sdk_version());
	os_printf("------------------start------------------\n\r");
	set_data();
	spi_master_init(HSPI);
	spi_mast_byte_write(HSPI,0xAA);
	spi_byte_write_espslave(HSPI,0xAA);
	spi_WR_espslave(HSPI);
	spi_WR_espslave(HSPI);
	os_printf("------------------done!------------------\n\r");
	os_printf("\n\r");
	os_printf("\n\r");
}
