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


	/////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

	char* FUNC_FLASHMEM f_printf(char const *fmt, ...)
	{
		va_list args;
		
		va_start(args, fmt);
		uint16_t len = vsprintf(NULL, fmt, args);
		va_end(args);

		char* buf = new char[len + 1];
		
		va_start(args, fmt);
		vsprintf(buf, fmt, args);
		va_end(args);

		return buf;
	}


	/////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////
	// inplaceURLDecode
	// str can stay only in RAM
	// returns str
	
	char* FUNC_FLASHMEM inplaceURLDecode(char* str)
	{
		char* rpos = str;
		char* wpos = str;
		while (*rpos)
		{
			if (*rpos == '%')
			{
				if (isxdigit(rpos[1]) && rpos[2] && isxdigit(rpos[2]))
				{
					*wpos++ = (hexDigitToInt(rpos[1]) << 4) | hexDigitToInt(rpos[2]);
					rpos += 3;
				}
			}
			else if (*rpos == '+')
			{
				*wpos++ = 0x20;
				++rpos;
			}
			else
			{
				*wpos++ = *rpos++;
			}
		}
		*wpos = 0;
		return str;
	}

	
	


	

}



