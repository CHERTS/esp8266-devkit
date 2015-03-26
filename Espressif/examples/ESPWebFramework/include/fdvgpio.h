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
}


namespace fdv
{
	
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// GPIOn classes
	//
	// Examples:
	//
	//   // switch GPIO2 on and off every 1s
	//   GPIO2::modeOutput();
	//   while (1) {
	//     GPIO2::write(true);
	//     Task::delay(1000);
	//     GPIO2::write(false);
	//     Task::delay(1000);
	//   }
	//
	//   // replicate GPIO0 input to GPIO2 output
	//   GPIO0::modeInput();
	//   GPIO2::modeOutput();
	//   while (1) {
	//	   GPIO2::write( GPIO0::read() );
	//   }
	
	
	// GPIONUM_V = 0..16 (16=special case)
	// PINNAME_V = PERIPHS_IO_MUX_GPIO0_U, PERIPHS_IO_MUX_U0TXD_U, etc...
	// PINFUNC_V = FUNC_GPIO0, FUNC_GPIO2, etc...
	template <uint32_t GPIONUM_V, uint32_t PINNAME_V, uint32_t PINFUNC_V>
	struct GPIO
	{
		static void STC_FLASHMEM modeInput()
		{
			setupAsGPIO();
			if (GPIONUM_V < 16)
			{
				GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 0);
				GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 0);
				GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, 0);
				GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, GPIONUM_V);
			}
			else
			{
				WRITE_PERI_REG(PAD_XPD_DCDC_CONF, (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32_t)0x1);
				WRITE_PERI_REG(RTC_GPIO_CONF, (READ_PERI_REG(RTC_GPIO_CONF) & (uint32_t)0xfffffffe) | (uint32_t)0x0);
				WRITE_PERI_REG(RTC_GPIO_ENABLE, READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32_t)0xfffffffe);
			}
		}
		
		static void STC_FLASHMEM modeOutput()
		{
			setupAsGPIO();
			if (GPIONUM_V < 16)
			{
				GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 0);
				GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 0);
				GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, GPIONUM_V);
				GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, 0);
		    }
			else
			{
				WRITE_PERI_REG(PAD_XPD_DCDC_CONF, (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32_t)0x1);
				WRITE_PERI_REG(RTC_GPIO_CONF, (READ_PERI_REG(RTC_GPIO_CONF) & (uint32_t)0xfffffffe) | (uint32_t)0x0);
				WRITE_PERI_REG(RTC_GPIO_ENABLE, (READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32_t)0xfffffffe) | (uint32_t)0x1);
			}
		}
		
		static void STC_FLASHMEM enablePullUp()
		{
			if (GPIONUM_V < 16)
				PIN_PULLUP_EN(PINNAME_V);
		}
		
		static void STC_FLASHMEM disablePullUp()
		{
			if (GPIONUM_V < 16)
				PIN_PULLUP_DIS(PINNAME_V);
		}
		
		static void STC_FLASHMEM write(bool value)
		{
			if (GPIONUM_V < 16)
			{
				GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, (uint8_t)value << GPIONUM_V);
				GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, ((~(uint8_t)value) & 0x01) << GPIONUM_V);
				GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, 1 << GPIONUM_V);
				GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, 0);
			}
			else
			{
				WRITE_PERI_REG(RTC_GPIO_OUT, (READ_PERI_REG(RTC_GPIO_OUT) & (uint32_t)0xfffffffe) | (uint32_t)((uint8_t)value & 1));
			}
		}
		
		static bool STC_FLASHMEM read()
		{
			if (GPIONUM_V < 16)
				return (bool)((GPIO_REG_READ(GPIO_IN_ADDRESS) >> GPIONUM_V) & 1);
			else
				return (bool)(READ_PERI_REG(RTC_GPIO_IN_DATA) & 1);
		}
		
	private:
		static void STC_FLASHMEM setupAsGPIO()
		{
			if (GPIONUM_V < 16)
				PIN_FUNC_SELECT(PINNAME_V, PINFUNC_V);
		}
		
	};
	
	
	typedef GPIO<0, PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0>      GPIO0;
	typedef GPIO<1, PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1>      GPIO1;		// Don't use! UART 0 - TX
	typedef GPIO<2, PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2>      GPIO2;
	typedef GPIO<3, PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3>      GPIO3;		// Don't use! UART 0 - RX
	typedef GPIO<4, PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4>      GPIO4;
	typedef GPIO<5, PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5>      GPIO5;
	typedef GPIO<6, PERIPHS_IO_MUX_SD_CLK_U, FUNC_GPIO6>     GPIO6;		// Don't use! Used for Flash
	typedef GPIO<7, PERIPHS_IO_MUX_SD_DATA0_U, FUNC_GPIO7>   GPIO7;		// Don't use! Used for Flash 
	typedef GPIO<8, PERIPHS_IO_MUX_SD_DATA1_U, FUNC_GPIO8>   GPIO8;		// Don't use! Used for Flash 
	typedef GPIO<9, PERIPHS_IO_MUX_SD_DATA2_U, FUNC_GPIO9>   GPIO9;		// Don't use! Used for Flash 
	typedef GPIO<10, PERIPHS_IO_MUX_SD_DATA3_U, FUNC_GPIO10> GPIO10;	// Don't use! Used for Flash 
	typedef GPIO<11, PERIPHS_IO_MUX_SD_CMD_U, FUNC_GPIO11>   GPIO11;	// Don't use! Used for Flash 
	typedef GPIO<12, PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12>     GPIO12;
	typedef GPIO<13, PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13>     GPIO13;
	typedef GPIO<14, PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14>     GPIO14;
	typedef GPIO<15, PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15>     GPIO15;
	typedef GPIO<16, 0, 0>                                   GPIO16;
	
	
	
	
	
}

#endif

