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


namespace fdv
{
	

	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	
	// size in Bytes
	uint32_t FUNC_FLASHMEM getFlashSize()
	{
		uint32_t flags = *((uint32_t volatile*)0x40200000);
		switch(flags >> 24 & 0xF0)
		{
			case 0:
				return 0x080000;	// 512K
			case 1:
				return 0x040000;	// 256K
			case 2:
				return 0x100000;	// 1MB
			case 3:
				return 0x200000;	// 2MB
			case 4:
				return 0x400000;	// 4MB				
		}
		return 0;	// unknown
	}

	// size in MHz
	uint32_t FUNC_FLASHMEM getFlashSpeed()
	{
		uint32_t flags = *((uint32_t volatile*)0x40200000);
		switch(flags >> 24 & 0x0F)
		{
			case 0:
				return 40;
			case 1:
				return 26;
			case 2:
				return 20;
			case 15:
				return 80;
		}
		return 0;	// unknown
	}
	

	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////

	char FUNC_FLASHMEM getChar(char const* str, uint32_t index)
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

	char FUNC_FLASHMEM getChar(char const* str)
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

	uint16_t FUNC_FLASHMEM getWord(void const* buffer)
	{
		char const* pc = (char const*)buffer;
		return (uint8_t)getChar(pc) | ((uint8_t)getChar(pc + 1) << 8);
	}
	
	
	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////

	uint32_t FUNC_FLASHMEM getDWord(void const* buffer)
	{
		char const* pc = (char const*)buffer;
		return (uint8_t)getChar(pc) | ((uint8_t)getChar(pc + 1) << 8) | ((uint8_t)getChar(pc + 2) << 16) | ((uint8_t)getChar(pc + 3) << 24);
	}



	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// CharIterator

    char const* MTD_FLASHMEM CharIterator::get()
    {
        return m_str;
    }

    char MTD_FLASHMEM CharIterator::operator*()
    {
        return getChar(m_str);
    }

    CharIterator MTD_FLASHMEM CharIterator::operator++(int)
    {
        CharIterator p = *this;
        ++m_str;
        return p;
    }

    CharIterator MTD_FLASHMEM CharIterator::operator++()
    {
        ++m_str;
        return *this;
    }

    CharIterator MTD_FLASHMEM CharIterator::operator+(int32_t rhs)
    {
        return m_str + rhs;
    }

    int32_t MTD_FLASHMEM CharIterator::operator-(CharIterator const& rhs)
    {
        return m_str - rhs.m_str;
    }

    bool MTD_FLASHMEM CharIterator::operator==(char const* rhs)
    {
        return getChar(m_str) == *rhs;
    }

    bool MTD_FLASHMEM CharIterator::operator==(CharIterator const& rhs)
    {
        return m_str == rhs.m_str;
    }

    bool MTD_FLASHMEM CharIterator::operator!=(char const* rhs)
    {
        return getChar(m_str) != *rhs;
    }

    bool MTD_FLASHMEM CharIterator::operator!=(CharIterator const& rhs)
    {
        return m_str != rhs.m_str;
    }
		
	
}