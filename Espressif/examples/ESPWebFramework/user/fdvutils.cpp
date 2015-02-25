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

extern "C"
{
    #include "esp_common.h"    
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

	//#include <stdio.h>
	//#include <stdarg.h>
	//#include <string.h>
	//#include <stdlib.h>
}





void *__dso_handle;




////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

void * ICACHE_FLASH_ATTR operator new(size_t size)
{
  return malloc(size);
}

void * ICACHE_FLASH_ATTR operator new[](size_t size) 
{
  return malloc(size);
}

void ICACHE_FLASH_ATTR operator delete(void * ptr) 
{
  free(ptr);
}

void ICACHE_FLASH_ATTR operator delete[](void * ptr) 
{
  free(ptr);
}

extern "C" void __cxa_pure_virtual(void) __attribute__ ((__noreturn__));

extern "C" void __cxa_deleted_virtual(void) __attribute__ ((__noreturn__));

extern "C" void ICACHE_FLASH_ATTR abort() 
{
  while(true); // enter an infinite loop and get reset by the WDT
}

void ICACHE_FLASH_ATTR __cxa_pure_virtual(void) 
{
  abort();
}

void ICACHE_FLASH_ATTR __cxa_deleted_virtual(void) 
{
  abort();
}





////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
