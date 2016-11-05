/*
The driver for the sensor BME280

Copyright (c) 2016, Cosmin Plasoianu
Copyright (c) 2016, Mikhail Grigorev
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
*/

#ifndef _BME280_H_
#define _BME280_H_

#include <osapi.h>
#include <c_types.h>
#include "user_config.h"
#include "I2C.h"

// Chip ID
#define BME280_CHIP_ID		0x60

// I2C device address
#define BME280_I2C_ADDR			0xEC	// 0xEC or 0xEE
#define BME280_I2C_ADDRESS_LOW	0x76	// Not in uses
#define BME280_I2C_ADDRESS_HIGH	0x77	// Not in uses

// Configuration parameters
// Temperature oversampling
#define BME280_OS_T_SKP		0x00
#define BME280_OS_T_1		0x01
#define BME280_OS_T_2		0x02
#define BME280_OS_T_4		0x03
#define BME280_OS_T_8		0x04
#define BME280_OS_T_16		0x05

// Pressure oversampling
#define BME280_OS_P_SKP		0x00
#define BME280_OS_P_1		0x01
#define BME280_OS_P_2		0x02
#define BME280_OS_P_4		0x03
#define BME280_OS_P_8		0x04
#define BME280_OS_P_16		0x05

// Humidity oversampling
#define BME280_OS_H_SKP		0x00
#define BME280_OS_H_1		0x01
#define BME280_OS_H_2		0x02
#define BME280_OS_H_4		0x03
#define BME280_OS_H_8		0x04
#define BME280_OS_H_16		0x05

// Filter coefficient
#define BME280_FILTER_OFF	0x00
#define BME280_FILTER_2		0x01
#define BME280_FILTER_4		0x02
#define BME280_FILTER_8		0x03
#define BME280_FILTER_16	0x04

// Power Mode
#define BME280_MODE_SLEEP		0x00
#define BME280_MODE_FORCED		0x01
#define BME280_MODE_NORMAL		0x03
#define BME280_SOFT_RESET_CODE  0xB6

// Standby time (ms)
#define BME280_TSB_05		0x00	// 0.5 ms
#define BME280_TSB_62		0x01	// 62.5 ms
#define BME280_TSB_125		0x02	// 125 ms
#define BME280_TSB_250		0x03	// 250 ms
#define BME280_TSB_500		0x04
#define BME280_TSB_1000		0x05
#define BME280_TSB_10		0x06
#define BME280_TSB_20		0x07

// Data read delay
#define BME280_REGISTER_READ_DELAY	1

// User API
uint8 BME280_Init(uint8 os_t, uint8 os_p, uint8 os_h,
					uint8 filter, uint8 mode, uint8 t_sb);
uint8 BME280_ReadAll(sint32* t, uint32* p, uint32* h);
uint8 BME280_SetMode(uint8 mode);

#endif /* _BME280_H_ */
