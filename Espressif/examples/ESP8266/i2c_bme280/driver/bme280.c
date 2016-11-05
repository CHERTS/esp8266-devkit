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

#include "driver/bme280.h"

#define ID_REG		0xD0
#define PRESS_MSB_REG	0xF7
#define CALIB_00_REG	0x88
#define CALIB_26_REG	0xE1
#define CONFIG_REG	0xF5
#define CTRL_MEAS_REG	0xF4
#define STATUS_REG	0xF3
#define CTRL_HUM_REG	0xF2

static struct {
	uint16 dig_T1;
	uint16 dig_T2;
	uint16 dig_T3;

	uint16 dig_P1;
	sint16 dig_P2;
	sint16 dig_P3;
	sint16 dig_P4;
	sint16 dig_P5;
	sint16 dig_P6;
	sint16 dig_P7;
	sint16 dig_P8;
	sint16 dig_P9;

	uint8  dig_H1;
	sint16 dig_H2;
	uint8  dig_H3;
	sint16 dig_H4;
	sint16 dig_H5;
	sint8  dig_H6;
} CalibrationParam;

sint32 t_fine;		// t_fine carries fine temperature as global value

/**********************************************************************
Return: 0 	  - Everything OK
		non 0 - Failed
Parameters:	os_t - Temperature Oversampling
			os_p - Pressure Oversampling
			os_h - Humidity Oversampling
			filter - Filter coefficient
			mode - Mode (Sleep/Forced/Normal)
			t_sb - Standby time between conversions
**********************************************************************/
uint8 BME280_Init(uint8 os_t, uint8 os_p, uint8 os_h,
					uint8 filter, uint8 mode, uint8 t_sb)
{
	uint8 ID = 0;
	uint8 Buff[26] = {0};
	uint8 Temp;

	I2C_ReadData(BME280_I2C_ADDR, ID_REG, &ID, 1);
	if(ID != BME280_CHIP_ID)
		return 1;

	/* Delay added concerning the low speed of power up system to
		facilitate the proper reading of the chip ID */
	os_delay_us(1000*BME280_REGISTER_READ_DELAY);

	I2C_ReadData(BME280_I2C_ADDR, CALIB_00_REG, Buff, 26);

	//ToDo: test im_update bit
	CalibrationParam.dig_T1 = (Buff[1] << 8) | Buff[0];
	CalibrationParam.dig_T2 = (Buff[3] << 8) | Buff[2];
	CalibrationParam.dig_T3 = (Buff[5] << 8) | Buff[4];

	CalibrationParam.dig_P1 = (Buff[7] << 8) | Buff[6];
	CalibrationParam.dig_P2 = (Buff[9] << 8) | Buff[8];
	CalibrationParam.dig_P3 = (Buff[11] << 8) | Buff[10];
	CalibrationParam.dig_P4 = (Buff[13] << 8) | Buff[12];
	CalibrationParam.dig_P5 = (Buff[15] << 8) | Buff[14];
	CalibrationParam.dig_P6 = (Buff[17] << 8) | Buff[16];
	CalibrationParam.dig_P7 = (Buff[19] << 8) | Buff[18];
	CalibrationParam.dig_P8 = (Buff[21] << 8) | Buff[20];
	CalibrationParam.dig_P9 = (Buff[23] << 8) | Buff[22];

	CalibrationParam.dig_H1 = Buff[25];

	memset (Buff, 0, 7);
	I2C_ReadData(BME280_I2C_ADDR, CALIB_26_REG, Buff, 7);

	CalibrationParam.dig_H2 = (Buff[1] << 8) | Buff[0];
	CalibrationParam.dig_H3 = Buff[2];
	CalibrationParam.dig_H4 = (Buff[3] << 4) | (Buff[4] & 0x0F);
	CalibrationParam.dig_H5 = (Buff[5] << 4) | (Buff[4] >> 4);
	CalibrationParam.dig_H6 = Buff[6];

	Temp = (t_sb << 5) | ((filter & 0x07) << 2);				//config
	I2C_WriteData(BME280_I2C_ADDR, CONFIG_REG, &Temp, 1);

	Temp = os_h & 0x07;							//hum
	I2C_WriteData(BME280_I2C_ADDR, CTRL_HUM_REG, &Temp, 1);

	Temp = (os_t << 5) | ((os_p & 0x07) << 2) | (mode & 0x03);		//meas
	I2C_WriteData(BME280_I2C_ADDR, CTRL_MEAS_REG, &Temp, 1);

	return 0;
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
sint32 BME280_CompensateT(sint32 adc_T)
{
	sint32 var1, var2, T;

	var1 = ((((adc_T>>3) - ((sint32)CalibrationParam.dig_T1<<1))) * ((sint32)CalibrationParam.dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((sint32)CalibrationParam.dig_T1)) * ((adc_T>>4) - ((sint32)CalibrationParam.dig_T1))) >> 12) * ((sint32)CalibrationParam.dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;

	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
uint32 BME280_CompensateP(sint32 adc_P)
{
	sint64 var1, var2, p;

	var1 = ((sint64)t_fine) - 128000;
	var2 = var1 * var1 * (sint64)CalibrationParam.dig_P6;
	var2 = var2 + ((var1*(sint64)CalibrationParam.dig_P5)<<17);
	var2 = var2 + (((sint64)CalibrationParam.dig_P4)<<35);
	var1 = ((var1 * var1 * (sint64)CalibrationParam.dig_P3)>>8) + ((var1 * (sint64)CalibrationParam.dig_P2)<<12);
	var1 = (((((sint64)1)<<47) + var1)) * ((sint64)CalibrationParam.dig_P1)>>33;

	if (var1 == 0)
		return 0; 	//avoid exception caused by division by zero

	p = 1048576 - adc_P;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((sint64)CalibrationParam.dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((sint64)CalibrationParam.dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((sint64)CalibrationParam.dig_P7)<<4);

	return (uint32)p;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of “47445” represents 47445/1024 = 46.333 %RH
uint32 BME280_CompensateH(sint32 adc_H)
{
	sint32 v_x1_u32r;

	v_x1_u32r = (t_fine - ((sint32)76800));
	v_x1_u32r = (((((adc_H << 14) - (((sint32)CalibrationParam.dig_H4) << 20) - (((sint32)CalibrationParam.dig_H5) * v_x1_u32r)) +
		((sint32)16384)) >> 15) * (((((((v_x1_u32r * ((sint32)CalibrationParam.dig_H6)) >> 10) * (((v_x1_u32r *
		((sint32)CalibrationParam.dig_H3)) >> 11) + ((sint32)32768))) >> 10) + ((sint32)2097152)) *
		((sint32)CalibrationParam.dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((sint32)CalibrationParam.dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

	return (uint32)(v_x1_u32r>>12);
}

/**********************************************************************
Return: 0 	  - Everything OK
		non 0 - Failed
Parameters:	t - Pointer to variable in which to write the temperature
			p - Pointer to variable in which to write the pressure
			h - Pointer to variable in which to write the humidity
**********************************************************************/
uint8 BME280_ReadAll(sint32* t, uint32* p, uint32* h)
{
	uint8 Buff[8] = {0};
	sint32 UncT, UncP, UncH;

	if(I2C_ReadData(BME280_I2C_ADDR, PRESS_MSB_REG, Buff, 8))
		return 1;

	UncP = ((uint32)Buff[0] << 16) | ((uint16)Buff[1] << 8) | Buff[2];
	UncP >>= 4;

	UncT = ((uint32)Buff[3] << 16) | ((uint16)Buff[4] << 8) | Buff[5];
	UncT >>= 4;

	UncH = ((uint16)Buff[6] << 8) | Buff[7];

	*t = BME280_CompensateT(UncT);
	*p = BME280_CompensateP(UncP);
	*h = BME280_CompensateH(UncH);

	return 0;
}

/**********************************************************************
Return: 0 	  - Everything OK
		non 0 - Failed
Parameters:	mode - Mode (Sleep/Forced/Normal)
**********************************************************************/
uint8 BME280_SetMode(uint8 mode)
{
	uint8 RegVal = 0;

	mode &= 0x03;
	I2C_ReadData(BME280_I2C_ADDR, CTRL_MEAS_REG, &RegVal, 1);
	RegVal &= 0xFC;
	RegVal |= mode;
	return I2C_WriteData(BME280_I2C_ADDR, CTRL_MEAS_REG, &RegVal, 1);
}
