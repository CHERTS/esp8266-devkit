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

#ifndef _FDVSYNC_H
#define _FDVSYNC_H



#include "fdv.h"

extern "C"
{
#include "freertos/semphr.h"
#include "freertos/queue.h"
}



namespace fdv
{
	
	
	uint32_t millisISR();		
	uint32_t millis();
	uint32_t millisDiff(uint32_t time1, uint32_t time2);
	void DisableWatchDog();
	void EnableWatchDog();


	inline void enterCritical()
	{
		taskENTER_CRITICAL();
	}
	
	inline void exitCritical()
	{
		taskEXIT_CRITICAL();
	}

	/////////////////////////////////////////////////////////////////////+
	/////////////////////////////////////////////////////////////////////+
	// Mutex
	// An FreeRTOS semaphore wrapper	
	//
	// Example:
	//
	// fdv::Mutex mutex;
	//
	// {
	//   fdv::MutexLock lock(mutex);
	// } <- mutex unlocked
	
	class Mutex
	{
		public:		
			Mutex()
				: m_handle(NULL)
			{
				vSemaphoreCreateBinary(m_handle);
			}
			
			virtual ~Mutex()
			{
				vSemaphoreDelete(m_handle);
			}
			
			bool lock(uint32_t msTimeOut = portMAX_DELAY)
			{
				return xSemaphoreTake(m_handle, msTimeOut / portTICK_RATE_MS);
			}
			
			bool lockFromISR()
			{
				signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
				return xSemaphoreTakeFromISR(m_handle, &xHigherPriorityTaskWoken);
			}
			
			void unlock()
			{
				xSemaphoreGive(m_handle);
			}
			
			void unlockFromISR()
			{
				signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
				xSemaphoreGiveFromISR(m_handle, &xHigherPriorityTaskWoken);				
			}
					
		private:		
			xSemaphoreHandle m_handle;
	};

	
	/////////////////////////////////////////////////////////////////////+
	/////////////////////////////////////////////////////////////////////+
	// MutexLock & MutexLockFromISR
	// Mutex automatic lock/unlock helper

	class MutexLock
	{
		public:
			MutexLock(Mutex* mutex, uint32_t msTimeOut = portMAX_DELAY)
			  : m_mutex(mutex)
			{
				m_acquired = m_mutex->lock(msTimeOut);
			}

			virtual ~MutexLock()
			{
				if (m_acquired)
					m_mutex->unlock();
			}
			
			operator bool()
			{
				return m_acquired;
			}

		private:
			Mutex* m_mutex;
			bool   m_acquired;
	};
	
	
	class MutexLockFromISR
	{
		public:
			MutexLockFromISR(Mutex* mutex)
			  : m_mutex(mutex)
			{
				m_acquired = m_mutex->lockFromISR();
			}

			virtual ~MutexLockFromISR()
			{
				if (m_acquired)
					m_mutex->unlockFromISR();
			}
			
			operator bool()
			{
				return m_acquired;
			}

		private:
			Mutex* m_mutex;
			bool   m_acquired;
	};

	
	/////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////
	// SafeValue
	// A task safe value
	// Don't use inside ISR
	
	template <typename T>
	struct SafeValue
	{
		SafeValue(T const& value)
			: m_value(value)
		{
		}
		
		operator const T() const
		{
			MutexLock lock(&m_mutex);
			return m_value;
		}
		
		void operator =(T const& value)
		{
			MutexLock lock(&m_mutex);
			m_value = value;
		}
		
		const T operator++()
		{
			MutexLock lock(&m_mutex);
			return ++m_value;
		}
		
		const T operator++(int)
		{
			MutexLock lock(&m_mutex);
			return m_value++;
		}
			
		const T operator--()
		{
			MutexLock lock(&m_mutex);
			return --m_value;
		}
		
		const T operator--(int)
		{
			MutexLock lock(&m_mutex);
			return m_value--;
		}
		
	private:
		T     m_value;
		Mutex m_mutex;
	};
	
	
	/////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////
	// ResourceCounter
	// Note: cannot be implemented using RTOS CountingSemaphpore because not linked into ESP8266 sdk. Implemented using two semaphores.
	// Count starts from resourceCount. get() decreases available resources.
	// Do not use inside ISR
	
	struct ResourceCounter
	{
		ResourceCounter(uint32_t resourcesCount)
			: m_resources(resourcesCount)
		{
		}
		
		// decrements counter. Wait if no resource are available.
		bool get(uint32_t msTimeOut = portMAX_DELAY)
		{
			if (m_gate.lock(msTimeOut))
			{
				m_mutex.lock();
				--m_resources;
				if (m_resources > 0)
					m_gate.unlock();
				m_mutex.unlock();
				return true;
			}
			return false;
		}
		
		// increments counter
		void release()
		{
			m_mutex.lock();
			++m_resources;
			if (m_resources == 1)
				m_gate.unlock();
			m_mutex.unlock();
		}
		
	private:
		Mutex    m_mutex;
		Mutex    m_gate;
		uint32_t m_resources;
	};


	
	/////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////
	// SoftTimeOut class
	// ex. SoftTimeOut(200)  <- after 200ms SoftTimeOut() returns true
	// note: not use inside ISR!!
	//
	// Example:
	// SoftTimeOut timeOut(200);
	// while (!timeOut)
	//   dosomething();

	class SoftTimeOut
	{
		public:
			SoftTimeOut(uint32_t time)
				: m_timeOut(time), m_startTime(millis())
			{
			}

			operator bool()
			{
				return millisDiff(m_startTime, millis()) > m_timeOut;
			}
			
			void reset(uint32_t time)
			{
				m_timeOut   = time;
				m_startTime = millis();				
			}

		private:
			uint32_t m_timeOut;
			uint32_t m_startTime;
	};
	

	/////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////
	// Queue class
	// A wrapper over FreeRTOS queue
	//
	// Example:
	//
	// Queue<uint8_t> byteQueue;
	//
	// byteQueue.send(100);
	// ...
	// byteQueue.receive(&data);
	
	template <typename T>
	class Queue
	{
		
		public:
		
			Queue(uint32_t queueLength)
			{
				m_handle = xQueueCreate(queueLength, sizeof(T));
			}
						
			virtual ~Queue()
			{
				vQueueDelete(m_handle);
			}
			
			bool send(T& item, uint32_t msTimeOut = portMAX_DELAY)
			{
				return xQueueSend(m_handle, &item, msTimeOut / portTICK_RATE_MS);
			}
			
			bool send(uint32_t msTimeOut = portMAX_DELAY)
			{
				T dummyItem;
				return send(dummyItem, msTimeOut);
			}

			bool sendFromISR(T& item)
			{
				signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
				return xQueueSendFromISR(m_handle, &item, &xHigherPriorityTaskWoken);
			}
			
			bool receive(T* item, uint32_t msTimeOut = portMAX_DELAY)
			{
				return xQueueReceive(m_handle, item, msTimeOut / portTICK_RATE_MS);
			}
			
			bool receive(uint32_t msTimeOut = portMAX_DELAY)
			{
				T dummyItem;
				return receive(&dummyItem, msTimeOut);
			}
			
			bool peek(T* item, uint32_t msTimeOut = portMAX_DELAY)
			{
				return xQueuePeek(m_handle, item, msTimeOut / portTICK_RATE_MS);
			}			
			
			void clear()
			{
				xQueueReset(m_handle);
			}
			
			uint32_t available()
			{
				return uxQueueMessagesWaiting(m_handle);
			}
						
		private:
			
			xQueueHandle m_handle;
		
	};

	
	/////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////
	// Critical
	// Embends FreeRTOS taskENTER_CRITICAL and taskEXIT_CRITICAL
	
	struct Critical
	{
		Critical()
		{
			enterCritical();
		}
		
		~Critical()
		{
			exitCritical();
		}
	};
	
}





#endif