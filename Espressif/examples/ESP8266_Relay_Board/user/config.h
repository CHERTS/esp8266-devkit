/* config.h
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

#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_
#include "os_type.h"
#include "user_config.h"

#define THERMOSTAT_MANUAL 0
#define THERMOSTAT_AUTO 1

#define THERMOSTAT_HEATING 0
#define THERMOSTAT_COOLING 1

typedef struct {
 uint16_t start;
 uint16_t end;
 uint16_t setpoint;  // Degrees C in * 100 i.e. 2350=23.5*C
 uint16_t active; 	 // pad to 4 byte boundary
 } dayScheduleElement;
 
typedef struct {
dayScheduleElement daySched[8]; // Max 8 schedules per day
}  daySchedule;
 
typedef struct {
daySchedule weekSched[7]; // 7 days per week
}  weekSchedule;


typedef struct{
	
//4 byte alignment, hence uint32_t
uint32_t cfg_holder;

uint8_t sta_mode[8];
uint8_t sta_ip[16];
uint8_t sta_mask[16];
uint8_t sta_gw[16];
uint8_t sta_ssid[32];
uint8_t sta_pass[32];
uint32_t sta_type;

uint8_t ap_ip[32];
uint8_t ap_mask[32];
uint8_t ap_gw[32];

uint32_t httpd_port;
uint32_t httpd_auth;
uint8_t httpd_user[16];
uint8_t httpd_pass[16];

uint32_t broadcastd_enable;
uint32_t broadcastd_port;
uint8_t broadcastd_host[32];
uint8_t broadcastd_url[256];
uint32_t broadcastd_thingspeak_channel;
uint8_t broadcastd_ro_apikey[32];

uint32_t ntp_enable;
int32_t ntp_tz;

uint32_t mqtt_enable;
uint8_t mqtt_host[64];
uint32_t mqtt_port;
uint32_t mqtt_keepalive;
uint8_t mqtt_devid[32];
uint8_t mqtt_user[32];
uint8_t mqtt_pass[64];
uint32_t mqtt_use_ssl;
uint8_t mqtt_relay_subs_topic[64];
uint8_t mqtt_dht22_temp_pub_topic[64];
uint8_t mqtt_dht22_humi_pub_topic[64];
uint8_t mqtt_ds18b20_temp_pub_topic[64];

uint32_t sensor_ds18b20_enable;
uint32_t sensor_dht22_enable;
uint32_t sensor_dht22_humi_thermostat;

uint32_t relay_latching_enable;
uint32_t relay_1_state;
uint32_t relay_2_state;
uint32_t relay_3_state;
uint8_t relay1name[16];
uint8_t relay2name[16];
uint8_t relay3name[16];
	
uint32_t thermostat1state;
uint32_t thermostat1manualsetpoint;
uint32_t thermostat1mode;
uint32_t thermostat1opmode;
weekSchedule thermostat1schedule;

uint32_t thermostat2state;
uint32_t thermostat2manualsetpoint;
uint32_t thermostat2mode;
uint32_t thermostat2opmode;
weekSchedule thermostat2schedule;

uint32_t thermostat3state;
uint32_t thermostat3manualsetpoint;
uint32_t thermostat3mode;
uint32_t thermostat3opmode;
weekSchedule thermostat3schedule;

	
} SYSCFG;

typedef struct {
    uint8 flag;
    uint8 pad[3];
} SAVE_FLAG;

void CFG_Save();
void CFG_Load();

 
 
extern SYSCFG sysCfg;

#endif /* USER_CONFIG_H_ */
