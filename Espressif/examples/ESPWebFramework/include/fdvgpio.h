/*
# Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com)
# Copyright (c) 2015 Fabrizio Di Vittorio.
# All rights reserved.

# GNU GPL LICENSE
#
# This module is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; latest version thereof,
# available at: <http://www.gnu.org/licenses/gpl.txt>.
#
# This module is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this module; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*/

/*
Parts from gpio.h/gpio.c driver: Copyright (C) 2014 - 2016  Espressif System
*/


#ifndef _FDVGPIO_H_
#define _FDVGPIO_H_

#include "fdv.h"


extern "C"
{
	#include "esp8266\pin_mux_register.h"
	#include "esp8266\eagle_soc.h"
}


namespace fdv
{	
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// GPIO
	//
	// ON/OFF frequency up to 5.7 MHz.
	//
	// Applications should use only following GPIOs:
	//
	//   GPIO0
	//   GPIO2
	//   GPIO4
	//   GPIO5
	//   GPIO12
	//   GPIO13
	//   GPIO14
	//   GPIO15
	//   GPIO16
	//
	// Applications should NOT use following GPIOs:
	//
	//   GPIO1:   Used for UART 0 - TX
	//   GPIO3:	  Used for UART 0 - RX
	//   GPIO6:   Used for Flash
	//   GPIO7:	  Used for Flash 
	//   GPIO8:	  Used for Flash 
	//   GPIO9:	  Used for Flash 
	//   GPIO10:  Used for Flash 
	//   GPIO11:  Used for Flash 
	//
	//
	// Examples:
	//
	//   // switch GPIO2 on and off every 1s
	//   GPIO gpio2(2);
	//   gpio2.modeOutput();
	//   while (1) {
	//     gpio2.writeHigh();
	//     Task::delay(1000);
	//     gpio2.writeLow();
	//     Task::delay(1000);
	//   }
	//
	//   // replicate GPIO0 input to GPIO2 output
	//   GPIO gpio0(0), gpio2(2);
	//   gpio0.modeInput();
	//   gpio2.modeOutput();
	//   while (1) {
	//	   gpio2.write( gpio0.read() );
	//   }

	class GPIO
	{

	public:
		// gpioNum : 0..15
		GPIO(uint32_t gpioNum)
			: m_gpioNum(gpioNum)
		{
			uint32_t pinReg[16] = {PERIPHS_IO_MUX_GPIO0_U, PERIPHS_IO_MUX_U0TXD_U, PERIPHS_IO_MUX_GPIO2_U, PERIPHS_IO_MUX_U0RXD_U, 
								   PERIPHS_IO_MUX_GPIO4_U, PERIPHS_IO_MUX_GPIO5_U, PERIPHS_IO_MUX_SD_CLK_U, PERIPHS_IO_MUX_SD_DATA0_U, 
								   PERIPHS_IO_MUX_SD_DATA1_U, PERIPHS_IO_MUX_SD_DATA2_U, PERIPHS_IO_MUX_SD_DATA3_U, PERIPHS_IO_MUX_SD_CMD_U, 
								   PERIPHS_IO_MUX_MTDI_U, PERIPHS_IO_MUX_MTCK_U, PERIPHS_IO_MUX_MTMS_U, PERIPHS_IO_MUX_MTDO_U};
			uint8_t pinFunc[16] = {FUNC_GPIO0, FUNC_GPIO1, FUNC_GPIO2, FUNC_GPIO3, FUNC_GPIO4, FUNC_GPIO5, FUNC_GPIO6, FUNC_GPIO7, FUNC_GPIO8,
								   FUNC_GPIO9, FUNC_GPIO10, FUNC_GPIO11, FUNC_GPIO12, FUNC_GPIO13, FUNC_GPIO14, FUNC_GPIO15};
			m_pinReg  = pinReg[gpioNum];
			m_pinFunc = pinFunc[gpioNum];
		}
		
		void MTD_FLASHMEM modeInput()
		{
			setupAsGPIO();
			GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 0);
			GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 0);
			GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, 0);
			GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, 1 << m_gpioNum);
		}
		
		void MTD_FLASHMEM modeOutput()
		{
			setupAsGPIO();
			GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 0);
			GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 0);
			GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, 1 << m_gpioNum);
			GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, 0);
		}
		
		void MTD_FLASHMEM enablePullUp(bool value)
		{
			if (value)
				PIN_PULLUP_EN(m_pinReg);
			else
				PIN_PULLUP_DIS(m_pinReg);
		}
		
		void MTD_FLASHMEM write(bool value)
		{
			if (value)
				GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << m_gpioNum);
			else
				GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << m_gpioNum);
		}
		
		void MTD_FLASHMEM writeLow()
		{
			GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << m_gpioNum);
		}
		
		void MTD_FLASHMEM writeHigh()
		{
			GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << m_gpioNum);
		}
		
		bool MTD_FLASHMEM read()
		{
			return (bool)((GPIO_REG_READ(GPIO_IN_ADDRESS) >> m_gpioNum) & 1);
		}
		
	private:
		void MTD_FLASHMEM setupAsGPIO()
		{
			PIN_FUNC_SELECT(m_pinReg, m_pinFunc);
			enablePullUp(false);
		}
		
	private:
		uint32_t m_pinReg;
		uint8_t  m_gpioNum;
		uint8_t  m_pinFunc;
	};



	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// GPIO16 (specific for GPIO16 pin)
	
	class GPIO16
	{

	public:

		void MTD_FLASHMEM modeInput()
		{
			WRITE_PERI_REG(PAD_XPD_DCDC_CONF, (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32_t)0x1);
			WRITE_PERI_REG(RTC_GPIO_CONF, (READ_PERI_REG(RTC_GPIO_CONF) & (uint32_t)0xfffffffe) | (uint32_t)0x0);
			WRITE_PERI_REG(RTC_GPIO_ENABLE, READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32_t)0xfffffffe);
		}
		
		void MTD_FLASHMEM modeOutput()
		{
			WRITE_PERI_REG(PAD_XPD_DCDC_CONF, (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32_t)0x1);
			WRITE_PERI_REG(RTC_GPIO_CONF, (READ_PERI_REG(RTC_GPIO_CONF) & (uint32_t)0xfffffffe) | (uint32_t)0x0);
			WRITE_PERI_REG(RTC_GPIO_ENABLE, (READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32_t)0xfffffffe) | (uint32_t)0x1);
		}
		
		void MTD_FLASHMEM write(bool value)
		{
			WRITE_PERI_REG(RTC_GPIO_OUT, (READ_PERI_REG(RTC_GPIO_OUT) & (uint32_t)0xfffffffe) | (uint32_t)((uint8_t)value & 1));
		}
		
		void MTD_FLASHMEM writeLow()
		{
			WRITE_PERI_REG(RTC_GPIO_OUT, (READ_PERI_REG(RTC_GPIO_OUT) & (uint32_t)0xfffffffe));
		}
		
		void MTD_FLASHMEM writeHigh()
		{
			WRITE_PERI_REG(RTC_GPIO_OUT, (READ_PERI_REG(RTC_GPIO_OUT) & (uint32_t)0xfffffffe) | (uint32_t)(1));
		}

		bool MTD_FLASHMEM read()
		{
			return (bool)(READ_PERI_REG(RTC_GPIO_IN_DATA) & 1);
		}		
	};


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// GPIOX
	// Embeds functionalities of GPIO and GPIO16
	
	class GPIOX
	{
	public:
	
		// gpioNum 0..16
		GPIOX(uint32_t gpioNum)
			: m_gpioNum(gpioNum)
		{
		}

		void MTD_FLASHMEM modeInput()
		{
			if (m_gpioNum != 16)
				GPIO(m_gpioNum).modeInput();
			else
				GPIO16().modeInput();
		}
		
		void MTD_FLASHMEM modeOutput()
		{
			if (m_gpioNum != 16)
				GPIO(m_gpioNum).modeOutput();
			else
				GPIO16().modeOutput();	
		}
		
		void MTD_FLASHMEM enablePullUp(bool value)
		{
			if (m_gpioNum != 16)
				GPIO(m_gpioNum).enablePullUp(value);
		}
		
		void MTD_FLASHMEM write(bool value)
		{
			if (m_gpioNum != 16)
				GPIO(m_gpioNum).write(value);
			else
				GPIO16().write(value);
		}
		
		void MTD_FLASHMEM writeLow()
		{
			if (m_gpioNum != 16)
				GPIO(m_gpioNum).writeLow();
			else
				GPIO16().writeLow();
		}
		
		void MTD_FLASHMEM writeHigh()
		{
			if (m_gpioNum != 16)
				GPIO(m_gpioNum).writeHigh();
			else
				GPIO16().writeHigh();
		}
		
		bool MTD_FLASHMEM read()
		{
			if (m_gpioNum != 16)
				return GPIO(m_gpioNum).read();
			else
				return GPIO16().read();
		}
		
	private:
		uint32_t m_gpioNum;
	};
	
}

#endif

