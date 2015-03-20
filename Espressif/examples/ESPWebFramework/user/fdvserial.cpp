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

#include "fdv.h"

extern "C" 
{
	#include "ESP8266/uart_register.h"
	#include "ESP8266/gpio_register.h"
}





#define ETS_UART_INTR_ENABLE()  _xt_isr_unmask(1 << ETS_UART_INUM)
#define ETS_UART_INTR_DISABLE() _xt_isr_mask(1 << ETS_UART_INUM)
#define UART_INTR_MASK          0x1ff
#define UART_LINE_INV_MASK      (0x3f<<19)


extern "C" void uart_div_modify(int no, unsigned int freq);



namespace fdv
{

	void FUNC_FLASHMEM dummy_write_char(char c)
	{
	}

	
	void FUNC_FLASHMEM DisableStdOut()
	{
		os_install_putc1(dummy_write_char);	
	}
	
	
	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////
	// Serial


	uint16_t MTD_FLASHMEM Serial::read(uint8_t* buffer, uint16_t bufferLen)
	{
		uint16_t ret = 0;
		for (int16_t c; bufferLen > 0 && (c = read()) > -1; --bufferLen, ++ret)
		{
			*buffer++ = c;
		}
		return ret;
	}			
	

	void MTD_FLASHMEM Serial::writeNewLine()
	{
		write(0x0D);
		write(0x0A);	
	}
	
	
	void MTD_FLASHMEM Serial::write(uint8_t const* buffer, uint16_t bufferLen)
	{
		for (;bufferLen > 0; --bufferLen)
			write(*buffer++);
	}
	
	
	void MTD_FLASHMEM Serial::write(char const* str)
	{
		while (*str)
			write(*str++);
	}
	

	void MTD_FLASHMEM Serial::writeln(char const* str)
	{
		write(str);
		writeNewLine();
	}
	
	
	// buf can stay in RAM or Flash
	// "strings" of args can stay in RAM or Flash
	uint16_t MTD_FLASHMEM Serial::printf(char const *fmt, ...)
	{
		va_list args;
		
		va_start(args, fmt);
		uint16_t len = vsprintf(NULL, fmt, args);
		va_end(args);

		char buf[len + 1];
		
		va_start(args, fmt);
		vsprintf(buf, fmt, args);
		va_end(args);
		
		write(buf);

		return len;
	}
	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// HardwareSerial

	
	HardwareSerial* HardwareSerial::s_serials[1] = {0};
	

	HardwareSerial* MTD_FLASHMEM HardwareSerial::getSerial(uint32_t uart)
	{
		if (s_serials[uart] == NULL)
			s_serials[uart] = new HardwareSerial;
		return s_serials[uart];
	}
	
	
	// call only from ISR
	void MTD_FLASHMEM HardwareSerial::put(uint8_t value)
	{
		m_queue.sendFromISR(value);
	}
	
	
	int16_t MTD_FLASHMEM HardwareSerial::peek()
	{
		uint8_t ret;
		if (m_queue.peek(&ret, 0))
			return ret;
		return -1;
	}
	
	
	int16_t MTD_FLASHMEM HardwareSerial::read()
	{
		uint8_t ret;
		if (m_queue.receive(&ret, 0))
			return ret;
		return -1;				
	}
	
	
	uint16_t MTD_FLASHMEM HardwareSerial::available()
	{
		return m_queue.available();
	}
	
	
	void MTD_FLASHMEM HardwareSerial::flush()
	{
		m_queue.clear();
	}
	
	
	bool MTD_FLASHMEM HardwareSerial::waitForData(uint32_t timeOutMs)
	{
		uint8_t b;
		return m_queue.peek(&b, timeOutMs);
	}
	
	
	void HardwareSerial_rx_handler()
	{
		uint32_t uart_intr_status = READ_PERI_REG(UART_INT_ST(0));

		while (uart_intr_status != 0) 
		{
			if (UART_RXFIFO_FULL_INT_ST == (uart_intr_status & UART_RXFIFO_FULL_INT_ST)) 
			{
				while (READ_PERI_REG(UART_STATUS(0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S))
				{
					uint8_t recByte = READ_PERI_REG(UART_FIFO(0)) & 0xFF;
					HardwareSerial::getSerial(0)->put(recByte);
				}				
				WRITE_PERI_REG(UART_INT_CLR(0), UART_RXFIFO_FULL_INT_CLR);
			}
			uart_intr_status = READ_PERI_REG(UART_INT_ST(0));
		}
	}
	
	
	void MTD_FLASHMEM HardwareSerial::reconfig(uint32_t baud_rate)
	{
		// wait tx fifo empty
		while (READ_PERI_REG(UART_STATUS(0)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S));

		PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD);
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);

		// no flow ctrl
		CLEAR_PERI_REG_MASK(UART_CONF0(0), UART_TX_FLOW_EN);
		CLEAR_PERI_REG_MASK(UART_CONF1(0), UART_RX_FLOW_EN);
		
		uart_div_modify(0, UART_CLK_FREQ / baud_rate);

		WRITE_PERI_REG(UART_CONF0(0),
					     (0x1 << UART_STOP_BIT_NUM_S)	// 1 stop bit
					   | (0x3 << UART_BIT_NUM_S));		// 8 data bits

		// reset fifo
		SET_PERI_REG_MASK(UART_CONF0(0), UART_RXFIFO_RST | UART_TXFIFO_RST);
		CLEAR_PERI_REG_MASK(UART_CONF0(0), UART_RXFIFO_RST | UART_TXFIFO_RST);		
		
		uint8_t const UART_RX_TimeOutIntrThresh   = 2;
		uint8_t const UART_TX_FifoEmptyIntrThresh = 20;
		uint8_t const UART_RX_FifoFullIntrThresh  = 1;
		uint32_t reg_val = 0;		
		WRITE_PERI_REG(UART_INT_CLR(0), UART_INTR_MASK);		
		reg_val = READ_PERI_REG(UART_CONF1(0)) & ~((UART_RX_FLOW_THRHD << UART_RX_FLOW_THRHD_S) | UART_RX_FLOW_EN) ;
		reg_val |= ((UART_RX_TimeOutIntrThresh & UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S) | UART_RX_TOUT_EN;
		reg_val |= (UART_RX_FifoFullIntrThresh & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S;
		reg_val |= (UART_TX_FifoEmptyIntrThresh & UART_TXFIFO_EMPTY_THRHD) << UART_TXFIFO_EMPTY_THRHD_S;
		WRITE_PERI_REG(UART_CONF1(0), reg_val);
		
		CLEAR_PERI_REG_MASK(UART_INT_ENA(0), UART_INTR_MASK);		
		SET_PERI_REG_MASK(UART_INT_ENA(0), UART_RXFIFO_FULL_INT_ENA);		
		_xt_isr_attach(ETS_UART_INUM, HardwareSerial_rx_handler);
		
		ETS_UART_INTR_ENABLE();		
	}

	
	void MTD_FLASHMEM HardwareSerial::write(uint8_t b)
	{
		while (true)
		{
			uint32_t fifo_cnt = READ_PERI_REG(UART_STATUS(0)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);
			if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126)
				break;
		}
		WRITE_PERI_REG(UART_FIFO(0) , b);
	}
	

	// HardwareSerial
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
}