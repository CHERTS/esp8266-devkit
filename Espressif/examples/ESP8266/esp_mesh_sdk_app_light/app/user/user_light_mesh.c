#include "user_light_mesh.h"
#include "user_light_hint.h"
#if ESP_MESH_SUPPORT
#include "mesh.h"

LIGHT_MESH_PROC LightMeshProc;
os_timer_t mesh_check_t;
os_timer_t mesh_tout_t;
os_timer_t mesh_user_t;

uint8 mesh_last_status=MESH_DISABLE;
LOCAL bool mesh_init_flag = true;

LOCAL char* mdev_mac = NULL;

void ICACHE_FLASH_ATTR
	mesh_MacIdInit()
{
	if(mdev_mac){
		os_printf("Mesh mdev_mac: %s \r\n",mdev_mac);
		return;
	}
	mdev_mac = (char*)os_zalloc(ESP_MESH_JSON_DEV_MAC_ELEM_LEN+1);	
	uint32 MAC_FLG = READ_PERI_REG(0x3ff00054);
	
    uint8 mac_sta[6] = {0};
    wifi_get_macaddr(STATION_IF, mac_sta);
	os_sprintf(mdev_mac,"\"mdev_mac\":\"%02X%02X%02X%02X%02X%02X\"",MAC2STR(mac_sta));

	#if 0
	MAC_FLG = ((MAC_FLG>>16)&0xff);
	if(MAC_FLG == 0){
		os_sprintf(mdev_mac,"\"mdev_mac\":\"18FE34%06X\"",system_get_chip_id());
	}else if(MAC_FLG == 1){
		os_sprintf(mdev_mac,"\"mdev_mac\":\"ACD074%06X\"",system_get_chip_id());
	}else{
		os_printf("dev mac error? 0x%02x\r\n",MAC_FLG);
		return;
	}
	#endif
	os_printf("Disp mdev_mac: %s\r\n",mdev_mac);

}

char* ICACHE_FLASH_ATTR
	mesh_GetMdevMac()
{
	if(mdev_mac){
	    return mdev_mac;
	}else{
		mesh_MacIdInit();
		return mdev_mac;
	}
}

/******************************************************************************
 * FunctionName : mesh_StopCheckTimer
 * Description  : Stop the mesh initialization status check
*******************************************************************************/
void ICACHE_FLASH_ATTR
	mesh_StopCheckTimer()
{
	os_timer_disarm(&mesh_check_t);
	os_timer_disarm(&mesh_tout_t);
}


/******************************************************************************
 * FunctionName : mesh_SetSoftap
 * Description  : If the device failed to join mesh network,
                  open the SoftAP interface for webserver
                  The SSID should not be the same form as that of the device in mesh network
*******************************************************************************/
void ICACHE_FLASH_ATTR
    mesh_SetSoftap()
{
    MESH_INFO("----------------------\r\n");
	MESH_INFO("MESH ENABLE SOFTAP \r\n");
	MESH_INFO("----------------------\r\n");

    struct softap_config config_softap;
    char ssid[33]={0};

    wifi_softap_get_config(&config_softap);
    os_memset(config_softap.password, 0, sizeof(config_softap.password));
    os_memset(config_softap.ssid, 0, sizeof(config_softap.ssid));
    os_sprintf(ssid,"ESP_%06X",system_get_chip_id());
    os_memcpy(config_softap.ssid, ssid, os_strlen(ssid));
    config_softap.ssid_len = os_strlen(ssid);
	
	config_softap.ssid_hidden = 0;
	config_softap.channel = wifi_get_channel();
	
#ifdef SOFTAP_ENCRYPT
    char password[33];
	char macaddr[6];

    os_sprintf(password, MACSTR "_%s", MAC2STR(macaddr), PASSWORD);
    os_memcpy(config_softap.password, password, os_strlen(password));
    config_softap.authmode = AUTH_WPA_WPA2_PSK;

#else
    os_memset(config_softap.password,0,sizeof(config_softap.password));
    config_softap.authmode = AUTH_OPEN;
#endif
	wifi_set_opmode(STATIONAP_MODE);
    wifi_softap_set_config(&config_softap);
	wifi_set_opmode(STATIONAP_MODE);
	
    wifi_softap_get_config(&config_softap);
	MESH_INFO("SSID: %s \r\n",config_softap.ssid);
	MESH_INFO("CHANNEL: %d \r\n",config_softap.channel);
	MESH_INFO("-------------------------\r\n");

}

/******************************************************************************
 * FunctionName : mesh_EnableCb
 * Description  : callback func for espconn_mesh_enable
                  enable callback will only be called if the device has already joined a mesh network
*******************************************************************************/
void ICACHE_FLASH_ATTR
	mesh_EnableCb()
{
	mesh_StopCheckTimer();
	_LINE_DESP();
	MESH_INFO("TEST IN MESH ENABLE CB\r\n");
    MESH_INFO("%s\n", __func__);
	MESH_INFO("HEAP: %d \r\n",system_get_free_heap_size());
	_LINE_DESP();
	if(LightMeshProc.mesh_suc_cb){
		LightMeshProc.mesh_suc_cb(NULL);
	}
}


/******************************************************************************
 * FunctionName : mesh_DisableCb
 * Description  : callback func when mesh is disabled
                  In this case, do nothing but display some info.
*******************************************************************************/
void ICACHE_FLASH_ATTR
    mesh_DisableCb()
{
	_LINE_DESP();
	MESH_INFO("TEST IN MESH DISABLE CB\r\n");
    MESH_INFO("%s\n", __func__);
	MESH_INFO("HEAP: %d \r\n",system_get_free_heap_size());
	_LINE_DESP();
}

/******************************************************************************
 * FunctionName : mesh_SuccessCb
 * Description  : callback func when mesh init finished successfully
                  In this demo , we run the platform code after mesh initialization
*******************************************************************************/
void ICACHE_FLASH_ATTR
	mesh_SuccessCb(void* arg)
{
	////light_hint_abort();
	////light_set_aim(0,0,0,22222,22222,1000,0);
	
	_LINE_DESP();
	MESH_INFO("mesh log: mesh success!\r\n");
	MESH_INFO("CONNECTED, DO RUN ESP PLATFORM...\r\n");
	MESH_INFO("mesh status: %d\r\n",espconn_mesh_get_status());
	_LINE_DESP();

	//show light status at the first time mesh enables
	if(mesh_init_flag){
        struct ip_info sta_ip;
        wifi_get_ip_info(STATION_IF,&sta_ip);
    	if( espconn_mesh_local_addr(&sta_ip.ip)){
    		MESH_INFO("THIS IS A MESH SUB NODE..\r\n");
    		uint32 mlevel = sta_ip.ip.addr&0xff;
    		light_ShowDevLevel(mlevel);//debug
    	}else{
    		MESH_INFO("THIS IS A MESH ROOT..\r\n");
    	    light_ShowDevLevel(1);//debug
    	}
		mesh_init_flag = false;
	}else{

	}
	
#if ESP_NOW_SUPPORT
	//init ESP-NOW ,so that light can be controlled by ESP-NOW SWITCHER.
	light_EspnowInit();
#endif
	WIFI_StartCheckIp();//debug
	//run esp-platform procedure,register to server.
	user_esp_platform_connect_ap_cb();//debug
	return;
}

/******************************************************************************
 * FunctionName : mesh_FailCb
 * Description  : callback func when mesh init failed(both timeout and failed)
*******************************************************************************/
void ICACHE_FLASH_ATTR
	mesh_FailCb(void* arg)
{
	////light_hint_abort();
	////light_set_aim(22222,0,0,5000,5000,1000,0);

	MESH_INFO("mesh fail\r\n");
	mesh_StopCheckTimer();
#if ESP_NOW_SUPPORT
	//initialize ESP-NOW
	light_EspnowInit();
#endif
	MESH_INFO("CALL MESH DISABLE HERE...\r\n");
	//stop or disable mesh
	espconn_mesh_disable(mesh_DisableCb);
	//open softap interface
	mesh_SetSoftap();
	//try AP CACHE and connect
	WIFI_StartAPScan();
	return;
	
}

/******************************************************************************
 * FunctionName : mesh_TimeoutCb
 * Description  : callback func when mesh init timeout
*******************************************************************************/
void ICACHE_FLASH_ATTR
	mesh_TimeoutCb(void* arg)
{
	MESH_INFO("MESH INIT TIMEOUT, STOP MESH\r\n");
	mesh_FailCb(NULL);
	return;
}

/******************************************************************************
 * FunctionName : mesh_GetStartMs
 * Description  : Get the time expire since the beginning of mesh initialization
                  Count in Ms
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
	mesh_GetStartMs()
{
	return (system_get_time()-LightMeshProc.start_time)/1000;
}

/******************************************************************************
 * FunctionName : mesh_InitStatusCheck
 * Description  : Only used in mesh init, to check the current status of mesh initialization,
                  and handle different situation accordingly
*******************************************************************************/
void ICACHE_FLASH_ATTR
	mesh_InitStatusCheck()
{
    os_timer_disarm(&mesh_check_t);
	sint8 mesh_status = espconn_mesh_get_status();
	MESH_INFO("--------------\r\n");
	MESH_INFO("mesh status: %d ; %d\r\n",mesh_status,system_get_free_heap_size());
	MESH_INFO("--------------\r\n");
	
#if LIGHT_DEVICE
	

#endif

	if(mesh_status == MESH_DISABLE){
		MESH_INFO("MESH DISABLE , RUN FAIL CB ,retry:%d \r\n",LightMeshProc.init_retry);
		if(LightMeshProc.init_retry<MESH_INIT_RETRY_LIMIT && (mesh_GetStartMs()<MESH_INIT_TIME_LIMIT)){
			LightMeshProc.init_retry+=1;
			espconn_mesh_enable(mesh_EnableCb, MESH_ONLINE);
			MESH_INFO("MESH RETRY : %d \r\n",LightMeshProc.init_retry);
		}else{
			mesh_StopCheckTimer();
			MESH_INFO("MESH INIT RETRY FAIL...\r\n");
			if(LightMeshProc.mesh_fail_cb){
				LightMeshProc.mesh_fail_cb(NULL);
			}
			LightMeshProc.init_retry = 0;
			return;
		}
	}
	else if(mesh_status==MESH_NET_CONN){
		MESH_INFO("MESH WIFI CONNECTED\r\n");
		mesh_StopCheckTimer();
	}
	os_timer_arm(&mesh_check_t,MESH_STATUS_CHECK_MS,0);
}




/******************************************************************************
 * FunctionName : mesh_ReconCheck
 * Description  : in case that some router would record the DNS info.
                  If we got the IP addr from DNS , and still can't connect to esp-server
                  Call this check func to try enable and connect again every 20s
*******************************************************************************/
void ICACHE_FLASH_ATTR
	mesh_ReconCheck()
{

	MESH_INFO("---------\r\n");
	MESH_INFO("mesh_ReconCheck\r\n");
	MESH_INFO("---------\r\n");

	os_timer_disarm(&mesh_user_t);
	sint8 mesh_status = espconn_mesh_get_status();
	MESH_INFO("MESH STATUS CHECK: %d \r\n",mesh_status);

	if(mesh_status == MESH_ONLINE_AVAIL){
		MESH_INFO("MESH ONLINE AVAIL\r\n");
		//user_esp_platform_sent_data();
		user_esp_platform_connect_ap_cb();
    }	//if(mesh_status == MESH_LOCAL_AVAIL){
    else{
		espconn_mesh_enable(mesh_EnableCb, MESH_ONLINE);
		os_timer_setfn(&mesh_user_t,mesh_ReconCheck,NULL);
		os_timer_arm(&mesh_user_t,20000,1);
		
	}
}


/******************************************************************************
 * FunctionName : mesh_StopReconnCheck
 * Description  : If we receive a packet from the server,stop the check task
                  called in platform to restart mesh if can not get any response from esp-server.
*******************************************************************************/
void ICACHE_FLASH_ATTR
	mesh_StopReconnCheck()
{
	//MESH_INFO("---------\r\n");
	//MESH_INFO("mesh_reconn_stop\r\n");
	//MESH_INFO("---------\r\n");
    os_timer_disarm(&mesh_user_t);
}


/******************************************************************************
 * FunctionName : mesh_StartReconnCheck
 * Description  : start a timer to check mesh status
                  called in platform(dns found) to restart mesh if can not get any response from esp-server.
*******************************************************************************/
void ICACHE_FLASH_ATTR
	mesh_StartReconnCheck(uint32 t)
{
	//ESP_DBG("\r\n\n\n\n\n-=======================\r\n");
	//os_printf("mesh_StartReconnCheck\r\n");
	//ESP_DBG("-=======================\r\n\n\n\n\n\n");
    os_timer_disarm(&mesh_user_t);
	os_timer_setfn(&mesh_user_t,mesh_ReconCheck,NULL);
	os_timer_arm(&mesh_user_t,t,0);
}



/******************************************************************************
 * FunctionName : user_MeshInit
 * Description  : mesh procedure init function
 * Parameters   : none
 * Returns      : none
 * Comments     :
                 Set callback function and start mesh.
                 MESH_ONLINE: CAN BE CONTROLLED BY WAN SERVER,AS WELL AS LAN APP VIA ROUTER.
                 MESH_LOCAL: CAN ONLY BE CONTROLLED BY LOCAL NETWORK.
*******************************************************************************/
void ICACHE_FLASH_ATTR
	user_MeshInit()
{
	mesh_MacIdInit();
	LightMeshProc.mesh_suc_cb=mesh_SuccessCb;
	LightMeshProc.mesh_fail_cb=mesh_FailCb;
	LightMeshProc.mesh_init_tout_cb=mesh_TimeoutCb;
	LightMeshProc.start_time = system_get_time();
	LightMeshProc.init_retry = 0;

    os_printf("test: %s\n", __func__);
	
	////light_shadeStart(HINT_WHITE,1000,0,1,NULL);
    user_MeshSetInfo();
    espconn_mesh_enable(mesh_EnableCb, MESH_ONLINE);
	//----add 151008---
	os_printf("mesh max hops: %d \r\n",espconn_mesh_get_max_hops());
	espconn_mesh_set_max_hops(5);
	os_printf("mesh set max hops: %d \r\n",espconn_mesh_get_max_hops());
	//----------------
	
	if(LightMeshProc.mesh_init_tout_cb){
    	os_timer_disarm(&mesh_tout_t);
    	os_timer_setfn(&mesh_tout_t,LightMeshProc.mesh_init_tout_cb,NULL);
    	os_timer_arm(&mesh_tout_t,MESH_TIME_OUT_MS,0);
	}
	os_timer_disarm(&mesh_check_t);
	os_timer_setfn(&mesh_check_t,mesh_InitStatusCheck,NULL);
	os_timer_arm(&mesh_check_t,MESH_STATUS_CHECK_MS,0);
}



/******************************************************************************
 * FunctionName : user_MeshSetInfo
 * Description  : set mesh node SSID,AUTH_MODE and PASSWORD
 * Parameters   : none
 * Returns      : none
 * Comments     : In this initial version, the MESH device is grouped by the specific SSID
				  Users can change the SSID and PASSWORD for SoftAP interface for the mesh nodes
 * NOTE         : Only call it once and before espconn_mesh_enable() 
*******************************************************************************/
void ICACHE_FLASH_ATTR
	user_MeshSetInfo()
{
#if 1
	uint8 group_id[6] = {0x18,0xfe,0x34,0x00,0x00,0x01};
	//espconn_mesh_init_group_list(group_id, 0);//CNT=1,MAC=GROUPID
	espconn_mesh_group_id_init(group_id,6);
	os_printf("==========================\r\n");
	os_printf("SET GROUP ID: "MACSTR"",MAC2STR(group_id));
	os_printf("==========================\r\n");
#endif	
	//If the device is in MESH mode,the SSID would finally be MESH_SSID_PREFIX_X_XXXXXX
	#if LIGHT_DEVICE
	    espconn_mesh_set_dev_type(ESP_DEV_LIGHT);
	#elif PLUG_DEVICE
	    espconn_mesh_set_dev_type(ESP_DEV_PLUG);
	#elif SENSOR_DEVICE
	    espconn_mesh_set_dev_type(ESP_DEV_HUMITURE);
	#endif

	if( espconn_mesh_set_ssid_prefix(MESH_SSID_PREFIX,os_strlen(MESH_SSID_PREFIX))){
		MESH_INFO("SSID PREFIX SET OK..\r\n");
	}else{
		MESH_INFO("SSID PREFIX SET ERROR..\r\n");
	}

	if(espconn_mesh_encrypt_init(MESH_AUTH,MESH_PASSWORD,os_strlen(MESH_PASSWORD))){
		MESH_INFO("SOFTAP ENCRYPTION SET OK..\r\n");
	}else{
		MESH_INFO("SOFTAP ENCRYPTION SET ERROR..\r\n");
	}
}

void ICACHE_FLASH_ATTR mesh_enable_task()
{

	//espconn_mesh_enable(mesh_EnableCb, MESH_ONLINE);
	espconn_mesh_enable(NULL, MESH_ONLINE);


}
#endif

