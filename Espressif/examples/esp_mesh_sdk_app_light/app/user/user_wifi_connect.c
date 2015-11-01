#include "user_config.h"


#include "user_wifi_connect.h"
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
#include "user_config.h"
#include "user_esp_platform.h"
#if ESP_MESH_SUPPORT
#include "mesh.h"
#endif
#define INFO os_printf



static ETSTimer WiFiLinker;
WifiCallback wifiCb = NULL;
static uint8_t wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;
static bool wifiReconFlg = false;
bool ap_cache_if = false;


void ICACHE_FLASH_ATTR WIFI_StopCheckIp();
void ICACHE_FLASH_ATTR WIFI_Connect(uint8_t* ssid, uint8_t* pass, WifiCallback cb);



//=====================================================================
//AP CACHE
//=====================================================================

os_timer_t ap_cache_t;
os_timer_t mesh_scan_t;
#include "user_light_mesh.h"

#define AP_CACHE_TOUT_MS 20000
uint8 ICACHE_FLASH_ATTR
    APCache_GetNum()
{
	struct station_config config[5];
	uint8 ap_record_num = wifi_station_get_ap_info(config);	
	INFO("AP CACHE NUM : %d \r\n",ap_record_num);
	return ap_record_num;
}

void ICACHE_FLASH_ATTR
	APCache_TimeoutCb()
{
	INFO("------------------\r\n");
	INFO("APCache_TimeoutCb\r\n");
	
	os_timer_disarm(&ap_cache_t);
	ap_cache_if = false;
	_LINE_DESP();
	INFO("AP CACHE TIMEOUT, STOP CONNECTING, START ESP-TOUCH");
	_LINE_DESP();
	
	wifi_station_disconnect();
	WIFI_StopCheckIp();//check
#if ESP_TOUCH_SUPPORT
	//INFO("ESP_TOUCH FLG: %d \r\n",esptouch_getAckFlag());
	if(false == esptouch_getAckFlag()){
	    esptouch_FlowStart();
		//esptouch_setAckFlag(true);
		return;
	}
#endif
#if ESP_MESH_SUPPORT
	//else{
	    INFO("AP CACHE TOUT,ALREADY DID ESP-TOUCH\r\n");
		mesh_SetSoftap();
		INFO("RE-SEARCH MESH NETWORK,in 5s \r\n");
		//user_StopClientTimer();
		//user_MeshInit();
		
		os_timer_disarm(&mesh_scan_t);
		os_timer_setfn(&mesh_scan_t,user_MeshInit,NULL);
		os_timer_arm(&mesh_scan_t,5000,0);
		return;
	//}
#endif

    WIFI_StartCheckIp();

}

uint8 ap_cache_record_num = 0;
uint8 ap_cache_record_flg = 0;

#if 0
bool ICACHE_FLASH_ATTR
	APCache_GetStatus()
{
	return ap_cache_if;
}
#endif

void ICACHE_FLASH_ATTR
	APCache_Connect()
{
	//INFO("------------------\r\n");
	//INFO("APCache_Connect\r\n");
	ap_cache_if=true;
	struct station_config config[5];
	uint8 ap_cnt = wifi_station_get_ap_info(config);	
	INFO("AP CACHE NUM : %d \r\n",ap_cnt);


	//uint8 ap_cnt = APCache_GetNum();
	uint8 ap_cur;
	if(ap_cnt>0){
		INFO("AP CACHE FIND, TRY CONNECTING WIFI \r\n");
		ap_cur = wifi_station_get_current_ap_id();		
		ap_cache_record_num = ap_cur;
		
        while (1) {
			ap_cur = ((ap_cur==AP_CACHE_NUMBER-1)?0:ap_cur+1);

			if(ap_cache_record_num == ap_cur){
				INFO("AC CACHE CONNECT FAIL...\r\n");
				break;
			}
			INFO("ap_cur: %d \r\n",ap_cur);
			INFO("DEBUG:SSID: %s\r\n",config[ap_cur].ssid);
			INFO("DEBUG:PASSWORD: %s \r\n",config[ap_cur].password);
    		if(wifi_station_ap_change(ap_cur) == true){
				INFO("----------------\r\n");				
				INFO("CURRENT AP NUM: %d \r\n",ap_cur);
				INFO("AP CACHE CONNECT: \r\n");
				INFO("SSID: %s\r\n",config[ap_cur].ssid);
				INFO("PASSWORD: %s \r\n",config[ap_cur].password);
				INFO("----------------\r\n");
				wifi_station_disconnect();
				//wifi_station_set_config(&config[ap_cur]);
				//wifi_station_connect();
				WIFI_Connect(config[ap_cur].ssid,config[ap_cur].password,NULL);
				
				INFO("CONNECT...\r\n");
				INFO("----------------\r\n");
				/*
				if(ap_cache_record_flg == 0){
					ap_cache_record_num = ap_cur;
					ap_cache_record_flg = 1;
				}else{
					if(ap_cache_record_num == ap_cur){
						APCache_TimeoutCb();
					}
				}
				*/
				
				break;
    		}


        }	
		if(ap_cache_record_num == ap_cur){
			APCache_TimeoutCb();
		}

	}else{
		_LINE_DESP();
		INFO("FIND NO AP CACHE,ENABLE ESPTOUCH NEXT\r\n");
		_LINE_DESP();
		os_timer_disarm(&ap_cache_t);
		ap_cache_if = false;
	#if ESP_TOUCH_SUPPORT
		INFO("ESP_TOUCH FLG: %d \r\n",esptouch_getAckFlag());
		if(false == esptouch_getAckFlag()){
			
			esptouch_FlowStart();
			//esptouch_setAckFlag(true);
			return;
		}
		
	#endif
	#if ESP_MESH_SUPPORT
	    //else{
			INFO("ALREADY DID ESP-TOUCH,RESTART MESH IN 60 S\r\n");
	        mesh_StopReconnCheck();
			mesh_SetSoftap();
			os_timer_disarm(&mesh_scan_t);
			os_timer_setfn(&mesh_scan_t,user_MeshInit,NULL);
			os_timer_arm(&mesh_scan_t,60000,0);
			//user_MeshInit();
		//}
	#endif
	}


}






//=======================================================================

#if ESP_MESH_SUPPORT
void ICACHE_FLASH_ATTR
	mesh_en_cb()
{
	INFO("--------------\r\n");
	INFO("MESH ENABLE CB in wifi connect\r\n");
	INFO("-------------\r\n");
	if(MESH_ONLINE_AVAIL == espconn_mesh_get_status()){
		INFO("MESH ONLINE, CONNECT TO SERVER...\r\n");
	    user_esp_platform_connect_ap_cb();
	}else{
		INFO("MESH NOT ONLINE: %d, DO NOT CONNECT SERVER...\r\n",espconn_mesh_get_status());
	}

}
#endif

void ICACHE_FLASH_ATTR
    WIFI_ConnectCb(uint8_t status)
{
	//INFO("------------------\r\n");
	//INFO("WIFI_ConnectCb\r\n");
    //wifi_station_set_auto_connect(1);
    if(wifiReconFlg && (ap_cache_if==false)){
		INFO("------------------\r\n");
	    INFO("001\r\n");
		wifiReconFlg = false;
		INFO("WIFI RECONN FLG: %d \r\n",wifiReconFlg);
		INFO("DO AP CACHE\r\n");
		os_timer_disarm(&ap_cache_t);
		os_timer_setfn(&ap_cache_t,APCache_TimeoutCb,NULL);
		os_timer_arm(&ap_cache_t,AP_CACHE_TOUT_MS,0);
		APCache_Connect();	
		return;
    }
    
    if(status == STATION_GOT_IP){
		//INFO("------------------\r\n");
	    //INFO("002\r\n");
		#if ESP_TOUCH_SUPPORT
		esptouch_setAckFlag(true);
		#endif
		os_timer_disarm(&ap_cache_t);
		ap_cache_if = false;
        INFO("WIFI CONNECTED , RUN ESP PLATFORM...\r\n");
		#if ESP_MESH_SUPPORT
    		if(MESH_DISABLE == espconn_mesh_get_status() ){
				_LINE_DESP();
    			INFO("CONNECTED TO ROUTER, ENABLE MESH\r\n");
				_LINE_DESP();
				
				espconn_mesh_enable(mesh_en_cb,MESH_ONLINE);//debug
				//user_esp_platform_connect_ap_cb();//debug
    		}
	    #else
		    user_esp_platform_connect_ap_cb();
		#endif
    }else{
		//if((status != STATION_IDLE)&&(status != STATION_CONNECTING)){
		//INFO("------------------\r\n");
	    //INFO("003\r\n");
		if(status != STATION_CONNECTING){
			//_LINE_DESP();
			INFO("---------------------\r\n");
			INFO("STATION STATUS: %d \r\n",status);
			INFO("---------------------\r\n");
			//_LINE_DESP();
			APCache_Connect();
		}

	}
}



static void ICACHE_FLASH_ATTR WIFI_CheckIp(void *arg)
{
	//INFO("-----------------\r\n");
	//INFO("WIFI_CheckIp \r\n");
	
	os_timer_disarm(&WiFiLinker);

	#if ESP_MESH_SUPPORT
		if(MESH_DISABLE != espconn_mesh_get_status()){
			/*MESH layer would handle wifi status exception at first*/
			//os_timer_setfn(&WiFiLinker, (os_timer_func_t *)WIFI_CheckIp, NULL);
			os_timer_arm(&WiFiLinker, 1000, 0);
			return;
		}else{
			wifiReconFlg = true;
			INFO("wifiReconFlg : %d \r\n",wifiReconFlg);
			INFO("ap_cache_if: %d \r\n",ap_cache_if);
			
			INFO("MESH STATUS : %d \r\n",espconn_mesh_get_status());
			INFO("WIFI STATUS : CUR:%d ; LAST:%d\r\n",wifiStatus,lastWifiStatus);
			INFO("----------------\r\n");
			//user_StopClientTimer();
		}
	#endif
	
	struct ip_info ipConfig;
	wifi_get_ip_info(STATION_IF, &ipConfig);
	wifiStatus = wifi_station_get_connect_status();
	if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0)
	{
		//os_timer_setfn(&WiFiLinker, (os_timer_func_t *)WIFI_CheckIp, NULL);
		//INFO("----------------\r\n");
	    //INFO("STATION_GOT_IP\r\n");
		os_timer_arm(&WiFiLinker, 2000, 0);
	}
	else
	{
		if(wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
		{
			INFO("----------------\r\n");
			INFO("STATION_WRONG_PASSWORD\r\n");
			//wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_NO_AP_FOUND)
		{
		    INFO("----------------\r\n");
			INFO("STATION_NO_AP_FOUND\r\n");
			//wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_CONNECT_FAIL)
		{
		    INFO("----------------\r\n");
			INFO("STATION_CONNECT_FAIL\r\n");
			//wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_CONNECTING)
		{
		    INFO("----------------\r\n");
			INFO("STATION_CONNECTING\r\n");
			//wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_IDLE)
		{
		    INFO("----------------\r\n");
			INFO("STATION_IDLE\r\n");
			//INFO("TEST STATION STATUS: %d \r\n",wifi_station_get_connect_status());
		}else{
			INFO("STATUS ERROR\r\n");
		}
		//os_timer_setfn(&WiFiLinker, (os_timer_func_t *)WIFI_CheckIp, NULL);
		os_timer_arm(&WiFiLinker, 1000, 0);
	}
	if(wifiStatus != lastWifiStatus || wifiReconFlg){
		lastWifiStatus = wifiStatus;
		if(wifiCb){
			wifiCb(wifiStatus);
		}else{
			WIFI_ConnectCb(wifiStatus);
		}
	}
}


void ICACHE_FLASH_ATTR 
	WIFI_Connect(uint8_t* ssid, uint8_t* pass, WifiCallback cb)
{
	struct station_config stationConf;

	INFO("WIFI_INIT\r\n");
	wifi_set_opmode(STATIONAP_MODE);//
	//wifi_station_set_auto_connect(FALSE);
	if(cb){
	    wifiCb = cb;
	}else{
		wifiCb = WIFI_ConnectCb;
	}

	os_memset(&stationConf, 0, sizeof(struct station_config));

	os_sprintf(stationConf.ssid, "%s", ssid);
	os_sprintf(stationConf.password, "%s", pass);

	wifi_station_set_config(&stationConf);

	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)WIFI_CheckIp, NULL);
	os_timer_arm(&WiFiLinker, 1000, 0);

	//wifi_station_set_auto_connect(TRUE);
	wifi_station_connect();
}

void ICACHE_FLASH_ATTR
	WIFI_StopCheckIp()
{
	os_timer_disarm(&WiFiLinker);
}


void ICACHE_FLASH_ATTR
	WIFI_StartCheckIp()
{
	lastWifiStatus = wifiStatus;
	
	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)WIFI_CheckIp, NULL);
    WIFI_CheckIp(NULL);
}



void ICACHE_FLASH_ATTR
	WIFI_StartAPScan()
{
	INFO("WIFI AP SCAN START\r\n");

	//start a timer to check ap cache timeout;
	os_timer_disarm(&ap_cache_t);
	os_timer_setfn(&ap_cache_t,APCache_TimeoutCb,NULL);
	os_timer_arm(&ap_cache_t,AP_CACHE_TOUT_MS,0);
	ap_cache_record_flg = 0;
	ap_cache_record_num = 0;
	APCache_Connect();
}




