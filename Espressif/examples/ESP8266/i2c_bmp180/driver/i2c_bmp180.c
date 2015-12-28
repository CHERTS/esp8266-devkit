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

#include <math.h>
#include "driver/i2c.h"
#include "driver/i2c_bmp180.h"

int16_t ac1, ac2, ac3;
uint16_t ac4, ac5, ac6;
int16_t b1, b2;
int16_t mb, mc, md; 

int16_t ICACHE_FLASH_ATTR BMP180_readRegister16(uint8_t reg)
{
	i2c_start();
	i2c_writeByte(BMP180_W);
	if(!i2c_check_ack()){
		#ifdef BMP180_DEBUG
		//os_printf("slave not ack..\n return \n");
		ets_uart_printf("BMP180_readRegister16: i2c_writeByte(0xEE) slave not ack..\r\nreturn\r\n");
		#endif
		i2c_stop();
		return(0);
	}
	i2c_writeByte(reg);
	if(!i2c_check_ack()){
		#ifdef BMP180_DEBUG
		//os_printf("slave not ack..\n return \n");
		ets_uart_printf("BMP180_readRegister16: i2c_writeByte(reg) slave not ack..\r\nreturn\r\n");
		#endif
		i2c_stop();
		return(0);
	}
	i2c_start();
	i2c_writeByte(BMP180_R);
	if(!i2c_check_ack()){
		#ifdef BMP180_DEBUG
		//os_printf("slave not ack..\n return \n");
		ets_uart_printf("BMP180_readRegister16: i2c_writeByte(0xEF) slave not ack..\r\nreturn\r\n");
		#endif
		i2c_stop();
		return(0);
	}
	uint8_t msb = i2c_readByte();
	i2c_send_ack(1);
	uint8_t lsb = i2c_readByte();
	i2c_send_ack(0);
	i2c_stop();
	int16_t res = (msb << 8) + lsb;
	//int16_t res = (msb << 8) | lsb;
	return res;
}

int16_t ICACHE_FLASH_ATTR BMP180_readExRegister16(uint8_t reg, enum PRESSURE_RESOLUTION resolution)
{
	i2c_start();
	i2c_writeByte(BMP180_W);
	if(!i2c_check_ack()){
		#ifdef BMP180_DEBUG
		//os_printf("slave not ack..\n return \n");
		ets_uart_printf("BMP180_readExRegister16: i2c_writeByte(0xEE) slave not ack..\r\nreturn\r\n");
		#endif
		i2c_stop();
		return(0);
	}
	i2c_writeByte(reg);
	if(!i2c_check_ack()){
		#ifdef BMP180_DEBUG
		//os_printf("slave not ack..\n return \n");
		ets_uart_printf("BMP180_readExRegister16: i2c_writeByte(reg) slave not ack..\r\nreturn\r\n");
		#endif
		i2c_stop();
		return(0);
	}
	i2c_start();
	i2c_writeByte(BMP180_R);
	if(!i2c_check_ack()){
		#ifdef BMP180_DEBUG
		//os_printf("slave not ack..\n return \n");
		ets_uart_printf("BMP180_readExRegister16: i2c_writeByte(0xEF) slave not ack..\r\nreturn\r\n");
		#endif
		i2c_stop();
		return(0);
	}
	uint8_t msb = i2c_readByte();
	i2c_send_ack(1);
	uint8_t lsb = i2c_readByte();
	i2c_send_ack(0);
	uint8_t xlsb = i2c_readByte();
	i2c_send_ack(0);
	i2c_stop();
	int32_t res = ((msb << 16) + (lsb << 8) + xlsb) >> (8-resolution);
	return res;
}

int16_t ICACHE_FLASH_ATTR BMP180_readRawValue(uint8_t cmd)
{
	i2c_start();
	i2c_writeByte(BMP180_W);
	if(!i2c_check_ack()){
		#ifdef BMP180_DEBUG
		//os_printf("slave not ack..\n return \n");
		ets_uart_printf("BMP180_readRawValue: i2c_writeByte(0xEE) slave not ack..\r\nreturn\r\n");
		#endif
		i2c_stop();
		return(0);
	}
	i2c_writeByte(BMP180_CTRL_REG);
	if(!i2c_check_ack()){
		#ifdef BMP180_DEBUG
		//os_printf("slave not ack..\n return \n");
		ets_uart_printf("BMP180_readRawValue: i2c_writeByte(0xF4) slave not ack..\r\nreturn\r\n");
		#endif
		i2c_stop();
		return(0);
	}
	i2c_writeByte(cmd);
	if(!i2c_check_ack()){
		#ifdef BMP180_DEBUG
		//os_printf("slave not ack..\n return \n");
		ets_uart_printf("BMP180_readRawValue: i2c_writeByte(cmd) slave not ack..\r\nreturn\r\n");
		#endif
		i2c_stop();
		return(0);
	}
	i2c_stop();
	os_delay_us(CONVERSION_TIME*900); // max time is 4.5ms
	int16_t res = BMP180_readRegister16(BMP180_DATA_REG);
	return res;
}

int16_t ICACHE_FLASH_ATTR BMP180_readExRawValue(uint8_t cmd, enum PRESSURE_RESOLUTION resolution)
{
	i2c_start();
	i2c_writeByte(BMP180_W);
	if(!i2c_check_ack()){
		#ifdef BMP180_DEBUG
		//os_printf("slave not ack..\n return \n");
		ets_uart_printf("BMP180_readExRawValue: i2c_writeByte(0xEE) slave not ack..\r\nreturn\r\n");
		#endif
		i2c_stop();
		return(0);
	}
	i2c_writeByte(BMP180_CTRL_REG);
	if(!i2c_check_ack()){
		#ifdef BMP180_DEBUG
		//os_printf("slave not ack..\n return \n");
		ets_uart_printf("BMP180_readExRawValue: i2c_writeByte(0xF4) slave not ack..\r\nreturn\r\n");
		#endif
		i2c_stop();
		return(0);
	}
	i2c_writeByte(cmd);
	if(!i2c_check_ack()){
		#ifdef BMP180_DEBUG
		//os_printf("slave not ack..\n return \n");
		ets_uart_printf("BMP180_readExRawValue: i2c_writeByte(cmd) slave not ack..\r\nreturn\r\n");
		#endif
		i2c_stop();
		return(0);
	}
	i2c_stop();
	switch(resolution)
	{
		case OSS_0:
			os_delay_us(CONVERSION_TIME*900);
			break;
		case OSS_1:
			os_delay_us(CONVERSION_TIME*1500);
			break;
		case OSS_2:
			os_delay_us(CONVERSION_TIME*2700);
			break;
		case OSS_3:
			os_delay_us(CONVERSION_TIME*5100);
			break;
		default:
			os_delay_us(CONVERSION_TIME*900);
	}
	int16_t res = BMP180_readExRegister16(BMP180_DATA_REG, resolution);
	return res;
}

bool ICACHE_FLASH_ATTR BMP180_Init()
{
	i2c_init();
	int16_t version = BMP180_readRegister16(BMP180_CHIP_ID_REG);
	if (version != BMP180_CHIP_ID) {
		#ifdef BMP180_DEBUG
		char temp[80];
		os_sprintf(temp, "BMP180: wanted chip id 0x%X, found chip id 0x%X\r\n",
				  BMP180_CHIP_ID, version);
		ets_uart_printf(temp);
		#endif
	    return 0;
	}

	if (!BMP180_readRegister16(BMP180_VERSION_REG))
		return 0;

	#ifdef BMP180_DEBUG
	//os_printf("BMP180 read calibration data...\r\n");
	ets_uart_printf("BMP180 read calibration data...\r\n");
	#endif
	ac1 = BMP180_readRegister16(0xAA);
	ac2 = BMP180_readRegister16(0xAC);
	ac3 = BMP180_readRegister16(0xAE);
	ac4 = BMP180_readRegister16(0xB0);
	ac5 = BMP180_readRegister16(0xB2);
	ac6 = BMP180_readRegister16(0xB4);
	b1 =  BMP180_readRegister16(0xB6);
	b2 =  BMP180_readRegister16(0xB8);
	mb =  BMP180_readRegister16(0xBA);
	mc =  BMP180_readRegister16(0xBC);
	md =  BMP180_readRegister16(0xBE);

	#ifdef BMP180_DEBUG
	ets_uart_printf("BMP180_Calibration:\r\n");
	char temp[128];
	os_sprintf(temp, "AC1: %ld, AC2: %ld, AC3: %ld, AC4: %ld, AC5: %ld, AC6: %ld, B1: %ld, B2: %ld, MB: %ld, MC: %ld, MD: %ld\r\n",
				ac1, ac2, ac3, ac4, ac5, ac6, b1, b2, mb, mc, md);
	ets_uart_printf(temp);
	#endif
	return 1;
}

int32_t ICACHE_FLASH_ATTR BMP180_GetTemperature()
{
	int32_t UT;
	int32_t B5, X1, X2, T;
	UT = BMP180_readRawValue(BMP_CMD_MEASURE_TEMP);
	X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15;
	X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md);
	B5 = X1 + X2;
	T  = (B5+8) >> 4;
	return (T);
}

int32_t ICACHE_FLASH_ATTR BMP180_GetPressure(enum PRESSURE_RESOLUTION resolution)
{
	int32_t UT;
	uint16_t UP;
	int32_t B3, B5, B6;
	uint32_t B4, B7;
	int32_t X1, X2, X3;
	int32_t P;
	UT = BMP180_readRawValue(BMP_CMD_MEASURE_TEMP);
	UP = BMP180_readExRawValue(BMP_CMD_MEASURE_PRESSURE_0+(resolution<<6), resolution);
	// Calculate temperature
	X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15;
	X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md);
	B5 = X1 + X2;
	// Calculate pressure
	B6 = B5 - 4000;
	X1 = ((int32_t)b2 * ((B6 * B6) >> 12)) >> 11;
	X2 = ((int32_t)ac2 * B6) >> 11;
	X3 = X1 + X2;
	B3 = (((((int32_t) ac1) * 4 + X3)<<resolution) + 2)>> 2;
	X1 = ((int32_t)ac3 * B6) >> 13;
	X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
	B7 = ((uint32_t) (UP - B3) * (50000 >> resolution));
	if (B7 < 0x80000000) {
		P = (B7 * 2) / B4;
	} else {
		P = (B7 / B4) * 2;
	}
	X1 = (P >> 8) * (P >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * P) >> 16;
	P = P + ((X1 + X2 + (int32_t)3791) >> 4);
	return P;
}

int32_t ICACHE_FLASH_ATTR BMP180_CalcAltitude(int32_t pressure)
{
	return (int32_t)(pow(((float)MYALTITUDE/44330)+1,5.255F)*pressure);
}

char* ICACHE_FLASH_ATTR BMP180_Int2String(char* buffer, int32_t value)
{
	os_sprintf(buffer, "%d.%d", (int)(value/10), (int)(value%10));
	return buffer;
}

char* ICACHE_FLASH_ATTR BMP180_Float2String(char* buffer, float value)
{
  os_sprintf(buffer, "%d.%d", (int)(value),(int)((value - (int)value)*100));
  return buffer;
}

