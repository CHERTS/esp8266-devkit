
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "gpio.h"
#include "io.h"
#include <stdlib.h>
#include "config.h"
#include "sntp.h"
#include "time_utils.h"
#include "dht22.h"
#include "ds18b20.h"

static int ICACHE_FLASH_ATTR wd(int year, int month, int day) {
  size_t JND =                                                     \
          day                                                      \
        + ((153 * (month + 12 * ((14 - month) / 12) - 3) + 2) / 5) \
        + (365 * (year + 4800 - ((14 - month) / 12)))              \
        + ((year + 4800 - ((14 - month) / 12)) / 4)                \
        - ((year + 4800 - ((14 - month) / 12)) / 100)              \
        + ((year + 4800 - ((14 - month) / 12)) / 400)              \
        - 32045;
  return (int)JND % 7;
}


void ICACHE_FLASH_ATTR thermostat(int current_t, int setpoint)
{

	//hardcoded .5 degrees hysteresis
	if(current_t < setpoint - 50 ) {
		os_printf("Current reading (%d) is less than the setpoint.\n",current_t);

		if(sysCfg.thermostat1opmode==THERMOSTAT_HEATING)
			currGPIO12State=1;
		else
			currGPIO12State=0;
			
		ioGPIO(currGPIO12State,12);
	} else if(current_t > setpoint + 50 ) {
		os_printf("Current reading (%d) is more than the setpoint.\n",current_t);

		if(sysCfg.thermostat1opmode==THERMOSTAT_HEATING)
			currGPIO12State=0;
		else
			currGPIO12State=1;

		ioGPIO(currGPIO12State,12);
	}
}

static  void ICACHE_FLASH_ATTR pollThermostatCb(void * arg)
{
		unsigned long epoch = sntp_time+(sntp_tz*3600);
		int year=get_year(&epoch);
		int month=get_month(&epoch,year);
		int day=day=1+(epoch/86400);
		int dow=wd(year,month,day);
		epoch=epoch%86400;
		unsigned int hour=epoch/3600;
		epoch%=3600;
		unsigned int min=epoch/60;
		int minadj = (min*100/60);
		int currtime = hour*100+minadj;
				
		if(sysCfg.thermostat1state == 0) {
			os_printf("Thermostat switched off, abandoning routine.\n");
			return;
		}
		
		long Treading=-9999;
		
		if(sysCfg.sensor_dht22_enable) { 
			struct sensor_reading* result = readDHT();
			if(result->success) {
				Treading=result->temperature*100;
				if(sysCfg.sensor_dht22_humi_thermostat)
					Treading=result->humidity*100;
			}			
		}
		else {
			struct sensor_reading* result = read_ds18b20();
			if(result->success) {
	
			    int SignBit, Whole, Fract;
	
				Treading = result->temperature;
				
				SignBit = Treading & 0x8000;  // test most sig bit
				if (SignBit) // negative
				Treading = (Treading ^ 0xffff) + 1; // 2's comp
	
				Whole = Treading >> 4;  // separate off the whole and fractional portions
				Fract = (Treading & 0xf) * 100 / 16;

				if (SignBit) // negative
					Whole*=-1;

				Treading=Whole*100+Fract;

			}
		}

		if(Treading ==-9999 || Treading >12000 || Treading <-30000) { // Check for valid reading
			os_printf("Could not get valid temperature reading\n");
			return;
		} 

		if(sysCfg.thermostat1mode == THERMOSTAT_MANUAL) {
			thermostat(Treading, (int)sysCfg.thermostat1manualsetpoint);
			return;
		}



		if(year<2015) { // Something is wrong with the NTP time, maybe not enabled?
			os_printf("NTP time seems incorrect!\n");
			return;
		} 
		
		for(int sched=0; sched<8 && sysCfg.thermostat1schedule.weekSched[dow].daySched[sched].active==1; sched++) {
				if(currtime >= sysCfg.thermostat1schedule.weekSched[dow].daySched[sched].start && currtime < sysCfg.thermostat1schedule.weekSched[dow].daySched[sched].end) {
					os_printf("Current schedule (%d) setpoint is: %d\n",sched,sysCfg.thermostat1schedule.weekSched[dow].daySched[sched].setpoint);
					thermostat(Treading, sysCfg.thermostat1schedule.weekSched[dow].daySched[sched].setpoint);
				}
		}
}

void ICACHE_FLASH_ATTR thermostat_init(uint32_t polltime)
{
	
	os_printf("Thermostat daemon init; poll interval of %d sec\n", (int)polltime/1000);
  
	static ETSTimer thermostatTimer;
	os_timer_setfn(&thermostatTimer, pollThermostatCb, NULL);
	os_timer_arm(&thermostatTimer, polltime, 1);
  
}
