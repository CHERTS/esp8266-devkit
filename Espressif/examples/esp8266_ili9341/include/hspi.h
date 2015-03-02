/*
 * hspi.h
 *
 *  Created on: 12 џэт. 2015 у.
 *      Author: Sem
 */

#ifndef INCLUDE_HSPI_H_
#define INCLUDE_HSPI_H_

#include "driver/spi_register.h" // from 0.9.4 IoT_Demo
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>

#define SPI         0
#define HSPI        1
#define MAX_SIZE_BUFFER	64

#define CONFIG_FOR_TX			1
#define CONFIG_FOR_RX_TX		2

extern void hspi_init(void);
extern void hspi_TxRx( uint8_t * data, uint8_t numberByte);
extern void hspi_Tx(uint8_t * data, uint8_t numberByte, uint32_t numberRepeat);


#endif /* INCLUDE_HSPI_H_ */
