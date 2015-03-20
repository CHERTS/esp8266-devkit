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
	#include "freertos/semphr.h"
	#include "lwip/mem.h"
}




void *__dso_handle;


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Memory

namespace fdv
{

	void* STC_FLASHMEM Memory::malloc(uint32_t size)
	{
		return mem_malloc(size);
	}

	void STC_FLASHMEM Memory::free(void* ptr)
	{
		mem_free(ptr);
	}

}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

void * FUNC_FLASHMEM operator new(size_t size)
{
	return mem_malloc(size);
}

void * FUNC_FLASHMEM operator new[](size_t size) 
{
	return mem_malloc(size);
}

void FUNC_FLASHMEM operator delete(void * ptr) 
{
	mem_free(ptr);
}

void FUNC_FLASHMEM operator delete[](void * ptr) 
{
	mem_free(ptr);
}

extern "C" void __cxa_pure_virtual(void) __attribute__ ((__noreturn__));

extern "C" void __cxa_deleted_virtual(void) __attribute__ ((__noreturn__));

extern "C" void FUNC_FLASHMEM abort() 
{
  while(true); // enter an infinite loop and get reset by the WDT
}

void FUNC_FLASHMEM __cxa_pure_virtual(void) 
{
  abort();
}

void FUNC_FLASHMEM __cxa_deleted_virtual(void) 
{
  abort();
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// reboot
// creates a task and reboot after specified time (ms)

namespace fdv
{

	struct RebootTask : Task
	{
		RebootTask(uint32_t time) 
			: Task(false, 400), m_time(time)
		{
		}
		
		void MTD_FLASHMEM exec()
		{
			delay(m_time);
			EnableWatchDog();
			taskENTER_CRITICAL();
			taskDISABLE_INTERRUPTS();
			while(1);	// reset using watchdog
		}
		
		uint32_t m_time;
	};


	void FUNC_FLASHMEM reboot(uint32_t time)
	{	
		new RebootTask(time);
	}

}



