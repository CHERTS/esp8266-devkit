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

#ifndef _FDVTASK_H_
#define _FDVTASK_H_

extern "C"
{
    #include "esp_common.h"    
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
	#include "stdarg.h"
}



namespace fdv
{
	
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	// Task
	// An abstract wrapper for FreeRTOS task
	//
	// Example:
	//
	// struct MainTask : fdv::Task
	// {
	//   void ICACHE_FLASH_ATTR exec()
	//   {
	//     fdv::DisableStdOut();
	//     fdv::DisableWatchDog();		
	//     fdv::HardwareSerial serial(115200, 128);		
	//     Task1 anotherTask;  // creates another task		
	//     suspend();
	//   }
	// };

	
	class Task
	{
	public:
		
		Task(uint16_t stackDepth = 256, uint32_t priority = 2, bool suspended = false)
			: m_stackDepth(stackDepth), m_priority(priority), m_handle(NULL)
		{
			if (!suspended)
				resume();			
		}
		
		virtual ~Task()
		{
			vTaskDelete(m_handle);
		}
		
		// call only when "suspended" in constructor is true and before resume()
		void ICACHE_FLASH_ATTR setStackDepth(uint16_t stackDepth)
		{
			m_stackDepth = stackDepth;
		}
		
		void ICACHE_FLASH_ATTR suspend()
		{
			vTaskSuspend(m_handle);
		}
		
		void ICACHE_FLASH_ATTR resume()
		{
			if (m_handle)
				vTaskResume(m_handle);
			else
				xTaskCreate(entry, (const signed char*)"", m_stackDepth, this, m_priority, &m_handle);			
		}
		
		void ICACHE_FLASH_ATTR delay(uint32_t ms)
		{
			vTaskDelay(ms / portTICK_RATE_MS);
		}
		
		// task free stack (in bytes)
		static uint32_t ICACHE_FLASH_ATTR getFreeStack()
		{
			return uxTaskGetStackHighWaterMark(NULL) * 4;
		}
		
		// global free heap (in bytes)
		static uint32_t ICACHE_FLASH_ATTR getFreeHeap()
		{
			return system_get_free_heap_size();
		}
		
	public:
	
		virtual void exec() = 0;
		

	private:
	
		static void ICACHE_FLASH_ATTR entry(void* params)
		{
			static_cast<Task*>(params)->exec();
		}
		
	private:
		uint16_t    m_stackDepth;
		uint32_t    m_priority;
		xTaskHandle m_handle;
		
	};


	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	// MethodTask
	// A way to execute different tasks in different methods of the same class
	//
	// Example:
	// struct MyClass
	// {
	//   MyClass()
	//   {		
	//      m_task1 = new MethodTask<MyClass, &MyClass::task1>(*this);
	//      m_task2 = new MethodTask<MyClass, &MyClass::task2>(*this);
	//   }
	//   void task1()
	//   {
	//      ...
	//   }
	//   void task2()
	//   {
	//      ...
	//   }
	//   Task* m_task1;
	//   Task* m_task2;
	// };
	
	
	template <typename T, void (T::*Method)()>
	class MethodTask : public Task
	{
	public:
		MethodTask(T& object, uint16_t stackDepth = 256, uint32_t priority = 2)
			: Task(stackDepth, priority), m_object(object)
		{
		}
		
		void exec()
		{
			(m_object.*Method)();
		}
	private:
		T& m_object;
	};
	
	
}



#endif