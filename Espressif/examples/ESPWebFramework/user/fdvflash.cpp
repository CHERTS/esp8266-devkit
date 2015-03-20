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
	
	
	uint16_t FUNC_FLASHMEM getWord(void const* buffer)
	{
		char const* pc = (char const*)buffer;
		return (uint8_t)getChar(pc) | ((uint8_t)getChar(pc + 1) << 8);
	}
	
	
	uint32_t FUNC_FLASHMEM getDWord(void const* buffer)
	{
		char const* pc = (char const*)buffer;
		return (uint8_t)getChar(pc) | ((uint8_t)getChar(pc + 1) << 8) | ((uint8_t)getChar(pc + 2) << 16) | ((uint8_t)getChar(pc + 3) << 24);
	}
	
	
}