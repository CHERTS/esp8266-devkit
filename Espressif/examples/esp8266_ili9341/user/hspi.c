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

static void hspi_setAmountBit(uint8_t amountByte)
{
	uint16_t numberBit = amountByte * 8 - 1;
	WRITE_PERI_REG(SPI_FLASH_USER1(HSPI),
			( (numberBit & SPI_USR_OUT_BITLEN) << SPI_USR_OUT_BITLEN_S ) |
			( (numberBit & SPI_USR_DIN_BITLEN) << SPI_USR_DIN_BITLEN_S ) );
}

static void hspi_loadToBuffer(uint8_t *data, uint8_t numberByte)
{
	uint8_t i = 0;
	uint8_t amount4byte = (numberByte / 4) + 1;

	hspi_setAmountBit(numberByte);

	for (i = 0; i < amount4byte; ++i)
	{
	    ((uint32_t *)SPI_FLASH_C0(HSPI)) [i]= ((uint32_t *)data) [i];
	}
}

static void hspi_readFromBuffer(uint8_t * data, uint8_t numberByte)
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

static void hspi_startSend(void)
{
	SET_PERI_REG_MASK(SPI_FLASH_CMD(HSPI), SPI_FLASH_USR);
}

static void hspi_whaitReady(void)
{
	while (READ_PERI_REG(SPI_FLASH_CMD(HSPI)) & SPI_FLASH_USR) {};
}


static void hspi_config(int configState)
{
	uint32_t valueOfRegisters = 0;

	hspi_whaitReady();

	if (configState == CONFIG_FOR_TX)
	{
		valueOfRegisters |=  SPI_FLASH_DOUT;
		valueOfRegisters &= ~(BIT2 | SPI_FLASH_USR_ADDR | SPI_FLASH_USR_DUMMY | SPI_FLASH_USR_DIN | SPI_USR_COMMAND | SPI_DOUTDIN); //clear bit 2 see example IoT_Demo
	}
	else if  (configState == CONFIG_FOR_RX_TX)
	{
		valueOfRegisters |=  SPI_FLASH_DOUT | SPI_DOUTDIN | SPI_CK_I_EDGE;
		valueOfRegisters &= ~(BIT2 | SPI_FLASH_USR_ADDR | SPI_FLASH_USR_DUMMY | SPI_FLASH_USR_DIN | SPI_USR_COMMAND); //clear bit 2 see example IoT_Demo
	}
	else
	{
		return;
	}
	WRITE_PERI_REG(SPI_FLASH_USER(HSPI), valueOfRegisters);
}

void hspi_TxRx(uint8_t * data, uint8_t numberByte)
{
	hspi_config(CONFIG_FOR_RX_TX);
	hspi_loadToBuffer(data, numberByte);
	hspi_startSend();
	hspi_whaitReady();
	hspi_readFromBuffer(data, numberByte);
}

void hspi_Tx(uint8_t * data, uint8_t numberByte, uint32_t numberRepeat)
{
	uint32_t i = 0;
	uint32_t j = 0;

	if ((numberByte < 1) || (numberRepeat < 1))
		return;	// Error parameter

	hspi_config(CONFIG_FOR_TX);

	if (numberRepeat > 1)
	{
		uint8_t package[MAX_SIZE_BUFFER];
		uint8_t amountByteAtPackage = 0;
		uint8_t amountDataAtPackage = MAX_SIZE_BUFFER /  numberByte;

		if (numberByte > MAX_SIZE_BUFFER)
			return; // Require describe this case or exit :)

		if (amountDataAtPackage > numberRepeat)
			amountDataAtPackage = numberRepeat;

		for (i = 0; i < amountDataAtPackage; ++i)
			for (j = 0; j < numberByte; ++j)
			{
				package[amountByteAtPackage] = data[j];
				amountByteAtPackage++;
			}

		hspi_loadToBuffer(package, amountByteAtPackage);

		// send main part of data

		for (i = 0; i < numberRepeat; i += amountDataAtPackage)
		{
			hspi_startSend();
			hspi_whaitReady();
		}

		// send residual part of data

		if (numberRepeat % amountDataAtPackage != 0)
		{
			hspi_setAmountBit((numberRepeat % amountDataAtPackage) * amountByteAtPackage);
			hspi_startSend();
			hspi_whaitReady();
		}

	}
	else
	{
		uint8_t byteAtPackage = (numberByte < MAX_SIZE_BUFFER) ? numberByte : MAX_SIZE_BUFFER;

		// send main part of data

		for (i = 0; i < numberByte - numberByte % byteAtPackage; i += byteAtPackage)
		{
			hspi_loadToBuffer(&data[i], byteAtPackage);
			hspi_startSend();
			hspi_whaitReady();
		}

		// send residual part of data

		if (numberByte % byteAtPackage != 0)
		{
			hspi_loadToBuffer(&data[numberByte - numberByte % byteAtPackage], numberByte % byteAtPackage);
			hspi_startSend();
			hspi_whaitReady();
		}

	}
}
