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
#include "freertos/semphr.h"
}

#include "fdvsync.h"



extern "C" void ets_wdt_enable();
extern "C" void ets_wdt_disable();
extern "C" void ets_intr_lock();
extern "C" void ets_intr_unlock();



namespace fdv
{

	void ICACHE_FLASH_ATTR DisableWatchDog()
	{
		ets_wdt_disable();
	}
	
	
	void ICACHE_FLASH_ATTR EnableWatchDog()
	{
		ets_wdt_enable();
	}
	
	
	uint32_t ICACHE_FLASH_ATTR millisISR()
	{
		return xTaskGetTickCountFromISR() * portTICK_RATE_MS;
	}
	
	
	uint32_t ICACHE_FLASH_ATTR millis()
	{
		return xTaskGetTickCount() * portTICK_RATE_MS; 
	}
	
	
	// calculates time difference in milliseconds, taking into consideration the time overflow
	// note: time1 must be less than time2 (time1 < time2)
	uint32_t ICACHE_FLASH_ATTR millisDiff(uint32_t time1, uint32_t time2)
	{
		if (time1 > time2)
			// overflow
			return 0xFFFFFFFF - time1 + time2;
		else
			return time2 - time1;
	}
	
	
		
	
}