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

#ifndef _FDVFLASH_H_
#define _FDVFLASH_H_



#include "fdv.h"



// Flash from 0x0 to 0x8000      mapped to 0x40100000, len = 0x8000 (32KBytes)    - ".text"
// Flash from 0x40000 to 0x7C000 mapped to 0x40240000, len = 0x3C000 (240KBytes)  - ".irom0.text"
// Other content (settings, web files) may be written starting from 0x14000, with maximum (0x2C000) 180KBytes (usable in blocks of 4K)

static uint32_t const FLASH_MAP_START = 0x40200000;




namespace fdv
{

	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// isStoredInFlash
	static inline bool FUNC_FLASHMEM isStoredInFlash(void const* ptr)
	{
		return (uint32_t)ptr >= 0x40200000 && (uint32_t)ptr < 0x40300000;
	}
	

	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// getChar
	// flash mapped "text" is readable as 4 byte aligned blocks
	// works with both RAM and Flash stored data
	// str must be 32 bit aligned
	static inline char FUNC_FLASHMEM getChar(char const* str, uint32_t index)
	{
		if (isStoredInFlash(str))
		{
			uint32_t u32 = ((uint32_t const*)str)[index >> 2];
			return ((char const*)&u32)[index & 0x3];
		}
		else
			return str[index];
	}


	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// getChar
	// flash mapped "text" is readable as 4 byte aligned blocks
	// works with both RAM and Flash stored data
	// Returns first char of str
	// str may Not be 32 bit aligned
	static inline char FUNC_FLASHMEM getChar(char const* str)
	{
		if (isStoredInFlash(str))
		{
			uint32_t index = (uint32_t)str & 0x3;
			str = (char const*)((uint32_t)str & 0xFFFFFFFC);  // align str
			uint32_t u32 = *((uint32_t const*)str);
			return ((char const*)&u32)[index];
		}
		else
			return *str;
	}		


	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// getByte
	// like getChar but for uint8_t
	// buffer can be unaligned
	static inline uint8_t FUNC_FLASHMEM getByte(void const* buffer)
	{
		return (uint8_t)getChar((char const*)buffer);
	}


	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// getWord
	// like getByte but for uint16_t
	// buffer can be unaligned
	// read as little-endian
	static inline uint16_t FUNC_FLASHMEM getWord(void const* buffer)
	{
		char const* pc = (char const*)buffer;
		return (uint8_t)getChar(pc) | ((uint8_t)getChar(pc + 1) << 8);
	}


	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// getDWord
	// like getWord but for uint32_t
	// buffer can be unaligned
	// read as little-endian
	static inline uint32_t FUNC_FLASHMEM getDWord(void const* buffer)
	{
		char const* pc = (char const*)buffer;
		return (uint8_t)getChar(pc) | ((uint8_t)getChar(pc + 1) << 8) | ((uint8_t)getChar(pc + 2) << 16) | ((uint8_t)getChar(pc + 3) << 24);
	}
	
	
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// CharIterator
	// Can iterate both RAM and Flash strings

	struct CharIterator
	{
		CharIterator(char const* str = NULL)
			: m_str(str)
		{
		}
		
		char const* MTD_FLASHMEM get()
		{
			return m_str;
		}
		
		char MTD_FLASHMEM operator*()
		{
			return getChar(m_str);
		}
		
		CharIterator MTD_FLASHMEM operator++(int)
		{
			CharIterator p = *this;
			++m_str;
			return p;
		}
		
		CharIterator MTD_FLASHMEM operator++()
		{
			++m_str;
			return *this;
		}
		
		CharIterator MTD_FLASHMEM operator+(int32_t rhs)
		{
			return m_str + rhs;
		}
		
		int32_t MTD_FLASHMEM operator-(CharIterator const& rhs)
		{
			return m_str - rhs.m_str;
		}
		
		bool MTD_FLASHMEM operator==(char const* rhs)
		{
			return getChar(m_str) == *rhs;
		}
		
		bool MTD_FLASHMEM operator==(CharIterator const& rhs)
		{
			return m_str == rhs.m_str;
		}
		
		bool MTD_FLASHMEM operator!=(char const* rhs)
		{
			return getChar(m_str) != *rhs;
		}
		
		bool MTD_FLASHMEM operator!=(CharIterator const& rhs)
		{
			return m_str != rhs.m_str;
		}
		
	private:
		char const* m_str;
	};

	
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// ByteIterator
	// Can iterate both RAM and Flash buffers

	struct ByteIterator
	{
		ByteIterator(uint8_t const* buf = NULL)
			: m_buf(buf)
		{
		}
		
		uint8_t const* MTD_FLASHMEM get()
		{
			return m_buf;
		}
		
		uint8_t MTD_FLASHMEM operator*()
		{
			return getByte(m_buf);
		}
		
		ByteIterator MTD_FLASHMEM operator++(int)
		{
			ByteIterator p = *this;
			++m_buf;
			return p;
		}
		
		ByteIterator MTD_FLASHMEM operator++()
		{
			++m_buf;
			return *this;
		}
		
		bool MTD_FLASHMEM operator==(uint8_t const* rhs)
		{
			return getByte(m_buf) == *rhs;
		}
		
		bool MTD_FLASHMEM operator!=(uint8_t const* rhs)
		{
			return getByte(m_buf) != *rhs;
		}
		
	private:
		uint8_t const* m_buf;
	};
	
	
	
	
}


#endif