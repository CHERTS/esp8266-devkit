/*
    The driver for the temperature sensor and pressure BMP180
    Official repository: https://github.com/CHERTS/esp8266-i2c_bmp180
    Base on https://github.com/reaper7/esp8266_i2c_bmp180
    This driver depends on the I2C driver https://github.com/zarya/esp8266_i2c_driver/

    Copyright (C) 2014 reaper7
    Copyright (C) 2014 Mikhail Grigorev (CHERTS)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __I2C_BMP180_H
#define	__I2C_BMP180_H

#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"

#define CONVERSION_TIME				5
#define BMP180_W					0xEE
#define BMP180_R					0xEF
#define BMP180_CHIP_ID				0x5502
#define BMP180_VERSION_REG			0xD1
#define BMP180_CHIP_ID_REG			0xD0
#define BMP180_CTRL_REG				0xF4
#define BMP180_DATA_REG				0xF6
#define BMP_CMD_MEASURE_TEMP		0x2E	// Max conversion time 4.5ms
#define BMP_CMD_MEASURE_PRESSURE_0	0x34	// Max conversion time 4.5ms (OSS = 0)
//#define BMP_CMD_MEASURE_PRESSURE_1	0x74	// Max conversion time 7.5ms (OSS = 1)
//#define BMP_CMD_MEASURE_PRESSURE_2	0xB4	// Max conversion time 13.5ms (OSS = 2)
//#define BMP_CMD_MEASURE_PRESSURE_3	0xF4	// Max conversion time 25.5ms (OSS = 3)
#define MYALTITUDE  				153.0

#define BMP180_DEBUG 1

enum PRESSURE_RESOLUTION {
	OSS_0 = 0,
	OSS_1,
	OSS_2,
	OSS_3
};

bool BMP180_Init(void);
int32_t BMP180_GetTemperature();
int32_t BMP180_GetPressure(enum PRESSURE_RESOLUTION resolution);
int32_t BMP180_CalcAltitude(int32_t pressure);
char* BMP180Int2String(char* buffer, int32_t value);
char* BMP180Float2String(char* buffer, float value);

#endif
