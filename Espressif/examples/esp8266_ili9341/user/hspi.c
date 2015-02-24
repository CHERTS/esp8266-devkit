/*
 * hspi.c
 *
 *  Created on: 12 џэт. 2015 у.
 *      Author: Sem
 */

#include "hspi.h"

void hspi_init(void)
{
	WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105); //clear bit9

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2); // HSPIQ MISO
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); // HSPID MOSI
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); // CLK


	// SPI clock = CPU clock / 10 / 4
	// time length HIGHT level = (CPU clock / 10 / 2) ^ -1,
	// time length LOW level = (CPU clock / 10 / 2) ^ -1
	WRITE_PERI_REG(SPI_FLASH_CLOCK(HSPI),
	   ((1 & SPI_CLKDIV_PRE) << SPI_CLKDIV_PRE_S) |
	   ((3 & SPI_CLKCNT_N) << SPI_CLKCNT_N_S) |
	   ((1 & SPI_CLKCNT_H) << SPI_CLKCNT_H_S) |
	   ((3 & SPI_CLKCNT_L) << SPI_CLKCNT_L_S));
}

static void writeDataToBuffer(uint8_t *data, uint8_t numberByte)
{
	uint8_t i = 0;
	uint8_t amount4byte = (numberByte / 4) + 1;

	for (i = 0; i < amount4byte; ++i)
	{
	    ((uint32_t *)SPI_FLASH_C0(HSPI)) [i]= ((uint32_t *)data) [i];
	}
}

static void readDataFromBuffer(uint8_t * data, uint8_t numberByte)
{

	uint8_t i = 0;
	uint8_t shift = 0;
	uint32_t buffer = 0;

	for (i = 0; i < numberByte; ++i)
	{
		if ( (i % 4 == 0) )
		{
			buffer = ( *( (uint32_t *)SPI_FLASH_C0(HSPI) + i/4 ) );
			shift = 0;
		}
		data[i] = (buffer >> shift) & 0xFF;
		shift += 8;
	}
}

void hspi_TxRx(uint8_t * data, uint8_t numberByte)
{
	uint32_t regvalue = 0;
	uint16_t numberBit = 0;

	while (READ_PERI_REG(SPI_FLASH_CMD(HSPI))&SPI_FLASH_USR);

	regvalue |=  SPI_FLASH_DOUT | SPI_DOUTDIN | SPI_CK_I_EDGE;
    regvalue &= ~(BIT2 | SPI_FLASH_USR_ADDR | SPI_FLASH_USR_DUMMY | SPI_FLASH_USR_DIN | SPI_USR_COMMAND); //clear bit 2 see example IoT_Demo
	WRITE_PERI_REG(SPI_FLASH_USER(HSPI), regvalue);

	numberBit = numberByte * 8 - 1;

	WRITE_PERI_REG(SPI_FLASH_USER1(HSPI),
			( (numberBit & SPI_USR_OUT_BITLEN) << SPI_USR_OUT_BITLEN_S ) |
			( (numberBit & SPI_USR_DIN_BITLEN) << SPI_USR_DIN_BITLEN_S ) );

	writeDataToBuffer(data, numberByte);

	SET_PERI_REG_MASK(SPI_FLASH_CMD(HSPI), SPI_FLASH_USR);   // send

	while (READ_PERI_REG(SPI_FLASH_CMD(HSPI)) & SPI_FLASH_USR);

	readDataFromBuffer(data, numberByte);
}


void hspi_Tx(uint8_t * data, uint8_t numberByte, uint32_t numberRepeat)
{
#ifdef USE_HARD_OPTIMIZATION

	uint8_t dataBuffer[MAX_SIZE_BUFFER];
	uint8_t j = 0;
	uint8_t k = 0;

#endif
	uint32_t i = 0;


	uint32_t regvalue = 0;
	uint16_t numberBit = 0;

	while (READ_PERI_REG(SPI_FLASH_CMD(HSPI))&SPI_FLASH_USR);

	regvalue |=  SPI_FLASH_DOUT;
    regvalue &= ~(BIT2 | SPI_FLASH_USR_ADDR | SPI_FLASH_USR_DUMMY | SPI_FLASH_USR_DIN | SPI_USR_COMMAND | SPI_DOUTDIN); //clear bit 2 see example IoT_Demo
	WRITE_PERI_REG(SPI_FLASH_USER(HSPI), regvalue);


	if (numberByte > MAX_SIZE_BUFFER)
	{
		return;	// Need describe this case or exit :)
	}

#ifdef USE_HARD_OPTIMIZATION

	for (i = 0; i < numberRepeat; i ++)
	{
		for (j = 0; j < numberByte; j++)
		{
			dataBuffer[k] = data[j];
			k++;
		}
		if (k >= (MAX_SIZE_BUFFER - MAX_SIZE_BUFFER % numberByte) || (i == (numberRepeat - 1)))
		{
			numberBit = k * 8 - 1;
			WRITE_PERI_REG(SPI_FLASH_USER1(HSPI), (numberBit & SPI_USR_OUT_BITLEN) << SPI_USR_OUT_BITLEN_S );

			writeDataToBuffer(dataBuffer, k);

			SET_PERI_REG_MASK(SPI_FLASH_CMD(HSPI), SPI_FLASH_USR);   // send

			while (READ_PERI_REG(SPI_FLASH_CMD(HSPI)) & SPI_FLASH_USR);

			k = 0;
		}
	}

#else

	numberBit = numberByte * 8 - 1;

	WRITE_PERI_REG(SPI_FLASH_USER1(HSPI), (numberBit & SPI_USR_OUT_BITLEN) << SPI_USR_OUT_BITLEN_S );

	writeDataToBuffer(data, numberByte);

	for (i = 0; i < numberRepeat; i ++)
	{
		SET_PERI_REG_MASK(SPI_FLASH_CMD(HSPI), SPI_FLASH_USR);   // send
		while (READ_PERI_REG(SPI_FLASH_CMD(HSPI)) & SPI_FLASH_USR);
	}

#endif
}
