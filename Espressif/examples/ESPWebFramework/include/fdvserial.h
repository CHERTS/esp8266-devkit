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

#ifndef _FDVSERIAL_H_
#define _FDVSERIAL_H_

extern "C"
{
    #include "esp_common.h"    
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
	#include <stdarg.h>
}


#include "fdvsync.h"
#include "fdvprintf.h"
#include "fdvflash.h"



namespace fdv
{
	
 
	void DisableStdOut();

	
	
	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////
	// Serial

	class Serial
	{
		
		public:
			virtual void put(uint8_t value) = 0;
			virtual void write(uint8_t b) = 0;
			virtual int16_t peek() = 0;
			virtual int16_t read() = 0;
			virtual uint16_t available() = 0;
			virtual void flush() = 0;
			virtual bool waitForData(uint32_t timeOutMs = portMAX_DELAY) = 0;
					
			
			uint16_t ICACHE_FLASH_ATTR read(uint8_t* buffer, uint16_t bufferLen)
			{
				uint16_t ret = 0;
				for (int16_t c; bufferLen > 0 && (c = read()) > -1; --bufferLen, ++ret)
				{
					*buffer++ = c;
				}
				return ret;
			}			
						
			
			void ICACHE_FLASH_ATTR writeNewLine()
			{
				write(0x0D);
				write(0x0A);	
			}
			
			
			void ICACHE_FLASH_ATTR write(uint8_t const* buffer, uint16_t bufferLen)
			{
				for (;bufferLen > 0; --bufferLen)
					write(*buffer++);
			}


			void ICACHE_FLASH_ATTR write(char const* str)
			{
				while (*str)
					write(*str++);
			}
			
			void ICACHE_FLASH_ATTR writeln(char const* str)
			{
				write(str);
				writeNewLine();
			}

			// fmt can be in RAM or in Flash
			uint16_t ICACHE_FLASH_ATTR printf(char const *fmt, ...)
			{
				va_list args;
				
				char const* ramFmt = fmt;
				if (FStrUtils::isStoredInFlash(fmt))
					ramFmt = FStrUtils::strdup(fmt);

				va_start(args, fmt);
				uint16_t len = vsprintf(NULL, ramFmt, args);
				va_end(args);

				char buf[len + 1];
				
				va_start(args, fmt);
				vsprintf(buf, ramFmt, args);
				va_end(args);
				
				write(buf);

				if (ramFmt != fmt)
					free((void*)ramFmt);

				return len;
			}
			
			
			
	};

	// Serial
	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////
	
	
	

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// HardwareSerial
	
	// only UART0 is supported
	class HardwareSerial : public Serial
	{
		public:
		
			explicit HardwareSerial(uint32_t baud_rate, uint32_t rxBufferLength = 128)
				: m_queue(rxBufferLength)
			{		
				s_serials[0] = this;
				reconfig(baud_rate);
			}
			
			void reconfig(uint32_t baud_rate);
			
			using Serial::write;
			void write(uint8_t b);
		
			static HardwareSerial* getSerial(uint32_t uart)
			{
				return s_serials[uart];
			}
			
			// call only from ISR
			void put(uint8_t value)
			{
				m_queue.sendFromISR(value);
			}
			
			int16_t peek()
			{
				uint8_t ret;
				if (m_queue.peek(&ret, 0))
					return ret;
				return -1;
			}
			
			int16_t read()
			{
				uint8_t ret;
				if (m_queue.receive(&ret, 0))
					return ret;
				return -1;				
			}
			
			uint16_t available()
			{
				return m_queue.available();
			}
			
			void flush()
			{
				m_queue.clear();
			}
			
			bool waitForData(uint32_t timeOutMs = portMAX_DELAY)
			{
				uint8_t b;
				return m_queue.peek(&b, timeOutMs);
			}
			
		
		private:
		
			Queue<uint8_t>         m_queue;
			
			static HardwareSerial* s_serials[1];	// only one serial is supported
		
			
	};

	// HardwareSerial
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	
	
}



#endif