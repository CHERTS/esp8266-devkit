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
	#include <stdarg.h>
}




// fmt can be in RAM or in Flash
void FUNC_FLASHMEM debug(char const *fmt, ...)
{
	va_list args;
	
	char const* ramFmt = fmt;
	if (fdv::isStoredInFlash(fmt))
		ramFmt = fdv::f_strdup(fmt);

	va_start(args, fmt);
	uint16_t len = fdv::vsprintf(NULL, ramFmt, args);
	va_end(args);

	char buf[len + 1];
	
	va_start(args, fmt);
	fdv::vsprintf(buf, ramFmt, args);
	va_end(args);
	
	fdv::enterCritical();
	fdv::HardwareSerial::getSerial(0)->write(buf);
	fdv::exitCritical();

	if (ramFmt != fmt)
		delete[] ramFmt;
}

void FUNC_FLASHMEM debugstrn(char const* str, uint32_t len)
{
	fdv::enterCritical();
	while (len--)
		fdv::HardwareSerial::getSerial(0)->write(*str++);
	fdv::exitCritical();
}


