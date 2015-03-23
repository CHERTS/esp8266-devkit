/*
 config.c
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"

#include "mqtt.h"
#include "config.h"
#include "user_config.h"
#include "debug.h"

SYSCFG sysCfg;
SAVE_FLAG saveFlag;

void ICACHE_FLASH_ATTR CFG_Save()
{
	 spi_flash_read((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
	                   (uint32 *)&saveFlag, sizeof(SAVE_FLAG));

	if (saveFlag.flag == 0) {
		spi_flash_erase_sector(CFG_LOCATION + 1);
		spi_flash_write((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&sysCfg, sizeof(SYSCFG));
		saveFlag.flag = 1;
		spi_flash_erase_sector(CFG_LOCATION + 3);
		spi_flash_write((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	} else {
		spi_flash_erase_sector(CFG_LOCATION + 0);
		spi_flash_write((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&sysCfg, sizeof(SYSCFG));
		saveFlag.flag = 0;
		spi_flash_erase_sector(CFG_LOCATION + 3);
		spi_flash_write((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	}
}

void ICACHE_FLASH_ATTR CFG_Load()
{

	os_printf("\r\nload cfg...(%d bytes)\r\n", sizeof(SYSCFG) );
	spi_flash_read((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
				   (uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	if (saveFlag.flag == 0) {
		spi_flash_read((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
					   (uint32 *)&sysCfg, sizeof(SYSCFG));
	} else {
		spi_flash_read((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
					   (uint32 *)&sysCfg, sizeof(SYSCFG));
	}
	if(sysCfg.cfg_holder != CFG_HOLDER){
		os_memset(&sysCfg, 0x00, sizeof sysCfg);

		sysCfg.cfg_holder = CFG_HOLDER;
	
		os_sprintf((char *)sysCfg.sta_mode, "%s", STA_MODE);
		os_sprintf((char *)sysCfg.sta_ip, "%s", STA_IP);
		os_sprintf((char *)sysCfg.sta_mask, "%s", STA_MASK);
		os_sprintf((char *)sysCfg.sta_gw, "%s", STA_GW);
		os_sprintf((char *)sysCfg.sta_ssid, "%s", STA_SSID);
		os_sprintf((char *)sysCfg.sta_pass, "%s", STA_PASS);
		sysCfg.sta_type=STA_TYPE;

		os_sprintf((char *)sysCfg.ap_ip, "%s", AP_IP);
		os_sprintf((char *)sysCfg.ap_mask, "%s", AP_MASK);
		os_sprintf((char *)sysCfg.ap_gw, "%s", AP_GW);

		sysCfg.httpd_port=HTTPD_PORT;
		sysCfg.httpd_auth=HTTPD_AUTH;
		os_sprintf((char *)sysCfg.httpd_user, "%s", HTTPD_USER);
		os_sprintf((char *)sysCfg.httpd_pass, "%s", HTTPD_PASS);

		sysCfg.ntp_enable=NTP_ENABLE;
		sysCfg.ntp_tz=NTP_TZ;

		sysCfg.mqtt_enable=MQTT_ENABLE;
		os_sprintf((char *)sysCfg.mqtt_host, "%s", MQTT_HOST);
		sysCfg.mqtt_port=MQTT_PORT;
		sysCfg.mqtt_keepalive=MQTT_KEEPALIVE;
		os_sprintf((char *)sysCfg.mqtt_devid, MQTT_DEVID, system_get_chip_id());
		os_sprintf((char *)sysCfg.mqtt_user, "%s", MQTT_USER);
		os_sprintf((char *)sysCfg.mqtt_pass, "%s", MQTT_PASS);
		sysCfg.mqtt_use_ssl=MQTT_USE_SSL;
		os_sprintf((char *)sysCfg.mqtt_relay_subs_topic, MQTT_RELAY_SUBS_TOPIC, system_get_chip_id());
		os_sprintf((char *)sysCfg.mqtt_dht22_temp_pub_topic, MQTT_DHT22_TEMP_PUB_TOPIC, system_get_chip_id()); 
		os_sprintf((char *)sysCfg.mqtt_dht22_humi_pub_topic, MQTT_DHT22_HUMI_PUB_TOPIC, system_get_chip_id());		
		os_sprintf((char *)sysCfg.mqtt_ds18b20_temp_pub_topic, MQTT_DS18B20_TEMP_PUB_TOPIC, system_get_chip_id());

		sysCfg.sensor_ds18b20_enable=SENSOR_DS18B20_ENABLE;
		sysCfg.sensor_dht22_enable=SENSOR_DHT22_ENABLE;
		sysCfg.sensor_dht22_humi_thermostat=0;
				
		sysCfg.relay_latching_enable=RELAY_LATCHING_ENABLE;
		sysCfg.relay_1_state=0;
		sysCfg.relay_2_state=0;
		sysCfg.relay_3_state=0;
		
		os_sprintf((char *)sysCfg.relay1name, "%s", RELAY1NAME);
		os_sprintf((char *)sysCfg.relay2name, "%s", RELAY2NAME);
		os_sprintf((char *)sysCfg.relay3name, "%s", RELAY3NAME);
		
		sysCfg.broadcastd_enable=BROADCASTD_ENABLE;
		sysCfg.broadcastd_port=BROADCASTD_PORT;
		os_sprintf((char *)sysCfg.broadcastd_host, "%s", BROADCASTD_HOST);
		os_sprintf((char *)sysCfg.broadcastd_url, "%s", BROADCASTD_URL);
		sysCfg.broadcastd_thingspeak_channel=BROADCASTD_THINGSPEAK_CHANNEL;
		os_sprintf((char *)sysCfg.broadcastd_ro_apikey, "%s", BROADCASTD_RO_APIKEY);
		
		sysCfg.thermostat1state=0;
		sysCfg.thermostat1manualsetpoint=2100;
		sysCfg.thermostat1mode=THERMOSTAT_MANUAL;
		sysCfg.thermostat1opmode=THERMOSTAT_HEATING;

		//Build default schedule for the thermostat
		for(int dow=0; dow<7; dow++) {
			sysCfg.thermostat1schedule.weekSched[dow].daySched[0].start=0; //0am
			sysCfg.thermostat1schedule.weekSched[dow].daySched[0].end=600; //6am, hours are * 100
			sysCfg.thermostat1schedule.weekSched[dow].daySched[0].setpoint=1000; //10.0*C
			sysCfg.thermostat1schedule.weekSched[dow].daySched[0].active=1;
			
			sysCfg.thermostat1schedule.weekSched[dow].daySched[1].start=600;
			sysCfg.thermostat1schedule.weekSched[dow].daySched[1].end=900;
			sysCfg.thermostat1schedule.weekSched[dow].daySched[1].setpoint=1800;
			sysCfg.thermostat1schedule.weekSched[dow].daySched[1].active=1;
			
			sysCfg.thermostat1schedule.weekSched[dow].daySched[2].start=900;
			sysCfg.thermostat1schedule.weekSched[dow].daySched[2].end=1700;
			sysCfg.thermostat1schedule.weekSched[dow].daySched[2].setpoint=1600;
			sysCfg.thermostat1schedule.weekSched[dow].daySched[2].active=1;
			
			sysCfg.thermostat1schedule.weekSched[dow].daySched[3].start=1700;
			sysCfg.thermostat1schedule.weekSched[dow].daySched[3].end=2200;
			sysCfg.thermostat1schedule.weekSched[dow].daySched[3].setpoint=2100;
			sysCfg.thermostat1schedule.weekSched[dow].daySched[3].active=1;

			sysCfg.thermostat1schedule.weekSched[dow].daySched[4].start=2200;
			sysCfg.thermostat1schedule.weekSched[dow].daySched[4].end=2400;
			sysCfg.thermostat1schedule.weekSched[dow].daySched[4].setpoint=1500;
			sysCfg.thermostat1schedule.weekSched[dow].daySched[4].active=1;
			
			sysCfg.thermostat1schedule.weekSched[dow].daySched[5].active=0; //Terminate
		}



		sysCfg.thermostat2state=0;
		sysCfg.thermostat2manualsetpoint=2100;
		sysCfg.thermostat2mode=THERMOSTAT_MANUAL;
		sysCfg.thermostat2opmode=THERMOSTAT_HEATING;

		//Build default schedule for the thermostat
		for(int dow=0; dow<7; dow++) {
			sysCfg.thermostat2schedule.weekSched[dow].daySched[0].start=0; //0am
			sysCfg.thermostat2schedule.weekSched[dow].daySched[0].end=600; //6am, hours are * 100
			sysCfg.thermostat2schedule.weekSched[dow].daySched[0].setpoint=1000; //10.0*C
			sysCfg.thermostat2schedule.weekSched[dow].daySched[0].active=1;
			
			sysCfg.thermostat2schedule.weekSched[dow].daySched[1].start=600;
			sysCfg.thermostat2schedule.weekSched[dow].daySched[1].end=900;
			sysCfg.thermostat2schedule.weekSched[dow].daySched[1].setpoint=1800;
			sysCfg.thermostat2schedule.weekSched[dow].daySched[1].active=1;
			
			sysCfg.thermostat2schedule.weekSched[dow].daySched[2].start=900;
			sysCfg.thermostat2schedule.weekSched[dow].daySched[2].end=1700;
			sysCfg.thermostat2schedule.weekSched[dow].daySched[2].setpoint=1600;
			sysCfg.thermostat2schedule.weekSched[dow].daySched[2].active=1;
			
			sysCfg.thermostat2schedule.weekSched[dow].daySched[3].start=1700;
			sysCfg.thermostat2schedule.weekSched[dow].daySched[3].end=2200;
			sysCfg.thermostat2schedule.weekSched[dow].daySched[3].setpoint=2100;
			sysCfg.thermostat2schedule.weekSched[dow].daySched[3].active=1;

			sysCfg.thermostat2schedule.weekSched[dow].daySched[4].start=2200;
			sysCfg.thermostat2schedule.weekSched[dow].daySched[4].end=2400;
			sysCfg.thermostat2schedule.weekSched[dow].daySched[4].setpoint=1500;
			sysCfg.thermostat2schedule.weekSched[dow].daySched[4].active=1;
			
			sysCfg.thermostat2schedule.weekSched[dow].daySched[5].active=0; //Terminate
		}


		sysCfg.thermostat3state=0;
		sysCfg.thermostat3manualsetpoint=2100;
		sysCfg.thermostat3mode=THERMOSTAT_MANUAL;
		sysCfg.thermostat3opmode=THERMOSTAT_HEATING;

		//Build default schedule for the thermostat
		for(int dow=0; dow<7; dow++) {
			sysCfg.thermostat3schedule.weekSched[dow].daySched[0].start=0; //0am
			sysCfg.thermostat3schedule.weekSched[dow].daySched[0].end=600; //6am, hours are * 100
			sysCfg.thermostat3schedule.weekSched[dow].daySched[0].setpoint=1000; //10.0*C
			sysCfg.thermostat3schedule.weekSched[dow].daySched[0].active=1;
			
			sysCfg.thermostat3schedule.weekSched[dow].daySched[1].start=600;
			sysCfg.thermostat3schedule.weekSched[dow].daySched[1].end=900;
			sysCfg.thermostat3schedule.weekSched[dow].daySched[1].setpoint=1800;
			sysCfg.thermostat3schedule.weekSched[dow].daySched[1].active=1;
			
			sysCfg.thermostat3schedule.weekSched[dow].daySched[2].start=900;
			sysCfg.thermostat3schedule.weekSched[dow].daySched[2].end=1700;
			sysCfg.thermostat3schedule.weekSched[dow].daySched[2].setpoint=1600;
			sysCfg.thermostat3schedule.weekSched[dow].daySched[2].active=1;
			
			sysCfg.thermostat3schedule.weekSched[dow].daySched[3].start=1700;
			sysCfg.thermostat3schedule.weekSched[dow].daySched[3].end=2200;
			sysCfg.thermostat3schedule.weekSched[dow].daySched[3].setpoint=2100;
			sysCfg.thermostat3schedule.weekSched[dow].daySched[3].active=1;

			sysCfg.thermostat3schedule.weekSched[dow].daySched[4].start=2200;
			sysCfg.thermostat3schedule.weekSched[dow].daySched[4].end=2400;
			sysCfg.thermostat3schedule.weekSched[dow].daySched[4].setpoint=1500;
			sysCfg.thermostat3schedule.weekSched[dow].daySched[4].active=1;
			
			sysCfg.thermostat3schedule.weekSched[dow].daySched[5].active=0; //Terminate
		}

		os_printf(" default configurations\r\n");

		CFG_Save();
	}

}
