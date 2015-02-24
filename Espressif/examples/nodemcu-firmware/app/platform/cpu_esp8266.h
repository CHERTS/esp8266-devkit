#ifndef __CPU_ESP8266_H__
#define __CPU_ESP8266_H__

#include "os_type.h"
#include "spi_flash.h"
#include "pin_map.h"
#include "user_config.h"
#include "flash_api.h"
// Number of resources (0 if not available/not implemented)
#define NUM_GPIO              GPIO_PIN_NUM
#define NUM_SPI               2
#define NUM_UART              1
#define NUM_PWM               GPIO_PIN_NUM
#define NUM_ADC               1
#define NUM_CAN               0
#define NUM_I2C               1
#define NUM_OW                GPIO_PIN_NUM
#define NUM_TMR               7

#if defined(FLASH_512K)
#define FLASH_SEC_NUM 	0x80 	// 4MByte: 0x400, 2MByte: 0x200, 1MByte: 0x100, 512KByte: 0x80
#elif defined(FLASH_1M)
#define FLASH_SEC_NUM 	0x100
#elif defined(FLASH_2M)
#define FLASH_SEC_NUM 	0x200
#elif defined(FLASH_4M)
#define FLASH_SEC_NUM 	0x400
#elif defined(FLASH_8M)
#define FLASH_SEC_NUM 	0x800
#elif defined(FLASH_16M)
#define FLASH_SEC_NUM 	0x1000
#elif defined(FLASH_AUTOSIZE)
#define FLASH_SEC_NUM 	(flash_get_sec_num())
#else
#define FLASH_SEC_NUM 	0x80
#endif
#define SYS_PARAM_SEC_NUM 4
#define SYS_PARAM_SEC_START (FLASH_SEC_NUM - SYS_PARAM_SEC_NUM)

// #define WOFS_SEC_START	0x80
// #define WOFS_SEC_START	0x60
// #define WOFS_SEC_END	(SYS_PARAM_SEC_START)
// #define WOFS_SEC_NUM	(WOFS_SEC_END - WOFS_SEC_START)
// #define WOFS_SEC_NUM 0xc

#define INTERNAL_FLASH_SECTOR_SIZE      SPI_FLASH_SEC_SIZE
// #define INTERNAL_FLASH_SECTOR_ARRAY     { 0x4000, 0x4000, 0x4000, 0x4000, 0x10000, 0x20000, 0x20000, 0x20000, 0x20000, 0x20000 }
#define INTERNAL_FLASH_WRITE_UNIT_SIZE  4
#define INTERNAL_FLASH_READ_UNIT_SIZE	4

#define INTERNAL_FLASH_SIZE             ( (SYS_PARAM_SEC_START) * INTERNAL_FLASH_SECTOR_SIZE )
#define INTERNAL_FLASH_START_ADDRESS    0x40200000

// SpiFlashOpResult spi_flash_erase_sector(uint16 sec);
// SpiFlashOpResult spi_flash_write(uint32 des_addr, uint32 *src_addr, uint32 size);
// SpiFlashOpResult spi_flash_read(uint32 src_addr, uint32 *des_addr, uint32 size);
#define flash_write spi_flash_write
#define flash_erase spi_flash_erase_sector
#define flash_read spi_flash_read

#endif // #ifndef __CPU_ESP8266_H__
