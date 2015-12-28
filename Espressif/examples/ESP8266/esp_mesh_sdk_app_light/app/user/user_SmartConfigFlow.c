#include "c_types.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "user_SmartConfigFlow.h"
#include "user_light_hint.h"
#include "user_light_adj.h"
#include "user_light.h"
#include "mem.h"
#if ESP_MESH_SUPPORT
#include "mesh.h"
#endif

#if ESP_TOUCH_SUPPORT

ESPTOUCH_PROC  esptouch_proc;
os_timer_t esptouch_tout_t;
LOCAL bool esptouch_ack_flag = false;
#define SC_INFO os_printf



/******************************************************************************
 * FunctionName : esptouch_getAckFlag
 * Description  : return esptouch_ack_flag
                  esptouch_ack_flag: true : already did ESP-TOUCH, do not try it again
*******************************************************************************/
bool esptouch_getAckFlag()
{
	return esptouch_ack_flag;
}

/******************************************************************************
 * FunctionName : esptouch_setAckFlag
 * Description  : set esptouch_ack_flag
                  true : already did ESP-TOUCH, do not try it again
*******************************************************************************/
void esptouch_setAckFlag(bool flg)
{
	esptouch_ack_flag = flg;
}

/******************************************************************************
 * FunctionName : esptouch_ProcCb
 * Description  : esp-touch status change callback for smartconfig_start
*******************************************************************************/
LOCAL void esptouch_ProcCb(sc_status status, void *pdata)
{
	switch(status) {
		case SC_STATUS_WAIT:
			SC_INFO("SC_STATUS_WAIT\n");
			break;
		case SC_STATUS_FIND_CHANNEL:
			SC_INFO("SC_STATUS_FIND_CHANNEL\n");
			if(esptouch_proc.esptouch_start_cb){
				esptouch_proc.esptouch_start_cb(NULL);
			}
			break;
		case SC_STATUS_GETTING_SSID_PSWD:
			SC_INFO("SC_STATUS_GETTING_SSID_PSWD\n");
			#if LIGHT_DEVICE
			light_shadeStart(HINT_BLUE,1000,0,1,NULL);
			#endif
			if(esptouch_proc.esptouch_fail_cb){
				os_timer_disarm(&esptouch_tout_t);
				os_timer_setfn(&esptouch_tout_t,esptouch_proc.esptouch_fail_cb,NULL);
				os_timer_arm(&esptouch_tout_t,ESP_TOUCH_TIMEOUT_MS,0);
			}
			break;
		case SC_STATUS_LINK:
			SC_INFO("SC_STATUS_LINK\n");
			struct station_config *sta_conf = pdata;
			//os_printf();
			wifi_station_set_config(sta_conf);
			wifi_station_disconnect();
			
			//wifi_station_disconnect();
			//======================
			#if ESP_MESH_SUPPORT
			//mesh_enable_task();
			//user_MeshInit();
			//user_esp_platform_connect_ap_cb();
			#endif
			//======================
			wifi_station_connect();
			
			os_timer_disarm(&esptouch_tout_t);
			os_timer_arm(&esptouch_tout_t,ESPTOUCH_CONNECT_TIMEOUT_MS,0);
			#if LIGHT_DEVICE
			light_blinkStart(HINT_WHITE);
			#endif
			break;
		case SC_STATUS_LINK_OVER:
			os_timer_disarm(&esptouch_tout_t);
			SC_INFO("SC_STATUS_LINK_OVER\n");
			if (esptouch_proc.esptouch_type == SC_TYPE_ESPTOUCH) {
				uint8 phone_ip[4] = {0};
				os_memcpy(phone_ip, (uint8*)pdata, 4);
				SC_INFO("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
			}
			smartconfig_stop();
			SC_INFO("UPDATE PASSWORD HERE\r\n");
			if(esptouch_proc.esptouch_suc_cb){
			    esptouch_proc.esptouch_suc_cb(NULL);//run finish cb
			}
			break;
	}
	
}



/******************************************************************************
 * FunctionName : esptouch_SuccessCb
 * Description  : esp-touch success callback
*******************************************************************************/
void ICACHE_FLASH_ATTR
	esptouch_SuccessCb(void* data)
{
	wifi_set_opmode(STATIONAP_MODE);
	
	os_timer_disarm(&esptouch_tout_t);//disable check timeout 
	#if LIGHT_DEVICE
	light_hint_stop(HINT_WHITE);
	#endif
	SC_INFO("ESP-TOUCH SUCCESS \r\n");
	
	SC_INFO("ENABLE LIGHT ACTION(ESP-NOW)");
	SC_INFO("debug: channel:%d\r\n",wifi_get_channel());
#if ESP_MESH_SUPPORT
    if(MESH_DISABLE == espconn_mesh_get_status()){
	    //user_MeshInit();
	    //mesh_enable_task();
    }
    user_esp_platform_connect_ap_cb();
    //user_MeshInit();
#else
    user_esp_platform_connect_ap_cb();
#endif
#if ESP_NOW_SUPPORT
	light_EspnowInit();
#endif
	SC_INFO("CONNECTED TO AP...ENABLE MESH AND RUN PLATFORM CODE ...WAIT...\r\n");

}

/******************************************************************************
 * FunctionName : esptouch_FailCb
 * Description  : esp-touch fail callback
*******************************************************************************/
void ICACHE_FLASH_ATTR
	esptouch_FailCb(void* data)
{	
	wifi_station_disconnect();
	smartconfig_stop();
	wifi_set_opmode(STATIONAP_MODE);
	
	SC_INFO("ESP-TOUCH FAIL \r\n");
	os_timer_disarm(&esptouch_tout_t);
	#if LIGHT_DEVICE
	light_shadeStart(HINT_RED,2000,0,1,NULL);
	#endif
	
	SC_INFO("ENABLE LIGHT ACTION(ESP-NOW)");
	os_printf("debug: channel:%d\r\n",wifi_get_channel());
#if ESP_NOW_SUPPORT
	light_EspnowInit();
#endif

#if ESP_MESH_SUPPORT
	SC_INFO("ESP-TOUCH FAIL, OPEN WEBSERVER NOW");
	mesh_SetSoftap();//check
	SC_INFO("RESTART MESH NOW...\r\n");
	#if LIGHT_DEVICE
	light_hint_stop(HINT_RED);
	#endif
	user_MeshInit();
#endif
}

/******************************************************************************
 * FunctionName : esptouch_StartCb
 * Description  : esp-touch start callback
*******************************************************************************/
void ICACHE_FLASH_ATTR
	esptouch_StartCb(void* para)
{
    SC_INFO("LIGHT SHADE & START ESP-TOUCH");
	#if LIGHT_DEVICE
	light_shadeStart(HINT_GREEN,1000,0,1,NULL);
	#endif
}


/******************************************************************************
 * FunctionName : esptouch_FlowStart
 * Description  : Set esp-touch callback and start esp-touch
*******************************************************************************/
void ICACHE_FLASH_ATTR
	esptouch_FlowStart()
{
	
	wifi_station_disconnect();
#if ESP_NOW_SUPPORT
	////light_EspnowDeinit();
#endif
	WIFI_StopCheckIp();
	//esptouch_setAckFlag(true);
	
	SC_INFO("ESP-TOUCH FLOW INIT...\r\n");
	esptouch_proc.esptouch_fail_cb = esptouch_FailCb;
	esptouch_proc.esptouch_start_cb = esptouch_StartCb;
	esptouch_proc.esptouch_suc_cb = esptouch_SuccessCb;
	esptouch_proc.esptouch_type = SC_TYPE_ESPTOUCH;
	
    SC_INFO("ESP-TOUCH SET STATION MODE ...\r\n");
    wifi_set_opmode(STATION_MODE);

    if(esptouch_proc.esptouch_fail_cb){
    	os_timer_disarm(&esptouch_tout_t);
    	os_timer_setfn(&esptouch_tout_t,esptouch_proc.esptouch_fail_cb,NULL);
    	os_timer_arm(&esptouch_tout_t,ESP_TOUCH_TIME_ENTER,0);
    }
	SC_INFO("ESP-TOUCH START");
	smartconfig_start(esptouch_ProcCb);

}


#endif

