#include "ets_sys.h"
#include "osapi.h"
#include "espmissingincludes.h"

#include "wifi.h"
#include "user_interface.h"
#include "espconn.h"

#include "sntp.h"
#include "time_utils.h"
#include "driver/i2c.h"
#include "driver/i2c_oled.h"
#include "dht22.h"
#include "ds18b20.h"
#include "config.h"

static volatile bool OLED;

static void ICACHE_FLASH_ATTR pollOLEDCb(void * arg){

	static bool shown = false;
	if(!shown) {
		static uint8_t wifiStatus = STATION_IDLE;
		struct ip_info ipConfig;
		wifi_get_ip_info(STATION_IF, &ipConfig);
		wifiStatus = wifi_station_get_connect_status();
		if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0) {	
			shown=true;
			unsigned char displaystr[32];
			struct ip_info pTempIp;
			wifi_get_ip_info(0x00, &pTempIp);
			os_sprintf((char *)displaystr, "IP: %d.%d.%d.%d  ",IP2STR(&pTempIp.ip));
			OLED_Print(2, 2, displaystr, 1);
		}
	} else {
	
		OLED_CLS();
		char displaystr[32];
		char temp[16];
		
		unsigned long epoch = sntp_time+(sntp_tz*3600);
		epoch=epoch%86400;
		unsigned int hour=epoch/3600;
		
		if(hour >= 6 && hour <=23) { //Protect the OLED from burning
 
			if(sysCfg.sensor_dht22_enable) {
				dht_temp_str(temp);
				os_sprintf(displaystr, "Temp: %sC    ", temp);
				OLED_Print(2, 0, (unsigned char *)displaystr, 2);
  
				dht_humi_str(temp);
				os_sprintf(displaystr, "Humi: %s%%    ", temp);
				OLED_Print(2, 2, (unsigned char *)displaystr, 2);
			}
		
			if(sysCfg.sensor_ds18b20_enable) {
				ds_str(temp,0);
				os_sprintf(displaystr, "Temp: %sC    ", temp);
				OLED_Print(2 + rand()%2, rand()%4, (unsigned char *)displaystr, 2);		
			}

		}
			
		//os_sprintf(displaystr,"%s",epoch_to_str(sntp_time+(sntp_tz*3600)));  
		//OLED_Print(0, 6, (unsigned char *)displaystr, 1);
	}
}

void ICACHE_FLASH_ATTR OLEDInit(void) {

  i2c_init();
  OLED = OLED_Init();
  OLED_Print(2, 0, (unsigned char *)"ESP8266 3Ch Relay", 1);
  OLED_Print(2, 1, (unsigned char *)"harizanov.com", 1);
    
  static ETSTimer oledTimer;
  os_timer_setfn(&oledTimer, pollOLEDCb, NULL);
  os_timer_arm(&oledTimer, 20000, 1);
}
