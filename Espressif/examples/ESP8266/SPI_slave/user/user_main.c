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
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABBBCDDD
 *                A : rf cal
 *                B : at parameters
 *                C : rf init data
 *                D : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 8;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{
}

void ICACHE_FLASH_ATTR user_init(void)
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
