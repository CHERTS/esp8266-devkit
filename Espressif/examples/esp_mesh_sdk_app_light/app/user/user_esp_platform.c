/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_esp_platform.c
 *
 * Description: The client mode configration.
 *              Check your hardware connection with the host while use this mode.
 *
 * Modification history:
 *     2014/5/09, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"

#include "espconn.h"
#include "user_esp_platform.h"
#include "user_iot_version.h"
#include "upgrade.h"
#include "esp_send.h"

#if ESP_MESH_SUPPORT
	#include "mesh.h"
#endif
#include "user_webserver.h"
#include "user_json.h"
#include "user_simplepair.h"


#if ESP_PLATFORM

#define ESP_DEBUG

#ifdef ESP_DEBUG
#define ESP_DBG os_printf
#else
#define ESP_DBG
#endif

#define ACTIVE_FRAME    "{\"nonce\": %u,\"path\": \"/v1/device/activate/\", \"method\": \"POST\", \"body\": {\"encrypt_method\": \"PLAIN\", \"token\": \"%s\", \"bssid\": \""MACSTR"\",\"rom_version\":\"%s\"}, \"meta\": {\"Authorization\": \"token %s\",\"mode\":\"v\"}}\n"

#if PLUG_DEVICE
#include "user_plug.h"

#define RESPONSE_FRAME  "{\"status\": 200, \"datapoint\": {\"x\": %d}, \"nonce\": %u, \"deliver_to_device\": true}\n"
#define FIRST_FRAME     "{\"nonce\": %u, \"path\": \"/v1/device/identify\", \"method\": \"GET\",\"meta\": {\"Authorization\": \"token %s\"}}\n"

#elif LIGHT_DEVICE
#include "user_light.h"
#include "user_light_mesh.h"
#include "user_light_hint.h"

#define RESPONSE_FRAME  "{\"status\": 200,\"nonce\": %u, \"datapoint\": {\"x\": %d,\"y\": %d,\"z\": %d,\"k\": %d,\"l\": %d},\"deliver_to_device\":true}\n"
#define FIRST_FRAME     "{\"nonce\": %u, \"path\": \"/v1/device/identify\", \"method\": \"GET\",\"meta\": {\"mode\":\"v\",\"Authorization\": \"token %s\"}}\n"
#define MESH_UPGRADE    "{\"nonce\": %u, \"get\": {\"action\": \"download_rom_base64\", \"version\": \"%s\", \"filename\": \"%s\", \"offset\": %d, \"size\": %d}, \"meta\": {\"Authorization\": \"token %s\"}, \"path\": \"/v1/device/rom/\", \"method\": \"GET\"}\n"
#elif SENSOR_DEVICE
#include "user_sensor.h"

#if HUMITURE_SUB_DEVICE
#define UPLOAD_FRAME  "{\"nonce\": %u, \"path\": \"/v1/datastreams/tem_hum/datapoint/\", \"method\": \"POST\", \
\"body\": {\"datapoint\": {\"x\": %s%d.%02d,\"y\": %d.%02d}}, \"meta\": {\"Authorization\": \"token %s\"}}\n"
#elif FLAMMABLE_GAS_SUB_DEVICE
#define UPLOAD_FRAME  "{\"nonce\": %u, \"path\": \"/v1/datastreams/flammable_gas/datapoint/\", \"method\": \"POST\", \
\"body\": {\"datapoint\": {\"x\": %d.%03d}}, \"meta\": {\"Authorization\": \"token %s\"}}\n"
#endif

LOCAL uint32 count = 0;
#endif

#define UPGRADE_FRAME  "{\"path\": \"/v1/messages/\", \"method\": \"POST\", \"meta\": {\"Authorization\": \"token %s\"},\
\"get\":{\"action\":\"%s\"},\"body\":{\"pre_rom_version\":\"%s\",\"rom_version\":\"%s\"}}\n"

#if PLUG_DEVICE || LIGHT_DEVICE
#define BEACON_FRAME    "{\"path\": \"/v1/ping/\", \"method\": \"POST\",\"meta\": {\"Authorization\": \"token %s\"}}\n"
#define RPC_RESPONSE_FRAME  "{\"status\": %d, \"nonce\": %u, \"deliver_to_device\": true}\n"
#define RPC_RESPONSE_ELEMENT  "\"status\": %d, \"nonce\": %u, \"deliver_to_device\": true"
#define TIMER_FRAME     "{\"body\": {}, \"get\":{\"is_humanize_format_simple\":\"true\"},\"meta\": {\"Authorization\": \"Token %s\"},\"path\": \"/v1/device/timers/\",\"post\":{},\"method\": \"GET\"}\n"
#define pheadbuffer "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36 \r\n\
Accept: */*\r\n\
Authorization: token %s\r\n\
Accept-Encoding: gzip,deflate,sdch\r\n\
Accept-Language: zh-CN,zh;q=0.8\r\n\r\n"

LOCAL uint8 ping_status;
LOCAL os_timer_t beacon_timer;
#endif

#ifdef USE_DNS
ip_addr_t esp_server_ip;
#endif


//extern u8 g_ip_filter;
//LOCAL bool g_sent_identify = false;
LOCAL char *g_sip = NULL, *g_sport = NULL,*g_mac = NULL;
LOCAL struct espconn user_conn;
LOCAL struct _esp_tcp user_tcp;
LOCAL os_timer_t client_timer;
 struct esp_platform_saved_param esp_param;
LOCAL uint8 device_status;
LOCAL uint8 device_recon_count = 0;
LOCAL uint32 active_nonce = 0xffffffff;
LOCAL uint8 iot_version[IOT_BIN_VERSION_LEN + 1] = {0};
LOCAL uint8_t g_mesh_version[IOT_BIN_VERSION_LEN + 1] = {0};
LOCAL uint8_t g_devkey[TOKEN_SIZE] = {0};//
//LOCAL uint8 mdns_init_flg = 0;
struct rst_info rtc_info;



void user_esp_platform_check_ip(uint8 reset_flag);
LOCAL void user_esp_platform_sent_beacon(struct espconn *pespconn);


uint32 ota_start_time=0,ota_finished_time=0;
#if ESP_MESH_SUPPORT

bool mesh_json_add_elem(char *pbuf, size_t buf_size, char *elem, size_t elem_size);
uint32 get_ota_start_time()
{
	return ota_start_time;
}
uint32 get_ota_finish_time()
{
	return ota_start_time;
}
#endif



char* ICACHE_FLASH_ATTR user_json_find_section(const char *pbuf,uint16 len,const char* section)
{
#if 0
	return espconn_json_find_section(pbuf,len,section);
#else
    char* sect = NULL,c;
	bool json_split = false;
	if(!pbuf || !section)
		return NULL;

	sect = (char*)os_strstr(pbuf,section);
    /*
    * the formal json format
    *{"elem":"value"}\r\n
    *{"elem":value}\r\n
    */
    if(sect){
        sect += os_strlen(section);
		while((uint32)sect<(uint32)(pbuf + len)){
            c = *sect++;
			if(c == ':' && !json_split){
                json_split = true;
				continue;
			}
			//{"elem":"value"}\r\n
			if(c == '"'){
                break;
			}
			//{"elem":value}\r\n
			//{"elem":  value}\r\n
			//{"elem":\tvalue}\r\n
			//{"elem":\nvalue}\r\n
			//{"elem":\rvalue}\r\n
			if(c != ' ' && c!='\n' && c!='\r'){
                sect--;
				break;
			}
		}
		return sect == NULL || (uint32)sect - (uint32)pbuf >= len?NULL:sect ;

    }

#endif
}

uint32 ICACHE_FLASH_ATTR
	user_JsonGetValueInt(const char* pbuffer,uint16 buf_len,const uint8* json_key)
{
#if 0
    return user_json_get_value(pbuffer,buf_len,json_key);
#else
    char val[20];
	uint32 result = 0;
	char* pstr = NULL,*pparse = NULL;
	uint16 val_size=0;

	pstr = user_json_find_section(pbuffer,buf_len,json_key);
	if(pstr){
        //find the end char of value
        if( (pparse = (char*)os_strstr(pstr,","))!= NULL ||
			(pparse = (char*)os_strstr(pstr,"\""))!= NULL ||
			(pparse = (char*)os_strstr(pstr,"}"))!= NULL ||
			(pparse = (char*)os_strstr(pstr,"]"))!= NULL){
			val_size = pparse - pstr;
        }else{
            return 0;
        }
	    os_memset(val,0,sizeof(val));

		if(val_size > sizeof(val))
			os_memcpy(val,pstr,sizeof(val));
		else
			os_memcpy(val,pstr,val_size);

		result = atol(val);

	}
	return result;

#endif

}

sint8 ICACHE_FLASH_ATTR espconn_esp_sent(struct espconn *pconn, uint8_t *buf, uint16_t len)
{
#if 0
#ifdef CLIENT_SSL_ENABLE
    espconn_secure_sent(pconn, buf, len);
#else
    espconn_sent(pconn, buf, len);
#endif
    os_timer_disarm(&beacon_timer);
    os_timer_setfn(&beacon_timer, (os_timer_func_t *)user_esp_platform_sent_beacon, &user_conn);
    os_timer_arm(&beacon_timer, BEACON_TIME, 0);
#else
	sint8 res;
	bool queue_empty = espSendQueueIsEmpty(espSendGetRingbuf());
	os_printf("send in espconn_esp_sent\r\n");
    res = espSendEnq(buf, len, pconn, ESP_DATA,TO_SERVER,espSendGetRingbuf());
	if(res==-1){
		os_printf("espconn send error , no buf in app...\r\n");
	}

	/*send the packet if platform sending queue is empty*/
	/*if not, espconn sendcallback would post another sending event*/
	if(queue_empty){
		system_os_post(ESP_SEND_TASK_PRIO, 0, (os_param_t)espSendGetRingbuf());
	}
	return res;
#endif
}
#if 0
/******************************************************************************
 * FunctionName : user_esp_platform_load_param
 * Description  : load parameter from flash, toggle use two sector by flag value.
 * Parameters   : param--the parame point which write the flash
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_load_param_t(struct esp_platform_saved_param *param)
{
    struct esp_platform_sec_flag_param flag;

    spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
                   (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));

    if (flag.flag == 0) {
        spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0) * SPI_FLASH_SEC_SIZE,
                       (uint32 *)param, sizeof(struct esp_platform_saved_param));
    } else {
        spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1) * SPI_FLASH_SEC_SIZE,
                       (uint32 *)param, sizeof(struct esp_platform_saved_param));
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_save_param
 * Description  : toggle save param to two sector by flag value,
 *              : protect write and erase data while power off.
 * Parameters   : param -- the parame point which write the flash
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_save_param_t(struct esp_platform_saved_param *param)
{
    struct esp_platform_sec_flag_param flag;

    spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
                   (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));

    if (flag.flag == 0) {
        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1);
        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)param, sizeof(struct esp_platform_saved_param));
        flag.flag = 1;
        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_FLAG);
        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));
    } else {
        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0);
        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)param, sizeof(struct esp_platform_saved_param));
        flag.flag = 0;
        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_FLAG);
        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));
    }
}
#endif
/******************************************************************************
 * FunctionName : user_esp_platform_get_token
 * Description  : get the espressif's device token
 * Parameters   : token -- the parame point which write the flash
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_get_token(uint8_t *token)
{
    if (token == NULL) {
        return;
    }

    os_memcpy(token, esp_param.token, sizeof(esp_param.token));
}

void ICACHE_FLASH_ATTR
user_esp_platform_get_devkey(uint8_t *devkey)
{
    if (devkey == NULL) {
        return;
    }

    os_memcpy(devkey, esp_param.devkey, sizeof(esp_param.devkey));
}


/******************************************************************************
 * FunctionName : user_esp_platform_set_token
 * Description  : save the token for the espressif's device
 * Parameters   : token -- the parame point which write the flash
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_set_token(uint8_t *token)
{
    if (token == NULL || os_strlen(token) > sizeof(esp_param.token)) {
        return;
    }

    esp_param.activeflag = 0;
    os_memcpy(esp_param.token, token, os_strlen(token));

    system_param_save_with_protect(ESP_PARAM_START_SEC, &esp_param, sizeof(esp_param));
}

/******************************************************************************
 * FunctionName : user_esp_platform_set_active
 * Description  : set active flag
 * Parameters   : activeflag -- 0 or 1
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_set_active(uint8 activeflag)
{
    esp_param.activeflag = activeflag;

    system_param_save_with_protect(ESP_PARAM_START_SEC, &esp_param, sizeof(esp_param));
}

void ICACHE_FLASH_ATTR
user_esp_platform_set_connect_status(uint8 status)
{
    device_status = status;
}

/******************************************************************************
 * FunctionName : user_esp_platform_get_connect_status
 * Description  : get each connection step's status
 * Parameters   : none
 * Returns      : status
*******************************************************************************/
uint8 ICACHE_FLASH_ATTR
user_esp_platform_get_connect_status(void)
{
    uint8 status = wifi_station_get_connect_status();

    if (status == STATION_GOT_IP) {
        status = (device_status == 0) ? DEVICE_CONNECTING : device_status;
    }

    ESP_DBG("status %d\n", status);
    return status;
}

/******************************************************************************
 * FunctionName : user_esp_platform_parse_nonce
 * Description  : parse the device nonce
 * Parameters   : pbuffer -- the recivce data point
 * Returns      : the nonce
*******************************************************************************/
int ICACHE_FLASH_ATTR
user_esp_platform_parse_nonce(char *pbuffer, uint16_t buf_len)
{
    return user_JsonGetValueInt(pbuffer, buf_len, "\"nonce\"");
    //return user_json_get_value(pbuffer, buf_len, "\"nonce\"");
}

/******************************************************************************
 * FunctionName : user_esp_platform_get_info
 * Description  : get and update the espressif's device status
 * Parameters   : pespconn -- the espconn used to connect with host
 *                pbuffer -- prossing the data point
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_get_info(struct espconn *pconn, uint8 *pbuffer)
{
	char pbuf[800];
    int nonce = 0;

    os_memset(pbuf, 0, sizeof(pbuf));

    nonce = user_esp_platform_parse_nonce(pbuffer, 1460);

    if (pbuf != NULL) {
#if PLUG_DEVICE
        os_sprintf(pbuf, RESPONSE_FRAME, user_plug_get_status(), nonce);
#elif LIGHT_DEVICE
        uint32 white_val;

        white_val = (PWM_CHANNEL>LIGHT_COLD_WHITE?user_light_get_duty(LIGHT_COLD_WHITE):0);
        os_sprintf(pbuf, RESPONSE_FRAME, nonce, user_light_get_period(),
                   user_light_get_duty(LIGHT_RED), user_light_get_duty(LIGHT_GREEN),
                   user_light_get_duty(LIGHT_BLUE),white_val );//50);
#endif

#if ESP_MESH_SUPPORT

		if(g_mac){
			mesh_json_add_elem(pbuf, sizeof(pbuf), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
		}else{
			g_mac = (char*)mesh_GetMdevMac();
			mesh_json_add_elem(pbuf, sizeof(pbuf), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
		}
#endif
        ESP_DBG("user_esp_platform_get_info %s\n", pbuf);
        espconn_esp_sent(pconn, pbuf, os_strlen(pbuf));
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_set_info
 * Description  : prossing the data and controling the espressif's device
 * Parameters   : pespconn -- the espconn used to connect with host
 *                pbuffer -- prossing the data point
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_set_info(struct espconn *pconn, uint8 *pbuffer,uint8 cmd_type)
{
    char *pstr = NULL;
#if PLUG_DEVICE
    pstr = (char *)os_strstr(pbuffer, "plug-status");

    if (pstr != NULL) {
        pstr = (char *)os_strstr(pbuffer, "body");
        if (pstr != NULL) {
            if (os_strncmp(pstr + 27, "1", 1) == 0) {
                user_plug_set_status(0x01);
            } else if (os_strncmp(pstr + 27, "0", 1) == 0) {
                user_plug_set_status(0x00);
            }
        }
    }
#elif LIGHT_DEVICE
    //char *pdata = NULL;
    char *pbuf = NULL;
    //char recvbuf[10];    
    uint16 length = 0;
    uint32 data = 0;
    static uint32 rr,gg,bb,cw,ww,period=1000;
	static uint8 ctrl_mode = 0;
	int check_group;
    ww=0;
    cw=0;
    extern uint8 light_sleep_flg;

	if(cmd_type==CMD_TYPE_UNICAST){
		os_printf("UNICAST CMD...\r\n");
            pstr = (char *)os_strstr(pbuffer, "{\"datapoint\": ");
            if (pstr != NULL) {
                pbuf = (char *)os_strstr(pbuffer, "}}");
                length = pbuf - pstr;
                length += 2;
                //pdata = (char *)os_zalloc(length + 1);
                //os_memcpy(pdata, pstr, length);
    
                period = user_JsonGetValueInt(pstr, length, "\"x\"");
                //user_light_set_period(data);
                rr = user_JsonGetValueInt(pstr, length, "\"y\"");
                gg = user_JsonGetValueInt(pstr, length, "\"z\"");
                bb = user_JsonGetValueInt(pstr, length, "\"k\"");
    			cw = user_JsonGetValueInt(pstr, length, "\"l\"");
    			ww = cw;
            }

		
		if((rr|gg|bb|cw|ww) == 0){
			if(light_sleep_flg==0){
				
			}
			
		}else{
			if(light_sleep_flg==1){
				os_printf("modem sleep en\r\n");
				wifi_set_sleep_type(MODEM_SLEEP_T);
				light_sleep_flg =0;
			}
		}
		light_set_aim(rr,gg,bb,cw,ww,period,ctrl_mode);
		//user_light_restart();
	}
#if ESP_MESH_SUPPORT
	else if(cmd_type==CMD_TYPE_MULITCAST){
		os_printf("MULTICAST CMD...\r\n");
		check_group = webserver_RootCheckGroupMsg(pbuffer);
		os_printf("check group res: %d \r\n",check_group);
		if(MULTICAST_CMD_ROOT_INGROUP==check_group || MULTICAST_CMD_NODE_INGROUP==check_group){
    		os_printf("glen:%p \r\n",user_json_find_section(pbuffer, length, "\"glen\""));
    		os_printf("group:%p \r\n",user_json_find_section(pbuffer, length, "\"group\""));
    			
    	    if(os_strstr(pbuffer,"\"glen\"") &&
    			os_strstr(pbuffer,"\"group\"")){
    			os_printf("tt1\r\n");
    			pstr = (char *)os_strstr(pbuffer, "\"datapoint\":");
                if (pstr != NULL) {
                    pbuf = (char *)os_strstr(pbuffer, "}");
                    length = pbuf - pstr;
                    length += 1;
        
                    period = user_JsonGetValueInt(pstr, length, "\"x\"");
                    rr = user_JsonGetValueInt(pstr, length, "\"y\"");
                    gg = user_JsonGetValueInt(pstr, length, "\"z\"");
                    bb = user_JsonGetValueInt(pstr, length, "\"k\"");
        			cw = user_JsonGetValueInt(pstr, length, "\"l\"");
        			ww = cw;
                }
    			os_printf("tt2: %d %d %d %d %d %d ; \r\n",period,rr,gg,bb,cw,ww);
    	    }
			
			if((rr|gg|bb|cw|ww) == 0){
				if(light_sleep_flg==0){
				}
				
			}else{
				if(light_sleep_flg==1){
					os_printf("modem sleep en\r\n");
					wifi_set_sleep_type(MODEM_SLEEP_T);
					light_sleep_flg =0;
				}
			}
			light_set_aim(rr,gg,bb,cw,ww,period,ctrl_mode);
			//user_light_restart();
		}
	
	}
#endif


#endif


#if ESP_MESH_SUPPORT
    if(cmd_type==CMD_TYPE_MULITCAST && MULTICAST_CMD_ROOT_INGROUP==check_group){
        user_esp_platform_get_info(pconn, pbuffer);
    }else if(cmd_type == CMD_TYPE_UNICAST){
        user_esp_platform_get_info(pconn, pbuffer);
    }
#else
    user_esp_platform_get_info(pconn, pbuffer);
#endif

	os_printf("end of platform_set_info\r\n");
}

/******************************************************************************
 * FunctionName : user_esp_platform_reconnect
 * Description  : reconnect with host after get ip
 * Parameters   : pespconn -- the espconn used to reconnect with host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_reconnect(struct espconn *pespconn)
{
    ESP_DBG("user_esp_platform_reconnect\n");

    if (wifi_get_opmode() == SOFTAP_MODE)
        return;

    //g_sent_identify = false;
    user_esp_platform_check_ip(0);
}

/******************************************************************************
 * FunctionName : user_esp_platform_discon_cb
 * Description  : disconnect successfully with the host
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_discon_cb(void *arg)
{
    struct espconn *pespconn = arg;
    struct ip_info ipconfig;
	struct dhcp_client_info dhcp_info;
    ESP_DBG("user_esp_platform_discon_cb\n");

#if (PLUG_DEVICE || LIGHT_DEVICE)
    os_timer_disarm(&beacon_timer);
#endif

    if (pespconn == NULL) {
        return;
    }

    pespconn->proto.tcp->local_port = espconn_port();

#if (PLUG_DEVICE || SENSOR_DEVICE)
    user_link_led_output(1);
#endif

#if SENSOR_DEVICE
#ifdef SENSOR_DEEP_SLEEP

    if (wifi_get_opmode() == STATION_MODE) {
    	/***add by tzx for saving ip_info to avoid dhcp_client start****/
    	wifi_get_ip_info(STATION_IF, &ipconfig);

    	dhcp_info.ip_addr = ipconfig.ip;
    	dhcp_info.netmask = ipconfig.netmask;
    	dhcp_info.gw = ipconfig.gw ;
    	dhcp_info.flag = 0x01;
    	os_printf("dhcp_info.ip_addr = %d\n",dhcp_info.ip_addr);
    	system_rtc_mem_write(64,&dhcp_info,sizeof(struct dhcp_client_info));
        user_sensor_deep_sleep_enter();
    } else {
        os_timer_disarm(&client_timer);
        os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_reconnect, pespconn);
        os_timer_arm(&client_timer, SENSOR_DEEP_SLEEP_TIME / 1000, 0);
    }

#else
    os_timer_disarm(&client_timer);
    os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_reconnect, pespconn);
    os_timer_arm(&client_timer, 1000, 0);
#endif
#else
    user_esp_platform_reconnect(pespconn);
#endif
}

/******************************************************************************
 * FunctionName : user_esp_platform_discon
 * Description  : A new incoming connection has been disconnected.
 * Parameters   : espconn -- the espconn used to disconnect with host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_discon(struct espconn *pespconn)
{
    ESP_DBG("user_esp_platform_discon\n");

#if (PLUG_DEVICE || SENSOR_DEVICE)
    user_link_led_output(1);
#endif


#if ESP_MESH_SUPPORT
    #if ESP_MESH_STRIP
    espconn_mesh_disconnect(pespconn);
	#else
	espconn_disconnect(pespconn);
	#endif
#else
#ifdef CLIENT_SSL_ENABLE
    espconn_secure_disconnect(pespconn);
#else
    espconn_disconnect(pespconn);
#endif
#endif
}

/******************************************************************************
 * FunctionName : user_esp_platform_sent_cb
 * Description  : Data has been sent successfully and acknowledged by the remote host.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_sent_cb(void *arg)
{
    struct espconn *pespconn = arg;

    ESP_DBG("user_esp_platform_sent_cb\n");
    os_timer_disarm(&beacon_timer);
    os_timer_setfn(&beacon_timer, (os_timer_func_t *)user_esp_platform_sent_beacon, &user_conn);
    os_timer_arm(&beacon_timer, BEACON_TIME, 0);

	espSendAck(espSendGetRingbuf());
#if 0  //
	//espSendQueueUpdate(espSendGetRingbuf());//update once send res == 0
	if(espSendQueueIsEmpty(espSendGetRingbuf())) 
		return;
	else 
		system_os_post(ESP_SEND_TASK_PRIO, 0, (os_param_t)espSendGetRingbuf());
#endif
		
}

/******************************************************************************
 * FunctionName : user_esp_platform_sent
 * Description  : Processing the application data and sending it to the host
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_sent(struct espconn *pespconn)
{
    uint8 devkey[TOKEN_SIZE] = {0};
	os_memcpy(devkey,esp_param.devkey,sizeof(esp_param.devkey));
    uint32 nonce;
    //char *pbuf = (char *)os_zalloc(packet_size);
	char pbuf[512];

    ESP_DBG("%s, aflag = %u\n", __func__, esp_param.activeflag);
    os_memset(pbuf, 0, sizeof(pbuf));

    if (esp_param.activeflag != 0x1) {
        esp_param.activeflag = 0;
    }
	ESP_DBG("\r\n\r\n==============================\r\n");
	ESP_DBG("%s :\r\n",__func__);
	ESP_DBG("MESH DEBUG ACTIVE: %d \r\n",esp_param.activeflag);
	ESP_DBG("---------------------------\r\n\r\n");

    if (pbuf != NULL) {
        if (esp_param.activeflag == 0) {
            uint8 token[TOKEN_SIZE] = {0};
            uint8 bssid[6];
            active_nonce = os_random()&0x7fffffff;//rand();// os_random();


			os_memset(token, 0, sizeof(token));

            wifi_get_macaddr(STATION_IF, bssid);
            
            if ((esp_param.token[0] >= '0' && esp_param.token[0] <= '9') || 
                (esp_param.token[0] >= 'a' && esp_param.token[0] <= 'z') ||
                (esp_param.token[0] >= 'A' && esp_param.token[0] <= 'Z')) {
                os_memcpy(token, esp_param.token, 40);
            } else {
                uint8_t i, j;
                for (i = 0, j = 0; i < 38; i = i + 2, j ++) {
                    if (j == 6)
                        j = 0;
                    os_sprintf(esp_param.token + i, "%02X", bssid[j]);
                }
                esp_param.token[i ++] = 'F';
                esp_param.token[i ++] = 'F';
                os_memcpy(token, esp_param.token, 40);
            }

            os_sprintf(pbuf, ACTIVE_FRAME, active_nonce, token, MAC2STR(bssid),iot_version, devkey);
			ESP_DBG("PBUF1 MAX:512; LEN: %d \r\n",os_strlen(pbuf));
			//os_sprintf(pbuf, ACTIVE_FRAME, active_nonce, token, MAC2STR(bssid), iot_version, g_devkey);
        }

#if SENSOR_DEVICE
#if HUMITURE_SUB_DEVICE
        else {
#if 0
            uint16 tp, rh;
            uint8 data[4];

            if (user_mvh3004_read_th(data)) {
                rh = data[0] << 8 | data[1];
                tp = data[2] << 8 | data[3];
            }

#else
            uint16 tp, rh;
            uint8 *data;
            uint32 tp_t, rh_t;
            data = (uint8 *)user_mvh3004_get_poweron_th();

            rh = data[0] << 8 | data[1];
            tp = data[2] << 8 | data[3];
#endif
            tp_t = (tp >> 2) * 165 * 100 / (16384 - 1);
            rh_t = (rh & 0x3fff) * 100 * 100 / (16384 - 1);

            if (tp_t >= 4000) {
                os_sprintf(pbuf, UPLOAD_FRAME, count, "", tp_t / 100 - 40, tp_t % 100, rh_t / 100, rh_t % 100, devkey);
            } else {
                tp_t = 4000 - tp_t;
                os_sprintf(pbuf, UPLOAD_FRAME, count, "-", tp_t / 100, tp_t % 100, rh_t / 100, rh_t % 100, devkey);
            }
        }

#elif FLAMMABLE_GAS_SUB_DEVICE
        else {
            uint32 adc_value = system_adc_read();

            os_sprintf(pbuf, UPLOAD_FRAME, count, adc_value / 1024, adc_value * 1000 / 1024, devkey);
        }

#endif
#else
        else {
            nonce = os_random()&0x7fffffff;
            os_sprintf(pbuf, FIRST_FRAME, nonce , devkey);
        }

#endif
        //ESP_DBG("%s\n", pbuf);
        //ESP_DBG("%s\n", pbuf);
        //ESP_DBG("\r\n\r\n==============================\r\n");
        //g_sent_identify = true;
        //os_printf("activate: %s\n", pbuf);

#if ESP_MESH_SUPPORT
		if(g_mac){
			mesh_json_add_elem(pbuf, sizeof(pbuf), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
		}else{
			g_mac = (char*)mesh_GetMdevMac();
			mesh_json_add_elem(pbuf, sizeof(pbuf), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
		}
		
#endif
        espconn_esp_sent(pespconn, pbuf, os_strlen(pbuf));

        //os_free(pbuf);
    }
    //os_timer_arm(&beacon_timer, BEACON_TIME, 0);//
}

#if PLUG_DEVICE || LIGHT_DEVICE
/******************************************************************************
 * FunctionName : user_esp_platform_sent_beacon
 * Description  : sent beacon frame for connection with the host is activate
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_sent_beacon(struct espconn *pespconn)
{
    struct ip_info ipconfig;
    if (pespconn == NULL) {
        return;
    }

    wifi_get_ip_info(STATION_IF, &ipconfig);
    ESP_DBG("===============\r\n");
	ESP_DBG("ESPCONN STATE: %d \r\n",pespconn->state);
	ESP_DBG("===============\r\n");
    if(pespconn->state == ESPCONN_CONNECT) { 

        if (esp_param.activeflag == 0) {
            ESP_DBG("please check device is activated.\n");
            user_esp_platform_sent(pespconn);
        } else {
            uint8 devkey[TOKEN_SIZE] = {0};
            os_memcpy(devkey, esp_param.devkey, 40);

            ESP_DBG("%s %u\n", __func__, system_get_time());

            if (ping_status == 0) {
                ESP_DBG("user_esp_platform_sent_beacon sent fail!\n");
				#if ESP_MESH_SUPPORT==0
                    user_esp_platform_discon(pespconn);
				#else
				    //user_esp_platform_check_ip(0);

				#if 0
    				if (!espconn_mesh_local_addr(&ipconfig.ip)) {
                        os_timer_disarm(&beacon_timer);
                        ping_status = 1;
                        user_esp_platform_discon(pespconn);
                    }else{
    					user_esp_platform_check_ip(0);
                    }
				#else
    				os_printf("send beacon in timer now...\r\n");
				    ping_status = 1;
    				os_timer_arm(&beacon_timer, 0, 0);
				#endif

				                
				#endif
            } else {
                //char *pbuf = (char *)os_zalloc(packet_size);
                char pbuf[256];
                os_memset(pbuf, 0, sizeof(pbuf));

                //if (pbuf != NULL) {
                    os_sprintf(pbuf, BEACON_FRAME, devkey);
                    #if ESP_MESH_SUPPORT
					if(g_mac){
						mesh_json_add_elem(pbuf, sizeof(pbuf), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
					}else{
						g_mac = (char*)mesh_GetMdevMac();
						mesh_json_add_elem(pbuf, sizeof(pbuf), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
					}
					
					uint8* info_mesh = NULL;
					uint8 g_prnt_mac[33];
					uint8 cnt=0,i=0;
					if (espconn_mesh_get_node_info(MESH_NODE_PARENT, &info_mesh, &cnt)) {
						os_printf("get parent info success\n");
						if (cnt == 0) {
							os_printf("no parent\n");
						} else {
						    // children represents the count of children.
						    // you can read the child-information from child_info.
						    for(i=0;i<cnt;i++){
						        //os_printf("ptr[%d]: %p \r\n",i,info_mesh+i*6);
						        os_printf("MAC[%d]:"MACSTR"\r\n",i,MAC2STR(info_mesh+i*6));
					        }
						    os_memset(g_prnt_mac,0,sizeof(g_prnt_mac));
						    if(mesh_root_if()){
						        os_sprintf(g_prnt_mac,"\"parent_mdev_mac\":\"%02X%02X%02X%02X%02X%02X\"",MAC2STR(info_mesh));
                            }else{
							    //os_sprintf(g_prnt_mac,"\"parent_mdev_mac\":\"%02X%02X%02X%02X%02X%02X\"",0x18,MAC2STR5BYTES(info_mesh));
							    os_sprintf(g_prnt_mac,"\"parent_mdev_mac\":\"%02X%02X%02X%02X%02X%02X\"",MAC2STR(info_mesh));
						   	}
							os_printf("g_prnt_mac: %s \r\n",g_prnt_mac);
						}
					} else {
						os_printf("get parent info fail\n");
						info_mesh = NULL;
						cnt = 0;
					} 

					if(info_mesh!=NULL && cnt>0){
						mesh_json_add_elem(pbuf, sizeof(pbuf), g_prnt_mac, ESP_MESH_JSON_DEV_PRNT_MAC_ELEM_LEN);
					}else{
						mesh_json_add_elem(pbuf, sizeof(pbuf), "000000000000", ESP_MESH_JSON_DEV_PRNT_MAC_ELEM_LEN);
					}

					if (!espconn_mesh_local_addr(&ipconfig.ip)){
                        ping_status = 0;
                    }
                    #endif
                    espconn_esp_sent(pespconn, pbuf, os_strlen(pbuf));
					ESP_DBG("PBUF2 MAX:256; LEN: %d \r\n",os_strlen(pbuf));

                    os_timer_disarm(&beacon_timer);
                    os_timer_arm(&beacon_timer, BEACON_TIME, 0);
                    //os_free(pbuf);
                //}
            }
        }
    } else {
        ESP_DBG("user_esp_platform_sent_beacon sent fail! TCP NOT CONNECT:%d\n",pespconn->state == ESPCONN_CONNECT);
		#if ESP_MESH_SUPPORT==0
        user_esp_platform_discon(pespconn);
		#endif
    }
}

/******************************************************************************
 * FunctionName : user_platform_rpc_set_rsp
 * Description  : response the message to server to show setting info is received
 * Parameters   : pespconn -- the espconn used to connetion with the host
 *                nonce -- mark the message received from server
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_platform_rpc_set_rsp(struct espconn *pespconn, int nonce, int status)
{
    //char *pbuf = (char *)os_zalloc(packet_size);
    char pbuf[256];

    os_memset(pbuf, 0, sizeof(pbuf));
    if (pespconn == NULL) {
        return;
    }

    os_sprintf(pbuf, RPC_RESPONSE_FRAME, status, nonce);
    #if ESP_MESH_SUPPORT
    if (g_sip)
        mesh_json_add_elem(pbuf, sizeof(pbuf), g_sip, ESP_MESH_JSON_IP_ELEM_LEN);
    if (g_sport)
        mesh_json_add_elem(pbuf, sizeof(pbuf), g_sport, ESP_MESH_JSON_PORT_ELEM_LEN);
	if(g_mac){
		mesh_json_add_elem(pbuf, sizeof(pbuf), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
	}else{
		g_mac = (char*)mesh_GetMdevMac();
		mesh_json_add_elem(pbuf, sizeof(pbuf), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
	}
		
	#endif
    espconn_esp_sent(pespconn, pbuf, os_strlen(pbuf));
	ESP_DBG("PBUF3 MAX:256; LEN: %d \r\n",os_strlen(pbuf));
    //os_free(pbuf);
}

LOCAL bool ICACHE_FLASH_ATTR
user_platform_rpc_build_rsp( char* pdata,int data_max_size,int nonce, int status)
{
    char pbuf[256];
    os_memset(pbuf, 0, sizeof(pbuf));
    os_sprintf(pbuf, RPC_RESPONSE_ELEMENT, status, nonce);
	
	if(!mesh_json_add_elem(pdata,data_max_size, pbuf, os_strlen(pbuf))){
		os_printf("user_platform_rpc_build_rsp error...\r\n");
		return false;
	}

	while(1){
		char* ptmp = os_strchr(pdata,'\n');
		if(ptmp){
		    *ptmp = ' ';
		}else{
			break;
		}

	}
	int idx = os_strlen(pdata);
	if(pdata[idx-1]!='\n'){
		pdata[idx]='\n';
	}
	os_printf("debug:\r\nbuild rsp:%s\r\n",pdata);
	return true;
}


LOCAL void ICACHE_FLASH_ATTR
user_platform_rpc_battery_rsp(struct espconn *pespconn, char* data_buf,int data_buf_len, int nonce, int status)
{
    if (pespconn == NULL) {
        return;
    }

    #if ESP_MESH_SUPPORT
	if(g_mac){
		ESP_DBG("G_MAC: %s\r\n",g_mac);
		mesh_json_add_elem(data_buf, data_buf_len, g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
	}else{
		ESP_DBG("G_MAC: %s\r\n",g_mac);
		g_mac = (char*)mesh_GetMdevMac();
		mesh_json_add_elem(data_buf, data_buf_len, g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
	}

	if(! user_platform_rpc_build_rsp( data_buf,data_buf_len,nonce, status)){
		return;
	}
	#endif
    espconn_esp_sent(pespconn, data_buf, os_strlen(data_buf));
	ESP_DBG("PBUF4 MAX:256; LEN: %d \r\n",os_strlen(data_buf));
    //os_free(pbuf);
}

/******************************************************************************
 * FunctionName : user_platform_timer_get
 * Description  : get the timers from server
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_platform_timer_get(struct espconn *pespconn)
{
    uint8 devkey[TOKEN_SIZE] = {0};
	os_memcpy(devkey, esp_param.devkey, 40);
    //char *pbuf = (char *)os_zalloc(packet_size);
    char pbuf[512];

    os_memset(pbuf, 0, sizeof(pbuf));
    if (pespconn == NULL) {
        return;
    }

    os_sprintf(pbuf, TIMER_FRAME, devkey);
#if ESP_MESH_SUPPORT
	if(g_mac){
		ESP_DBG("G_MAC: %s\r\n",g_mac);
		mesh_json_add_elem(pbuf, sizeof(pbuf), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
	}else{
		ESP_DBG("G_MAC: %s\r\n",g_mac);
		g_mac = (char*)mesh_GetMdevMac();
		mesh_json_add_elem(pbuf, sizeof(pbuf), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
	}
#endif

	
	ESP_DBG("========================\r\n");
	ESP_DBG("%s \r\n",__func__);
	ESP_DBG("PBUF5 MAX:512; LEN: %d \r\n",os_strlen(pbuf));
	ESP_DBG("------------------\r\n");
    ESP_DBG("%s\n", pbuf);
	ESP_DBG("========================\r\n");
	
    espconn_esp_sent(pespconn, pbuf, os_strlen(pbuf));
    //os_free(pbuf);
}

/******************************************************************************
 * FunctionName : user_esp_platform_upgrade_cb
 * Description  : Processing the downloaded data from the server
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_upgrade_rsp(void *arg)
{
    struct upgrade_server_info *server = arg;
    struct espconn *pespconn = server->pespconn;
    uint8 devkey[41] = {0};
	os_memcpy(devkey, esp_param.devkey, 40);
	char *action = NULL;
    
	uint8 pbuf[512];
    os_memset(pbuf, 0, sizeof(pbuf));

    if (server->upgrade_flag == true) {
        ESP_DBG("user_esp_platform_upgarde_successfully\n");
        action = "device_upgrade_success";
        os_sprintf(pbuf, UPGRADE_FRAME, devkey, action, server->pre_version, server->upgrade_version);
        ESP_DBG("%s\n",pbuf);

        espconn_esp_sent(pespconn, pbuf, os_strlen(pbuf));

    } else {
        ESP_DBG("user_esp_platform_upgrade_failed\n");
        action = "device_upgrade_failed";
        os_sprintf(pbuf, UPGRADE_FRAME, devkey, action,server->pre_version, server->upgrade_version);
        ESP_DBG("%s\n",pbuf);

        espconn_esp_sent(pespconn, pbuf, os_strlen(pbuf));

    }

    if (server->url)
	    os_free(server->url);
    server->url = NULL;
    os_free(server);
    server = NULL;
}

/******************************************************************************
 * FunctionName : user_esp_platform_upgrade_begin
 * Description  : Processing the received data from the server
 * Parameters   : pespconn -- the espconn used to connetion with the host
 *                server -- upgrade param
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_upgrade_begin(struct espconn *pespconn, struct upgrade_server_info *server)
{
    uint8 devkey[41] = {0};
	os_memcpy(devkey, esp_param.devkey, 40);
	uint8 user_bin[10] = {0};
    const char *usr1 = "user1.bin";
    const char *usr2 = "user2.bin";

    server->pespconn = pespconn;

    os_memcpy(server->ip, pespconn->proto.tcp->remote_ip, 4);

#ifdef UPGRADE_SSL_ENABLE
    server->port = 8443;
#else
	server->port = 8000;
#endif

    server->check_cb = user_esp_platform_upgrade_rsp;
    server->check_times = 120000;

    if (server->url == NULL) {
		server->url = (uint8 *)os_zalloc(1024);
    }
	if (server->url == NULL) {
		return;
	}

    os_memset(user_bin, 0, sizeof(user_bin));
    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1) {
		os_memcpy(user_bin, usr2, os_strlen(usr2));
    } else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2) {
		os_memcpy(user_bin, usr1, os_strlen(usr1));
    }

	os_sprintf(server->url, "GET /v1/device/rom/?action=download_rom&version=%s&filename=%s HTTP/1.0\r\nContent-Length: 23\r\nHost: "IPSTR":%d\r\n" pheadbuffer "\n",
               server->upgrade_version, user_bin, IP2STR(server->ip),
               server->port, devkey);
    ESP_DBG("%s\n",server->url);

#ifdef UPGRADE_SSL_ENABLE

    if (system_upgrade_start_ssl(server) == false) {
#else

    if (system_upgrade_start(server) == false) {
#endif
        ESP_DBG("upgrade is already started\n");
    }
}
#endif

#if ESP_MESH_SUPPORT
struct espconn *g_mesh_esp = NULL;
os_timer_t g_mesh_upgrade_timer;

void ICACHE_FLASH_ATTR
    mesh_upgrade_check_func(void *arg)
{
    uint8_t pbuf[512] = {0};
	struct espconn *pespconn = arg;
    struct upgrade_server_info *server = NULL;
	os_timer_disarm(&g_mesh_upgrade_timer);
	system_upgrade_deinit();
    if (!pespconn)
        pespconn = g_mesh_esp;
    os_memset(pbuf, 0, sizeof(pbuf));
	if( system_upgrade_flag_check() == UPGRADE_FLAG_FINISH ) {
        os_sprintf(pbuf, UPGRADE_FRAME, g_devkey, "device_upgrade_success", iot_version, g_mesh_version);
		#if LIGHT_DEVICE&&ESP_DEBUG_MODE
    		//light_shadeStart(HINT_WHITE,1000,4,1,NULL);
			light_set_aim(0,22222,0,10000,10000,1000,0);
		#endif
	} else {
        os_sprintf(pbuf, UPGRADE_FRAME, g_devkey, "device_upgrade_failed", iot_version, g_mesh_version);
		system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
        #if LIGHT_DEVICE&&ESP_DEBUG_MODE
    		//light_shadeStart(HINT_RED,1000,4,1,NULL);
		    light_set_aim(22222,0,0,0,0,1000,0);
        #endif
	}
    if (g_sip)
        mesh_json_add_elem(pbuf, sizeof(pbuf), g_sip, ESP_MESH_JSON_IP_ELEM_LEN);
    if (g_sport)
        mesh_json_add_elem(pbuf, sizeof(pbuf), g_sport, ESP_MESH_JSON_PORT_ELEM_LEN);
	
	if(g_mac){
	    mesh_json_add_elem(pbuf, sizeof(pbuf), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
	}else{
		g_mac = (char*)mesh_GetMdevMac();
		mesh_json_add_elem(pbuf, sizeof(pbuf), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
	}
	
    espconn_esp_sent(pespconn, pbuf, os_strlen(pbuf));
    if (g_mesh_esp)
        os_free(g_mesh_esp);
    g_mesh_esp = NULL;
    os_memset(g_mesh_version, 0, sizeof(g_mesh_version));
}

LOCAL bool ICACHE_FLASH_ATTR
user_mesh_upgrade_build_pkt(uint8_t *pkt_buf, int nonce, char *version, uint32_t offset, uint16_t sect_len)
{
    char *bin_file = NULL;
	if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1) {
        bin_file = "user2.bin";
	} else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2) {
		bin_file = "user1.bin";
	}
    if (!bin_file)
        return false;
    os_sprintf(pkt_buf, MESH_UPGRADE, nonce, version, bin_file, offset, sect_len, g_devkey);
    return true;
}

LOCAL void ICACHE_FLASH_ATTR
user_mesh_upgrade_continue(struct espconn *esp, char *pkt, size_t pkt_len,
                           uint32_t offset, uint16_t sect_len)
{
    char *pstr = NULL;
    char buffer[512] = {0};
    char *version = "\"version\":";
    int nonce = user_JsonGetValueInt(pkt, pkt_len, "\"nonce\":");
    os_memset(buffer, 0, sizeof(buffer));
    if (g_mesh_version[0] == 0) {
        version = user_json_find_section(pkt, pkt_len, version);
        if (version)
            os_memcpy(g_mesh_version, version, IOT_BIN_VERSION_LEN);
    }
    if (!user_mesh_upgrade_build_pkt(buffer, nonce, g_mesh_version, offset, sect_len)) {
        user_platform_rpc_set_rsp(esp, nonce, 400);
        return;
    }
    if (g_sip)
        mesh_json_add_elem(buffer, sizeof(buffer), g_sip, ESP_MESH_JSON_IP_ELEM_LEN);
    if (g_sport)
        mesh_json_add_elem(buffer, sizeof(buffer), g_sport, ESP_MESH_JSON_PORT_ELEM_LEN);
	if (g_mac){
	    mesh_json_add_elem(buffer, sizeof(buffer), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
	}else{
		g_mac = (char*)mesh_GetMdevMac();
		mesh_json_add_elem(buffer, sizeof(buffer), g_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN);
	}

    espconn_esp_sent(esp, buffer, os_strlen(buffer));
}


LOCAL void ICACHE_FLASH_ATTR
user_esp_mesh_upgrade(struct espconn *esp, char *pkt, size_t pkt_len, char *base64_bin)
{
    uint8_t *bin_file = NULL;
    uint16_t sect_len, base64_len, bin_len;
    uint32_t total_len = 0, offset = 0;	
	int idx;
    sect_len = user_JsonGetValueInt(pkt, pkt_len, "\"size\":");
    base64_len = user_JsonGetValueInt(pkt, pkt_len, "\"size_base64\":");
    offset = user_JsonGetValueInt(pkt, pkt_len, "\"offset\":");
    total_len = user_JsonGetValueInt(pkt, pkt_len, "\"total\":");
	if(offset==0 && total_len>0){
		ESP_DBG("ERASE FLASH FOR UPGRADE \r\n");
		ESP_DBG("TOTAL LEN: %d \r\n",total_len);
		system_upgrade_erase_flash(total_len);
	}
    if (offset + sect_len <= total_len) {

		
        char c = base64_bin[base64_len];
        base64_bin[base64_len] = '\0';
        bin_len = base64Decode(base64_bin, base64_len + 1, &bin_file);
		os_printf("-----------\r\n");
		os_printf("test bin: len: %d \r\n",bin_len);
		//for(idx=0;idx<bin_len;idx++) os_printf("%02x ",bin_file[idx]);
		//os_printf("-----------\r\n");
		if(UPGRADE_FLAG_START != system_upgrade_flag_check()){
			os_printf("-------------------\r\n");
			os_printf("UPGRADE ALREADY STOPPED...ERROR OR TIMEOUT...status:%d\r\n",system_upgrade_flag_check());
			
			os_printf("-------------------\r\n");
			return;
		}
        system_upgrade(bin_file, bin_len);
        if (bin_file)
            os_free(bin_file);
        base64_bin[base64_len] = c;
        if (offset + sect_len < total_len) {
			os_printf("%d / %d \r\n",offset + sect_len,total_len);
            user_mesh_upgrade_continue(esp, pkt, pkt_len, offset + sect_len, MESH_UPGRADE_SEC_SIZE);
            return;
        }
        system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
		ota_finished_time = system_get_time();
		esp_param.ota_finish_time = ota_finished_time;
		system_param_save_with_protect(ESP_PARAM_START_SEC, &esp_param, sizeof(esp_param));

		os_printf("=================\r\n");
		os_printf("upgrade finish...SET FLAG : %d\r\n",system_upgrade_flag_check());
		os_printf("=================\r\n");
        os_timer_disarm(&g_mesh_upgrade_timer);
        mesh_upgrade_check_func(esp);
    } else {  // upgrade fail
        user_platform_rpc_set_rsp(esp, user_JsonGetValueInt(pkt, pkt_len, "\"nonce\":"), 400);
        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
        os_timer_disarm(&g_mesh_upgrade_timer);
		
        mesh_upgrade_check_func(esp);
    }
}

LOCAL void
user_esp_mesh_upgrade_begin(struct espconn *esp, int nonce, uint8_t *pkt, uint16_t pkt_len)
{
    if (!g_mesh_esp)
        g_mesh_esp = (struct espconn *)os_zalloc(sizeof(struct espconn));
    os_memcpy(g_mesh_esp, esp, sizeof(*esp));
    g_mesh_esp->reverse = (void *)nonce;
    //system_upgrade_init(RUN_BIN);
	system_upgrade_init();
    system_upgrade_flag_set(UPGRADE_FLAG_START);
    user_platform_rpc_set_rsp(esp, nonce, 200);
    os_memset(g_mesh_version, 0, sizeof(g_mesh_version));
    user_mesh_upgrade_continue(esp, pkt, pkt_len, 0, MESH_UPGRADE_SEC_SIZE);//1000
    os_timer_disarm(&g_mesh_upgrade_timer);
    os_timer_setfn(&g_mesh_upgrade_timer, (os_timer_func_t *)mesh_upgrade_check_func, g_mesh_esp);
    os_timer_arm(&g_mesh_upgrade_timer, 3600000, 0); //20 min
    #if ESP_DEBUG_MODE
        light_set_aim(0,22222,22222,5000,5000,1000,0);
	#endif
}

#endif


LOCAL void ICACHE_FLASH_ATTR
user_esp_upgrade_begin(struct espconn *pespconn, int nonce, char *version)
{
    struct upgrade_server_info *server = NULL;
    user_platform_rpc_set_rsp(pespconn, nonce, 200);
    server = (struct upgrade_server_info *)os_zalloc(sizeof(struct upgrade_server_info));
    os_memcpy(server->upgrade_version, version, IOT_BIN_VERSION_LEN);
    server->upgrade_version[IOT_BIN_VERSION_LEN] = '\0';
    os_sprintf(server->pre_version,"%s%d.%d.%dt%d(%s)", VERSION_TYPE,IOT_VERSION_MAJOR,
            IOT_VERSION_MINOR,IOT_VERSION_REVISION, device_type,UPGRADE_FALG);
    user_esp_platform_upgrade_begin(pespconn, server);
}
/*
csc add for simple pair
*/
LOCAL int ICACHE_FLASH_ATTR
button_status_set(struct jsontree_context *js_ctx, struct jsonparse_state *parser)
{
#if 1
    os_printf("---button set----\n");
    
    int type;
    
    uint8 mdev_mac_address[13]={0};
    uint32 replace=0;
    
    uint8 tempkey[33]={0};
    uint8 button_address[13]={0};
    uint8 mac_len=0;
    uint8 mac[13]={0};
    mac[12]=0;
    mdev_mac_address[12]=0;
    tempkey[32]=0;
    button_address [12]=0; 
    while ((type = jsonparse_next(parser)) != 0) {
        //os_printf("Type=%c\n",type);
        if (type == JSON_TYPE_PAIR_NAME) {
            if(jsonparse_strcmp_value(parser, "mdev_mac") == 0){
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value(parser, mdev_mac_address, sizeof(mdev_mac_address));
                os_printf("mdev_mac_address: %s \n",mdev_mac_address);
            } 
			else if (jsonparse_strcmp_value(parser, "temp_key") == 0) {
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value(parser, tempkey, sizeof(tempkey));
                //os_memcpy(buttonPairingInfo.tempkey_p,tempkey,sizeof(buttonPairingInfo.tempkey_p));
                int j;
                char tmp[17];
                char* ptmp = tempkey;
                for(j=0;j<16;j++){
                    os_bzero(tmp,sizeof(tmp));
                    os_memcpy(tmp,ptmp,2);
                    uint8 val = strtol(tmp,NULL,16);
                    os_printf("val[%d]: %02x \r\n",j,val);
                    buttonPairingInfo.tempkey[j] = val;
                    ptmp+=2;
                }
                os_printf("tempkey: %s\n",tempkey,js_ctx->depth);
                // buttonPairingInfo.tempkey= strtol(tempkey,NULL,16);
                //debug_print("button_tempkey:",buttonPairingInfo.tempkey,sizeof(buttonPairingInfo.tempkey));
                //user_light_set_duty(status, LIGHT_RED);
                //light_set_aim_r( r);
            } 
			else if (jsonparse_strcmp_value(parser, "button_mac") == 0) {
                #if 1
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value(parser, button_address, sizeof(button_address));
                os_printf("button_mac: %s \n",button_address);
                //os_memcpy(buttonPairingInfo.button_mac_p,button_address,sizeof(button_address));
                int j;
                char tmp[4];
                char* ptmp = button_address;
                for(j=0;j<6;j++){
                    os_bzero(tmp,sizeof(tmp));
                    os_memcpy(tmp,ptmp,2);
                    uint8 val = strtol(tmp,NULL,16);
                    os_printf("val[%d]: %02x \r\n",j,val);
                    buttonPairingInfo.button_mac[j] = val;
                    ptmp+=2;
                }
                //long long data= strtol(button_address,NULL,16);
                //os_printf("data=%x\n",data);
                // debug_print("button_mac change value:",buttonPairingInfo.button_mac,sizeof(buttonPairingInfo.button_mac));
                #endif
                //user_light_set_duty(status, LIGHT_GREEN);
                //light_set_aim_g( g);
            }
			else if  (jsonparse_strcmp_value(parser, "replace") == 0) {
                jsonparse_next(parser);
                jsonparse_next(parser);
                replace = jsonparse_get_value_as_int(parser);
                os_printf("replace=%d\n",replace);
            }
            else if (jsonparse_strcmp_value(parser, "mac_len") == 0) {
                jsonparse_next(parser);
                jsonparse_next(parser);
                
                mac_len = jsonparse_get_value_as_int(parser);
                os_printf("mac_len=%d\n",mac_len);
            
            }else if(jsonparse_strcmp_value(parser, "test") == 0){
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value(parser, mac, sizeof(mac));
                os_printf("test: %s \n",mac);
                //user_light_set_duty(status, LIGHT_GREEN);
            }
        
        }
    }
	os_printf("Type=%d\n",type);
    return 1;
#endif
}
LOCAL int ICACHE_FLASH_ATTR
button_status_get(struct jsontree_context *js_ctx)
{
   os_printf("button get\n");
   return 1;
}

LOCAL struct jsontree_callback button_callback =
    JSONTREE_CALLBACK(button_status_get, button_status_set);

JSONTREE_OBJECT(button_remove_tree,
              JSONTREE_PAIR("mac_len", &button_callback),
              JSONTREE_PAIR("test", &button_callback),
                 );

JSONTREE_OBJECT(button_new_tree,
              JSONTREE_PAIR("temp_key", &button_callback),
              JSONTREE_PAIR("button_mac", &button_callback),
                 );

JSONTREE_OBJECT(buttonPairingInfom,
                JSONTREE_PAIR("mdev_mac", &button_callback),
                JSONTREE_PAIR("button_new", &button_new_tree),
                 JSONTREE_PAIR("replace", &button_callback),
                 JSONTREE_PAIR("button_remove", &button_remove_tree),
                //JSONTREE_PAIR("switches", &switch_tree)
                );
JSONTREE_OBJECT(button_info_tree,
	           JSONTREE_PAIR("button_info", &buttonPairingInfom));


          	/*
		  csc add simple pair end
		*/





/******************************************************************************
 * FunctionName : user_esp_platform_recv_cb
 * Description  : Processing the received data from the server
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
#if ESP_MESH_SUPPORT
	mesh_StopReconnCheck();
#endif
    char *pstr = NULL;
	#if ESP_MESH_SUPPORT
	//char pbuffer[1024 * 2] = {0};//It is supposed that in mesh solution, we will only receive a entire packet
	char* pbuffer = pusrdata;
	#else
    LOCAL char pbuffer[1024 * 2] = {0};
	#endif
    struct espconn *pespconn = arg;
	


    if (length >= 1460) { //BUG : RCV 1640
        #if ESP_MESH_SUPPORT
		    ESP_DBG("--------------------\r\n");
		    ESP_DBG("GO TO MESH PARSE\r\n");
		    ESP_DBG("--------------------\r\n");
        	goto PARSE;
        #endif
        os_memcpy(pbuffer, pusrdata, length);
    } else {

    
	PARSE:
		#if !ESP_MESH_SUPPORT
        os_memcpy(pbuffer + os_strlen(pbuffer), pusrdata, length);
		#endif

		
	    if(NULL != (char*)os_strstr(pbuffer,"Content-Length")){
			webserver_recv(pespconn, pbuffer, length);
			return;
		}else{
			#if (PLUG_DEVICE || LIGHT_DEVICE)
			    //os_timer_disarm(&beacon_timer);
            #endif
		}
		/*
		  csc add simple pair
		*/
		
		extern char* sip;
		extern char* sport;
         if(os_strstr(pbuffer,PAIR_START_PATH))
		{
			os_printf("~~~~~~~~~~\r\n");
			os_printf("start pairing mode \r\n");
			os_printf("~~~~~~~~~~\r\n");

#if 1//data from iot platfom,not mesh ?
			sip = (char *)os_strstr(pusrdata, ESP_MESH_SIP_STRING);
			sport = (char *)os_strstr(pusrdata, ESP_MESH_SPORT_STRING);

			if(sip) {
				os_bzero(pair_sip,sizeof(pair_sip));
				os_memcpy(pair_sip,sip,ESP_MESH_JSON_IP_ELEM_LEN);
				os_printf("pair_sip,%d: %s \r\n",os_strlen(pair_sip),pair_sip);
			}
			if(sport){
				os_bzero(pair_sport,sizeof(pair_sport));
				os_memcpy(pair_sport,sport,ESP_MESH_JSON_PORT_ELEM_LEN);
				os_printf("pair_sport,%d: %s \r\n",os_strlen(pair_sport),pair_sport);
			}
#endif
			os_printf("Recv Button inform\n");
			struct jsontree_context js;
			jsontree_setup(&js, (struct jsontree_value *)&button_info_tree, json_putchar);
			json_parse(&js, pbuffer);
			//response_send(ptrespconn, true);
			char data_resp[100];
			os_bzero(data_resp,sizeof(data_resp));
			os_sprintf(data_resp, "{\"status\":200,\"path\":\"/device/button/configure\"}");
			os_printf("response send str...\r\n");
			
			//response_send_str(void *arg, bool responseOK,char* pJsonDataStr , int str_len,char* url,uint16 url_len,uint8 tag_if,uint8 header_if)
			response_send_str(pespconn, true, data_resp,os_strlen(data_resp),PAIR_START_PATH,os_strlen(PAIR_START_PATH),1,0);
			buttonPairingInfo.simple_pair_state=USER_PBULIC_BUTTON_INFO;
			sp_LightPairState();
			return;


		}
		else if(os_strstr(pbuffer,PAIR_FOUND_REQUEST))
		{
			os_printf("PAIR_FOUND_REQUEST \r\n");
			if((char*)os_strstr(pbuffer,"200")!=NULL){
				os_printf("device can pair\n");
				buttonPairingInfo.simple_pair_state=USER_PERMIT_SIMPLE_PAIR;
			}
			else if((char*)os_strstr(pbuffer,"403")){
				os_printf("device cannot pair\n");
				buttonPairingInfo.simple_pair_state=USER_REFUSE_SIMPLE_PAIR;
			}
			//pairStatus=SP_LIGHT_PAIR_REQ_RES;
			sp_LightPairState();
			return;
		}
		else if(os_strstr(pbuffer,PAIR_RESULT))
		{
			if((char*)os_strstr(pbuffer,"200")!=NULL){
				os_printf("device pair stop\n");
				buttonPairingInfo.simple_pair_state=USER_CONFIG_STOP_SIMPLE_PAIR;
			}
			else if((char*)os_strstr(pbuffer,"100")){
				os_printf("device pair continue\n");
				buttonPairingInfo.simple_pair_state=USER_CONFIG_CONTIUE_SIMPLE_PAIR;
			}
			//response_send(pespconn, true);
			return;
		}				
		else if(os_strstr(pbuffer,PAIR_KEEP_ALIVE)){
			os_printf("PAIR KEEP ALIVE: \r\n");
			sip = (char *)os_strstr(pusrdata, ESP_MESH_SIP_STRING);
			sport = (char *)os_strstr(pusrdata, ESP_MESH_SPORT_STRING);
			sp_LightPairReplyKeepAlive();
			return;
		}
		#if 0
		else if(os_strstr(pbuffer,PAIR_BUTTONS_MAC))
		{
			uint8 SaveStrBuffer[MAX_BUTTON_NUM*6*2+40];
			ResonseUserInquireButtonMacBody(SaveStrBuffer);
			os_printf("ResonseUserInquireButtonMacBody:\n%s",SaveStrBuffer);
			data_send(pespconn,true,SaveStrBuffer);
		}
		#endif

          	/*
		  csc add simple pair end
		*/
		#if ESP_DEBUG_MODE
			if(NULL == (char*)os_strstr(pbuffer,"download_rom_base64")){
        		ESP_DBG("========================\r\n");
        		ESP_DBG("user_esp_platform_recv_cb \r\n");
        		ESP_DBG("RCV LEN: %d \r\n",length);
        		ESP_DBG("------------------\r\n");
        		ESP_DBG("%s \r\n",pbuffer);
        		ESP_DBG("========================\r\n");
			}else{
        		ESP_DBG("========================\r\n");
        		ESP_DBG("platform recv: %d \r\n",length);
			}
		#endif

		



        struct espconn *pespconn = (struct espconn *)arg;
		#if ESP_MESH_SUPPORT
		g_sip = (char *)os_strstr(pusrdata, ESP_MESH_SIP_STRING);
		g_sport = (char *)os_strstr(pusrdata, ESP_MESH_SPORT_STRING);
		////g_mac = (char *)os_strstr(pusrdata, ESP_MESH_DEV_MAC_STRING);
		
		//if(g_mac==NULL) g_mac = (char*)mesh_GetMdevMac();
		#endif
		//ESP_DBG("================================\r\n");
		//ESP_DBG("ACTIVATE INFO: \r\n");
		//ESP_DBG("TEST NOUNCE: %u	;	%u	\r\n",user_esp_platform_parse_nonce(pbuffer,length),active_nonce);
		//ESP_DBG(" TEST PARSE: %s \r\n",(char *)os_strstr(pbuffer, "\"activate_status\": "));
		//ESP_DBG("================================\r\n");
        if ((pstr = user_json_find_section(pbuffer, length, "\"activate_status\":")) != NULL &&
        //if ((pstr = user_json_find_section(pbuffer, length, "\"status\": ")) != NULL &&
            user_esp_platform_parse_nonce(pbuffer, length) == active_nonce) {
            //os_printf("get nounce: %d ; ori nonce  : %d \r\n",user_esp_platform_parse_nonce(pbuffer, length),active_nonce);
			
            if (os_strncmp(pstr, "1", 1) == 0) {
			//if (os_strncmp(pstr, "200", 3) == 0) {
				ESP_DBG("\r\n\r\n===========================\r\n");
                ESP_DBG("device activates successful.\n");
				ESP_DBG("===========================\r\n\r\n");
				
                device_status = DEVICE_ACTIVE_DONE;
                esp_param.activeflag = 1;
                system_param_save_with_protect(ESP_PARAM_START_SEC, &esp_param, sizeof(esp_param));
                user_esp_platform_sent(pespconn);
            } else {
            
				ESP_DBG("\r\n\r\n===========================\r\n");
                ESP_DBG("device activates failed.\n");
				ESP_DBG("\r\n\r\n===========================\r\n");
				device_status = DEVICE_ACTIVE_FAIL;
            }
        }
	

#if (PLUG_DEVICE || LIGHT_DEVICE)
        else if ((pstr = user_json_find_section(pbuffer, length, "\"action\"")) != NULL) {
            int nonce = user_esp_platform_parse_nonce(pbuffer, length);
            if (!os_memcmp(pstr, "sys_upgrade", os_strlen("sys_upgrade"))) {  // upgrade start
            	if(UPGRADE_FLAG_START == system_upgrade_flag_check()){
					os_printf("Under Upgrading...return...\r\n");
                    user_platform_rpc_set_rsp(pespconn, nonce, 401);
					return;
            	}
                os_printf("upgrade start....!!!\r\n");
				ota_start_time = system_get_time();
				esp_param.ota_start_time = ota_start_time;
				esp_param.ota_finish_time = ota_start_time;
				system_param_save_with_protect(ESP_PARAM_START_SEC, &esp_param, sizeof(esp_param));

				
                if ((pstr = user_json_find_section(pbuffer, length, "\"version\"")) != NULL) {
					#if ESP_MESH_SUPPORT
                    if (espconn_mesh_get_status() == MESH_DISABLE) {  // normal upgrade start
                        user_esp_upgrade_begin(pespconn, nonce, pstr);
                    } else {  // mesh upgrade start
                        user_esp_mesh_upgrade_begin(pespconn, nonce, pbuffer, length);
                    }
					#else
                        user_esp_upgrade_begin(pespconn, nonce, pstr);
					#endif
                } else {
                    user_platform_rpc_set_rsp(pespconn, nonce, 400);
                }
            } else if (!os_memcmp(pstr, "download_rom_base64", os_strlen("download_rom_base64"))) {
                if ((pstr = user_json_find_section(pbuffer, length, "\"device_rom\"")) != NULL &&
                    (pstr = user_json_find_section(pstr, length, "\"rom_base64\"")) != NULL) {
                    #if ESP_MESH_SUPPORT
                    user_esp_mesh_upgrade(pespconn, pbuffer, length, pstr);
					#endif
                } else {
                    user_platform_rpc_set_rsp(pespconn, nonce, 400);
				}
            } else if (!os_memcmp(pstr, "sys_reboot", os_strlen("sys_reboot"))) {
                if(system_upgrade_flag_check() == UPGRADE_FLAG_FINISH){
					user_platform_rpc_set_rsp(pespconn, nonce, 200);
                    ESP_DBG("upgrade reboot..");
    				UART_WaitTxFifoEmpty(0,50000);
                    //espconn_mesh_setup_timer(&client_timer, 100,(os_timer_func_t *)system_upgrade_reboot, NULL, 0);
    				os_timer_disarm(&client_timer);
    				os_timer_setfn(&client_timer, (os_timer_func_t *)system_upgrade_reboot, NULL);
    				os_timer_arm(&client_timer, 1000, 0);
                }else{
                    user_platform_rpc_set_rsp(pespconn, nonce, 400);
					ESP_DBG("upgrade not finished..");
                }
    				
                
            } else if(!os_memcmp(pstr, "get_switches", os_strlen("get_switches"))){
                ESP_DBG("GET BUTTON STATUS CMD:\r\n");
				//json_send(pespconn, BATTERY_STATUS);
				char pbuf[512];
				os_memset(pbuf,0,sizeof(pbuf));
				json_build_packet(pbuf, BATTERY_STATUS);
				//user_platform_rpc_build_rsp( pbuf,sizeof(pbuf),nonce,200);
				user_platform_rpc_battery_rsp(pespconn,pbuf,sizeof(pbuf), nonce, 200);
				os_printf("debug cmd switches:\r\n %s \r\n",pbuf);
				
            
			} else if(!os_memcmp(pstr, "test", os_strlen("test"))){
                ESP_DBG("GET BUTTON STATUS CMD:\r\n");
				//json_send(pespconn, BATTERY_STATUS);
				char pbuf[512];
				os_memset(pbuf,0,sizeof(pbuf));
				//os_sprintf(pbuf,"{\"switches\":[{\"mac\":\"00:00:00:00:00:00\",\"status\":\"NA\",\"voltagemv\":0}]}");
				//os_sprintf(pbuf,"{\"switches\":{\"mac\":\"00:00:00:00:00:00\",\"status\":\"NA\",\"voltagemv\":0}}");
				//os_sprintf(pbuf,"{\"deliver_to_device\": true,\"router\":\"FFFFFFFF\"}\n");
				json_build_packet(pbuf, BATTERY_STATUS);
				//user_platform_rpc_build_rsp( pbuf,sizeof(pbuf),nonce,200);
				user_platform_rpc_battery_rsp(pespconn,pbuf,sizeof(pbuf), nonce, 200);
				os_printf("debug cmd test:\r\n %s \r\n",pbuf);
			}
			else if(!os_memcmp(pstr, "multicast", os_strlen("multicast"))){
			    if((pstr = user_json_find_section(pbuffer, length, "\"method\"")) != NULL){
					os_printf("test find section: %s\r\n",pstr);
					
                    if (!os_memcmp(pstr, "GET", os_strlen("GET"))) {
        				os_printf("FIND GET...\r\n");
                        user_esp_platform_get_info(pespconn, pbuffer);
                    } else if (!os_memcmp(pstr, "POST", os_strlen("POST"))) {
                        os_printf("FIND POST...\r\n");
						//os_printf("------------\r\n");
						//os_printf("pbuffer: %s\r\n",pbuffer);
						//os_printf("------------\r\n");
                        user_esp_platform_set_info(pespconn, pbuffer,CMD_TYPE_MULITCAST);
                    }
			    }
			}else {
                user_platform_rpc_set_rsp(pespconn, nonce, 400);
            }
        } else if ((pstr = (char *)os_strstr(pbuffer, "/v1/device/timers/")) != NULL) {
            int nonce = user_esp_platform_parse_nonce(pbuffer, length);
            user_platform_rpc_set_rsp(pespconn, nonce, 200);
            //espconn_mesh_setup_timer(&client_timer, 2000,(os_timer_func_t *)user_platform_timer_get, pespconn, 0);
            os_timer_disarm(&client_timer);
            os_timer_setfn(&client_timer, (os_timer_func_t *)user_platform_timer_get, pespconn);
            os_timer_arm(&client_timer, 2000, 0);
        } else if ((pstr = (char *)os_strstr(pbuffer, "\"method\": ")) != NULL) {
            if (os_strncmp(pstr + 11, "GET", 3) == 0) {
				os_printf("FIND GET...\r\n");
                user_esp_platform_get_info(pespconn, pbuffer);
            } else if (os_strncmp(pstr + 11, "POST", 4) == 0) {
                os_printf("FIND POST...\r\n");
                user_esp_platform_set_info(pespconn, pbuffer,CMD_TYPE_UNICAST);
            }
        } else if ((pstr = (char *)os_strstr(pbuffer, "ping success")) != NULL) {
            ESP_DBG("ping success\n");
            ping_status = 1;
        } else if ((pstr = (char *)os_strstr(pbuffer, "send message success")) != NULL) {
        } else if ((pstr = (char *)os_strstr(pbuffer, "timers")) != NULL) {
            user_platform_timer_start(pusrdata , pespconn);
        }

#elif SENSOR_DEVICE
        else if ((pstr = (char *)os_strstr(pbuffer, "\"status\":")) != NULL) {
            if (os_strncmp(pstr + 10, "200", 3) != 0) {
                ESP_DBG("message upload failed.\n");
            } else {
                count++;
                ESP_DBG("message upload sucessful.\n");
            }

            os_timer_disarm(&client_timer);
            os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_discon, pespconn);
            os_timer_arm(&client_timer, 10, 0);
        }

#endif
        else if ((pstr = (char *)os_strstr(pbuffer, "device")) != NULL) {
#if PLUG_DEVICE || LIGHT_DEVICE
            user_platform_timer_get(pespconn);
#elif SENSOR_DEVICE
#endif
        }

        os_memset(pbuffer, 0, sizeof(pbuffer));
    }

#if (PLUG_DEVICE || LIGHT_DEVICE)
    //os_timer_disarm(&beacon_timer);
    //os_timer_arm(&beacon_timer, BEACON_TIME, 0);
#endif

//ESP_DBG("END OF RCV CB\r\n");
}

#if AP_CACHE
/******************************************************************************
 * FunctionName : user_esp_platform_ap_change
 * Description  : add the user interface for changing to next ap ID.
 * Parameters   :
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_ap_change(void)
{
    uint8 current_id;
    uint8 i = 0;
    ESP_DBG("user_esp_platform_ap_is_changing\n");

    current_id = wifi_station_get_current_ap_id();
    ESP_DBG("current ap id =%d\n", current_id);

    if (current_id == AP_CACHE_NUMBER - 1) {
       i = 0;
    } else {
       i = current_id + 1;
    }
    while (wifi_station_ap_change(i) != true) {
       i++;
       if (i == AP_CACHE_NUMBER - 1) {
    	   i = 0;
       }
    }

    /* just need to re-check ip while change AP */
    device_recon_count = 0;
    os_timer_disarm(&client_timer);
    os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
    os_timer_arm(&client_timer, 100, 0);
}
#endif

LOCAL bool ICACHE_FLASH_ATTR
user_esp_platform_reset_mode(void)
{
	ESP_DBG("user_esp_platform_reset_mode\r\n");
    if (wifi_get_opmode() != STATIONAP_MODE) {
        wifi_set_opmode(STATIONAP_MODE);
    }

#if AP_CACHE
    /* delay 5s to change AP */
    os_timer_disarm(&client_timer);
    os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_ap_change, NULL);
    os_timer_arm(&client_timer, 5000, 0);

    return true;
#endif

    return false;
}

/******************************************************************************
 * FunctionName : user_esp_platform_recon_cb
 * Description  : The connection had an error and is already deallocated.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_recon_cb(void *arg, sint8 err)
{
    struct espconn *pespconn = (struct espconn *)arg;

    ESP_DBG("user_esp_platform_recon_cb\n");

#if (PLUG_DEVICE || LIGHT_DEVICE)
    os_timer_disarm(&beacon_timer);
#endif

#if (PLUG_DEVICE || SENSOR_DEVICE)
    user_link_led_output(1);
#endif

    if (++device_recon_count == 5) {
        device_status = DEVICE_CONNECT_SERVER_FAIL;
		#if ESP_MESH_SUPPORT==0
        if (user_esp_platform_reset_mode()) {
            return;
        }
		#endif
    }

#if SENSOR_DEVICE
#ifdef SENSOR_DEEP_SLEEP

    if (wifi_get_opmode() == STATION_MODE) {
        user_esp_platform_reset_mode();
        //user_sensor_deep_sleep_enter();
    } else {
        os_timer_disarm(&client_timer);
        os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_reconnect, pespconn);
        os_timer_arm(&client_timer, 1000, 0);
    }

#else
    os_timer_disarm(&client_timer);
    os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_reconnect, pespconn);
    os_timer_arm(&client_timer, 1000, 0);
#endif
#else
    os_timer_disarm(&client_timer);
    os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_reconnect, pespconn);
    os_timer_arm(&client_timer, 1000, 0);
#endif
}

/******************************************************************************
 * FunctionName : user_esp_platform_connect_cb
 * Description  : A new incoming connection has been connected.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_connect_cb(void *arg)
{
    struct espconn *pespconn = arg;
	ping_status = 1;

    ESP_DBG("user_esp_platform_connect_cb\n");
	os_printf("=======================\r\n");
	os_printf("test in connect cb\r\n");
	
	os_printf("==============\r\n");
	os_printf("CONN STATUS : %d , %p\r\n",pespconn->state , pespconn);

	os_printf("=======================\r\n");
    if (wifi_get_opmode() ==  STATIONAP_MODE ) {
        //wifi_set_opmode(STATION_MODE);
    }
	#if ESP_DEBUG_MODE
	debug_UploadExceptionInfo(pespconn);
	#endif

#if (PLUG_DEVICE || SENSOR_DEVICE)
    user_link_led_timer_done();
#endif
    device_recon_count = 0;
    espconn_regist_recvcb(pespconn, user_esp_platform_recv_cb);
    espconn_regist_sentcb(pespconn, user_esp_platform_sent_cb);
    user_esp_platform_sent(pespconn);
}


void ICACHE_FLASH_ATTR
	user_esp_platform_sent_data()
{
	user_esp_platform_sent(&user_conn);

}
/******************************************************************************
 * FunctionName : user_esp_platform_connect
 * Description  : The function given as the connect with the host
 * Parameters   : espconn -- the espconn used to connect the connection
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_connect(struct espconn *pespconn)
{
    ESP_DBG("user_esp_platform_connect\n");

#if ESP_MESH_SUPPORT
    #if ESP_MESH_STRIP
	os_printf("==============\r\n");
	os_printf("CONN STATUS : %d , %p\r\n",pespconn->state , pespconn);
	os_printf("ip: "IPSTR"\r\n",IP2STR(pespconn->proto.tcp->remote_ip));
	
    espconn_mesh_connect(pespconn);
	#else
	espconn_connect(pespconn);
	#endif

#else
#ifdef CLIENT_SSL_ENABLE
    espconn_secure_connect(pespconn);
#else
    espconn_connect(pespconn);
#endif
#endif
}

#ifdef USE_DNS
/******************************************************************************
 * FunctionName : user_esp_platform_dns_found
 * Description  : dns found callback
 * Parameters   : name -- pointer to the name that was looked up.
 *                ipaddr -- pointer to an ip_addr_t containing the IP address of
 *                the hostname, or NULL if the name could not be found (or on any
 *                other error).
 *                callback_arg -- a user-specified callback argument passed to
 *                dns_gethostbyname
 * Returns      : none
*******************************************************************************/
os_timer_t mesh_retry_t;
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;
	static int mesh_reinit_flg = 0;

	ESP_DBG("IN DNS FOUND:\r\n esp_server_ip.addr: %08x \r\n",esp_server_ip.addr);
	//ESP_DBG("IPADDR: %08x \r\n",ipaddr->addr);

    if (ipaddr == NULL) {
        ESP_DBG("user_esp_platform_dns_found NULL,RECON CNT:%d \r\n",device_recon_count);
		
        if (++device_recon_count == 5) {
            device_status = DEVICE_CONNECT_SERVER_FAIL;
			
			#if ESP_MESH_SUPPORT==0
            user_esp_platform_reset_mode();
			#else
			//os_timer_disarm(&client_timer);
			//os_timer_arm(&client_timer,10000,0);
			#endif
        }

        return;
    }

    ESP_DBG("user_esp_platform_dns_found %d.%d.%d.%d\n",
            *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
            *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));

    if (esp_server_ip.addr == 0 && ipaddr->addr != 0) {
        os_timer_disarm(&client_timer);
		//#if ESP_MESH_SUPPORT
		//    mesh_StartReconnCheck(10000);
		//#endif
		#if ESP_MESH_SUPPORT
		    ESP_DBG("MESH STATUS : %d \r\n",espconn_mesh_get_status());
			//if(MESH_LOCAL_AVAIL == espconn_mesh_get_status()){
			if(MESH_ONLINE_AVAIL != espconn_mesh_get_status()){
				ESP_DBG("--------------\r\n");
				ESP_DBG("TRY TO ENABLE MESH ONLINE MODE\r\n");
				ESP_DBG("--------------\r\n");
		        //user_MeshInit();//reset to online mode
		        mesh_StartReconnCheck(10);
				//mesh_ReconCheck();
		        return;
			}else{
				mesh_StartReconnCheck(10000);
			}
		#endif
        esp_server_ip.addr = ipaddr->addr;
        os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);

        pespconn->proto.tcp->local_port = espconn_port();

#ifdef CLIENT_SSL_ENABLE
        pespconn->proto.tcp->remote_port = 8443;
#else
        pespconn->proto.tcp->remote_port = 8000;
#endif

#if (PLUG_DEVICE || LIGHT_DEVICE)
        ping_status = 1;
#endif

        espconn_regist_connectcb(pespconn, user_esp_platform_connect_cb);
        espconn_regist_disconcb(pespconn, user_esp_platform_discon_cb);
        espconn_regist_reconcb(pespconn, user_esp_platform_recon_cb);
        user_esp_platform_connect(pespconn);
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_dns_check_cb
 * Description  : 1s time callback to check dns found
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_dns_check_cb(void *arg)
{

#if ESP_MESH_SUPPORT
    struct ip_info ipconfig;
    struct espconn *pespconn = arg;
    wifi_get_ip_info(STATION_IF, &ipconfig);
    os_timer_disarm(&client_timer);
    ESP_DBG("user_esp_platform_dns_check_cb, ip:" IPSTR "server:" IPSTR "\n", IP2STR(&ipconfig.ip.addr), IP2STR(&esp_server_ip.addr));
	ESP_DBG("IPADDR_ANY : %08x ; %d\r\n",IPADDR_ANY,(ipconfig.ip.addr == IPADDR_ANY));
	
    //espconn_gethostbyname(pespconn, ESP_DOMAIN, &esp_server_ip, user_esp_platform_dns_found);
    if (ipconfig.ip.addr == IPADDR_ANY || !espconn_mesh_local_addr(&ipconfig.ip)) {
		ESP_DBG("TEST IN NORMAL BRANCH\r\n");
        if (esp_server_ip.addr == IPADDR_ANY) {
			ESP_DBG("SERVER IP ZERO\r\n");
            espconn_gethostbyname(pespconn, ESP_DOMAIN, &esp_server_ip, user_esp_platform_dns_found);
            os_timer_disarm(&client_timer);
			
			//#if ESP_MESH_SUPPORT
			if(MESH_DISABLE == espconn_mesh_get_status()){
				os_printf("MESH DISABLE , RETURN...\r\n");
				return;
			}
			//#endif
			
			if(device_recon_count<5){
                os_timer_arm(&client_timer, 1000, 0);
			}else{
				os_timer_arm(&client_timer, 10000, 0);
			}
        } else {
            user_esp_platform_dns_found(ESP_DOMAIN, &esp_server_ip, pespconn);
        }
    } else { 
    	ESP_DBG("TEST IN IP GW\r\n");
        user_esp_platform_dns_found(ESP_DOMAIN, &ipconfig.gw, pespconn);
    }
#else
    struct espconn *pespconn = arg;
    ESP_DBG("user_esp_platform_dns_check_cb\n");
    espconn_gethostbyname(pespconn, ESP_DOMAIN, &esp_server_ip, user_esp_platform_dns_found);
    os_timer_arm(&client_timer, 1000, 0);

#endif
}

LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_start_dns(struct espconn *pespconn)
{
    struct ip_info ipconfig;
	esp_server_ip.addr = 0;
    ESP_DBG("user_esp_platform_start_dns\n");
    wifi_get_ip_info(STATION_IF, &ipconfig);
	#if ESP_MESH_SUPPORT
    if (esp_server_ip.addr == 0 && !espconn_mesh_local_addr(&ipconfig.ip)) {
	    esp_server_ip.addr = 0;
		espconn_gethostbyname(pespconn, ESP_DOMAIN, &esp_server_ip, user_esp_platform_dns_found);
	}
	#else
	espconn_gethostbyname(pespconn, ESP_DOMAIN, &esp_server_ip, user_esp_platform_dns_found);
	#endif

    os_timer_disarm(&client_timer);
    os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_dns_check_cb, pespconn);
	if(device_recon_count<5){
	    os_timer_arm(&client_timer, 1000, 0);
	}else{
		os_timer_arm(&client_timer, 10000, 0);

	}
}
#endif


#if ESP_MDNS_SUPPORT

#if LIGHT_DEVICE
struct mdns_info *mdns_info=NULL;
void ICACHE_FLASH_ATTR
    user_mdns_conf()
{
    os_printf("%s\r\n",__func__);
	wifi_set_broadcast_if(STATIONAP_MODE);
	espconn_mdns_close();
    struct ip_info ipconfig;
    wifi_get_ip_info(STATION_IF, &ipconfig);
	#if 1
    	if(mdns_info == NULL){
            mdns_info = (struct mdns_info *)os_zalloc(sizeof(struct mdns_info));
    	}else{
    		os_memset(mdns_info,0,sizeof(struct mdns_info));
    	}
		mdns_info->host_name = "ESP_LIGHT";
        mdns_info->ipAddr= ipconfig.ip.addr; //sation ip
    	//os_printf("mDNS ip: "IPSTR,IP2STR(info->ipAddr));
        mdns_info->server_name = "ESP_MESH";
        mdns_info->server_port = 8000;
        mdns_info->txt_data[0] = "version = 1.2.0";
    	mdns_info->txt_data[1] = "vendor = espressif";
    	mdns_info->txt_data[2] = "mesh_support = 1";
    	mdns_info->txt_data[3] = "PWM channel = 5";
        espconn_mdns_init(mdns_info);
	#else
    	static struct mdns_info mdns_info_t;
    	mdns_info_t.host_name = "ESP_LIGHT";
        mdns_info_t.ipAddr= ipconfig.ip.addr; //sation ip
    	//os_printf("mDNS ip: "IPSTR,IP2STR(info->ipAddr));
        mdns_info_t.server_name = "ESP_MESH";
        mdns_info_t.server_port = 8000;
        mdns_info_t.txt_data[0] = "version = 1.2.0";
    	mdns_info_t.txt_data[1] = "vendor = espressif";
    	mdns_info_t.txt_data[2] = "mesh_support = 1";
    	mdns_info_t.txt_data[3] = "PWM channel = 5";
        espconn_mdns_init(&mdns_info_t);
	#endif

	espconn_mdns_server_register();
	
}
#endif

#endif

void ICACHE_FLASH_ATTR
	mesh_enable_cb_t()
{
	ESP_DBG("--------------\r\n");
	ESP_DBG("MESH ENABLE CB\r\n");
	ESP_DBG("-------------\r\n");
	user_esp_platform_connect_ap_cb();
}
/******************************************************************************
 * FunctionName : user_esp_platform_check_ip
 * Description  : espconn struct parame init when get ip addr
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_check_ip(uint8 reset_flag)
{
    //os_printf("%s\r\n",__func__);
    struct ip_info ipconfig;
    uint8_t wifi_status = STATION_IDLE;//
    os_timer_disarm(&client_timer);

    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
#if (PLUG_DEVICE || SENSOR_DEVICE)
        user_link_led_timer_init();
#endif

//***************************
#if LIGHT_DEVICE
   #if ESP_MDNS_SUPPORT
		user_mdns_conf();
	#endif
#endif
#if ESP_MESH_SUPPORT
    ESP_DBG("STA STATUS: %d ;  ip: %d \r\n",wifi_station_get_connect_status(),ipconfig.ip.addr);
	if(MESH_DISABLE == espconn_mesh_get_status() ){
		ESP_DBG("======================\r\n");
		ESP_DBG("CONNECTED TO ROUTER, ENABLE MESH\r\n");
		ESP_DBG("======================\r\n");
		espconn_mesh_enable(mesh_enable_cb_t,MESH_ONLINE);
		//user_esp_platform_connect_ap_cb();
	}else{
		ESP_DBG("--------------------\r\n");
		ESP_DBG("CONNECT AP CB \r\n");
		ESP_DBG("--------------------\r\n");
		user_esp_platform_connect_ap_cb();
	}
	
//***************************

#else
        user_conn.proto.tcp = &user_tcp;
        user_conn.type = ESPCONN_TCP;
        user_conn.state = ESPCONN_NONE;

        device_status = DEVICE_CONNECTING;

        if (reset_flag) {
            device_recon_count = 0;
        }

#if (PLUG_DEVICE || LIGHT_DEVICE)
        os_timer_disarm(&beacon_timer);
        os_timer_setfn(&beacon_timer, (os_timer_func_t *)user_esp_platform_sent_beacon, &user_conn);
#endif

#ifdef USE_DNS
        user_esp_platform_start_dns(&user_conn);
#else
        const char esp_server_ip[4] = {114, 215, 177, 97};

        os_memcpy(user_conn.proto.tcp->remote_ip, esp_server_ip, 4);
        user_conn.proto.tcp->local_port = espconn_port();

#ifdef CLIENT_SSL_ENABLE
        user_conn.proto.tcp->remote_port = 8443;
#else
        user_conn.proto.tcp->remote_port = 8000;
#endif
        espconn_regist_connectcb(&user_conn, user_esp_platform_connect_cb);
        espconn_regist_reconcb(&user_conn, user_esp_platform_recon_cb);
        user_esp_platform_connect(&user_conn);
#endif

#endif

    } else {
        /* if there are wrong while connecting to some AP, then reset mode */
        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
                wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL)) {
            #if ESP_MESH_SUPPORT==0
            user_esp_platform_reset_mode();
			#endif
            os_timer_arm(&client_timer, 100, 0);
        } else {
            os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
            os_timer_arm(&client_timer, 100, 0);
        }
    }

}

/******************************************************************************
 * FunctionName : user_esp_platform_init
 * Description  : device parame init based on espressif platform
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_init(void)
{
	debug_FlashBufInit();
    os_memset(iot_version, 0, sizeof(iot_version));
	os_sprintf(iot_version,"%s%d.%d.%dt%d(%s)",VERSION_TYPE,IOT_VERSION_MAJOR,\
	IOT_VERSION_MINOR,IOT_VERSION_REVISION,device_type,UPGRADE_FALG);
	os_printf("IOT VERSION = %s\n",iot_version);

	system_param_load(ESP_PARAM_START_SEC, 0, &esp_param, sizeof(esp_param));
    os_memset(g_devkey, 0, sizeof(g_devkey));
	os_memcpy(g_devkey, esp_param.devkey, sizeof(esp_param.devkey));
	struct rst_info *rtc_info = system_get_rst_info();

	os_printf("reset reason: %x\n", rtc_info->reason);

	if (rtc_info->reason == REASON_WDT_RST ||
		rtc_info->reason == REASON_EXCEPTION_RST ||
		rtc_info->reason == REASON_SOFT_WDT_RST) {
		if (rtc_info->reason == REASON_EXCEPTION_RST) {
			os_printf("Fatal exception (%d):\n", rtc_info->exccause);
		}
		
		debug_FlashSvExceptInfo(rtc_info);
		os_printf("epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\n",
				rtc_info->epc1, rtc_info->epc2, rtc_info->epc3, rtc_info->excvaddr, rtc_info->depc);
	}

	/***add by tzx for saving ip_info to avoid dhcp_client start****/
    struct dhcp_client_info dhcp_info;
    struct ip_info sta_info;
    system_rtc_mem_read(64,&dhcp_info,sizeof(struct dhcp_client_info));
	if(dhcp_info.flag == 0x01 ) {
		if (true == wifi_station_dhcpc_status())
		{
			wifi_station_dhcpc_stop();
		}
		sta_info.ip = dhcp_info.ip_addr;
		sta_info.gw = dhcp_info.gw;
		sta_info.netmask = dhcp_info.netmask;
		if ( true != wifi_set_ip_info(STATION_IF,&sta_info)) {
			os_printf("set default ip wrong\n");
		}
	}
    os_memset(&dhcp_info,0,sizeof(struct dhcp_client_info));
    system_rtc_mem_write(64,&dhcp_info,sizeof(struct rst_info));


#if AP_CACHE
    wifi_station_ap_number_set(AP_CACHE_NUMBER);
#endif

#if 0
    {
        char sofap_mac[6] = {0x16, 0x34, 0x56, 0x78, 0x90, 0xab};
        char sta_mac[6] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xab};
        struct ip_info info;

        wifi_set_macaddr(SOFTAP_IF, sofap_mac);
        wifi_set_macaddr(STATION_IF, sta_mac);

        IP4_ADDR(&info.ip, 192, 168, 3, 200);
        IP4_ADDR(&info.gw, 192, 168, 3, 1);
        IP4_ADDR(&info.netmask, 255, 255, 255, 0);
        wifi_set_ip_info(STATION_IF, &info);

        IP4_ADDR(&info.ip, 10, 10, 10, 1);
        IP4_ADDR(&info.gw, 10, 10, 10, 1);
        IP4_ADDR(&info.netmask, 255, 255, 255, 0);
        wifi_set_ip_info(SOFTAP_IF, &info);
    }
#endif

    if (esp_param.activeflag != 1) {
#ifdef SOFTAP_ENCRYPT
        struct softap_config config;
        char password[33];
        char macaddr[6];

        wifi_softap_get_config(&config);
        wifi_get_macaddr(SOFTAP_IF, macaddr);

        os_memset(config.password, 0, sizeof(config.password));
        os_sprintf(password, MACSTR "_%s", MAC2STR(macaddr), PASSWORD);
        os_memcpy(config.password, password, os_strlen(password));
        config.authmode = AUTH_WPA_WPA2_PSK;

        wifi_softap_set_config(&config);
#endif
		ESP_DBG("user_esp_platform_init\r\n");
        wifi_set_opmode(STATIONAP_MODE);
    }

#if PLUG_DEVICE
    user_plug_init();
#elif LIGHT_DEVICE
    user_light_init();
#elif SENSOR_DEVICE
    user_sensor_init(esp_param.activeflag);
#endif

    if (wifi_get_opmode() != SOFTAP_MODE) {
        os_timer_disarm(&client_timer);
        os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_check_ip, 1);
        os_timer_arm(&client_timer, 100, 0);
    }
}


void ICACHE_FLASH_ATTR
	user_esp_platform_set_reset_flg(uint32 rst)
{
	
	esp_param.reset_flg = rst&0xff;
	//system_param_load(ESP_PARAM_START_SEC, 0, &esp_param, sizeof(esp_param));
	system_param_save_with_protect(ESP_PARAM_START_SEC,&esp_param,sizeof(esp_param));
}


void ICACHE_FLASH_ATTR
	user_esp_platform_reset_default()
{
#if ESP_MESH_SUPPORT
	espconn_mesh_disable(NULL);
#endif
	esp_param.reset_flg=0;
	esp_param.activeflag = 0;
	ESP_DBG("--------------------\r\n");
	ESP_DBG("SYSTEM PARAM RESET !\r\n");
	ESP_DBG("RESET: %d ;  ACTIVE: %d \r\n",esp_param.reset_flg,esp_param.activeflag);
	ESP_DBG("--------------------\r\n\n\n");
	UART_WaitTxFifoEmpty(0,3000);
	os_memset(esp_param.token,0,40);
	system_param_save_with_protect(ESP_PARAM_START_SEC, &esp_param, sizeof(esp_param));
	system_restore();
	system_restart();


}


void ICACHE_FLASH_ATTR
user_esp_platform_init_peripheral(void)
{
	debug_FlashBufInit();
	espSendQueueInit();
	#if ESP_MESH_SUPPORT
	    g_mac = (char*)mesh_GetMdevMac();
	#endif
	
    os_memset(iot_version, 0, sizeof(iot_version));
	os_sprintf(iot_version,"%s%d.%d.%dt%d(%s)",VERSION_TYPE,IOT_VERSION_MAJOR,\
	IOT_VERSION_MINOR,IOT_VERSION_REVISION,device_type,UPGRADE_FALG);
	os_printf("IOT VERSION = %s\n",iot_version);

	system_param_load(ESP_PARAM_START_SEC, 0, &esp_param, sizeof(esp_param));
    os_memset(g_devkey, 0, sizeof(g_devkey));
	os_memcpy(g_devkey, esp_param.devkey, sizeof(esp_param.devkey));

	struct rst_info *rtc_info = system_get_rst_info();

	#if ESP_DEBUG_MODE
	debug_SvExceptionInfo(rtc_info);
	#endif
	
	os_printf("reset reason: %x\n", rtc_info->reason);

	if (rtc_info->reason == REASON_WDT_RST ||
		rtc_info->reason == REASON_EXCEPTION_RST ||
		rtc_info->reason == REASON_SOFT_WDT_RST) {
		if (rtc_info->reason == REASON_EXCEPTION_RST) {
			os_printf("Fatal exception (%d):\n", rtc_info->exccause);
		}
		#if ESP_DEBUG_MODE
		debug_FlashSvExceptInfo(rtc_info);
		#endif
		os_printf("epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\n",
				rtc_info->epc1, rtc_info->epc2, rtc_info->epc3, rtc_info->excvaddr, rtc_info->depc);
	}

	/***add by tzx for saving ip_info to avoid dhcp_client start****/
    struct dhcp_client_info dhcp_info;
    struct ip_info sta_info;
    system_rtc_mem_read(64,&dhcp_info,sizeof(struct dhcp_client_info));
	if(dhcp_info.flag == 0x01 ) {
		if (true == wifi_station_dhcpc_status())
		{
			wifi_station_dhcpc_stop();
		}
		sta_info.ip = dhcp_info.ip_addr;
		sta_info.gw = dhcp_info.gw;
		sta_info.netmask = dhcp_info.netmask;
		if ( true != wifi_set_ip_info(STATION_IF,&sta_info)) {
			os_printf("set default ip wrong\n");
		}
	}
    os_memset(&dhcp_info,0,sizeof(struct dhcp_client_info));
    system_rtc_mem_write(64,&dhcp_info,sizeof(struct rst_info));


#if AP_CACHE
    wifi_station_ap_number_set(AP_CACHE_NUMBER);
#endif

#if 0
    {
        char sofap_mac[6] = {0x16, 0x34, 0x56, 0x78, 0x90, 0xab};
        char sta_mac[6] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xab};
        struct ip_info info;

        wifi_set_macaddr(SOFTAP_IF, sofap_mac);
        wifi_set_macaddr(STATION_IF, sta_mac);

        IP4_ADDR(&info.ip, 192, 168, 3, 200);
        IP4_ADDR(&info.gw, 192, 168, 3, 1);
        IP4_ADDR(&info.netmask, 255, 255, 255, 0);
        wifi_set_ip_info(STATION_IF, &info);

        IP4_ADDR(&info.ip, 10, 10, 10, 1);
        IP4_ADDR(&info.gw, 10, 10, 10, 1);
        IP4_ADDR(&info.netmask, 255, 255, 255, 0);
        wifi_set_ip_info(SOFTAP_IF, &info);
    }
#endif

os_printf("------------------\r\n");
os_printf("test flag: %d \r\n");
os_printf("-----------------\r\n");
    if (esp_param.activeflag != 1) {
#ifdef SOFTAP_ENCRYPT
        struct softap_config config;
        char password[33];
        char macaddr[6];
        wifi_softap_get_config(&config);
        wifi_get_macaddr(SOFTAP_IF, macaddr);
        os_memset(config.password, 0, sizeof(config.password));
        os_sprintf(password, MACSTR "_%s", MAC2STR(macaddr), PASSWORD);
        os_memcpy(config.password, password, os_strlen(password));
        config.authmode = AUTH_WPA_WPA2_PSK;
        wifi_softap_set_config(&config);
#endif
        ESP_DBG("user_esp_platform_init_peripheral\r\n");
        wifi_set_opmode(STATIONAP_MODE);
    }

#if PLUG_DEVICE
    user_plug_init();
#elif LIGHT_DEVICE
    user_light_init();
#elif SENSOR_DEVICE
    user_sensor_init(esp_param.activeflag);
#endif

}

void ICACHE_FLASH_ATTR
	user_esp_platform_check_connect()
{
    if (wifi_get_opmode() != SOFTAP_MODE) {
        os_timer_disarm(&client_timer);
        os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_check_ip, 0);
        os_timer_arm(&client_timer, 100, 1);
    }
}

void ICACHE_FLASH_ATTR
user_esp_platform_connect_ap_cb()
{
    struct ip_info ipconfig;
	#if ESP_TOUCH_SUPPORT
	esptouch_setAckFlag(true);
	#endif
    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
		
		os_timer_disarm(&client_timer);
		
		os_printf("PLATFORM DEBUG: CONNECTED TO AP,RUN PLATFORM CODES.\r\n");
		#if (PLUG_DEVICE || SENSOR_DEVICE)
        //user_link_led_timer_init();
		#endif

//***************************
#if LIGHT_DEVICE
#if ESP_MDNS_SUPPORT
	#if ESP_MESH_SUPPORT
    if(!espconn_mesh_local_addr(&ipconfig.ip)){
	    user_mdns_conf();
    }else{
        os_printf("mDNS close...\r\n");
		espconn_mdns_close();
    }
	#else
	user_mdns_conf();
	#endif
#endif

#endif
//***************************
        user_conn.proto.tcp = &user_tcp;
        user_conn.type = ESPCONN_TCP;
        user_conn.state = ESPCONN_NONE;

        device_status = DEVICE_CONNECTING;

        device_recon_count = 0;

#if (PLUG_DEVICE || LIGHT_DEVICE)
        os_timer_disarm(&beacon_timer);
        os_timer_setfn(&beacon_timer, (os_timer_func_t *)user_esp_platform_sent_beacon, &user_conn);
#endif

#ifdef USE_DNS
        user_esp_platform_start_dns(&user_conn);
#else
        const char esp_server_ip[4] = {114, 215, 177, 97};

        os_memcpy(user_conn.proto.tcp->remote_ip, esp_server_ip, 4);
        user_conn.proto.tcp->local_port = espconn_port();

#ifdef CLIENT_SSL_ENABLE
        user_conn.proto.tcp->remote_port = 8443;
#else
        user_conn.proto.tcp->remote_port = 8000;
#endif

        espconn_regist_connectcb(&user_conn, user_esp_platform_connect_cb);
        espconn_regist_reconcb(&user_conn, user_esp_platform_recon_cb);
        user_esp_platform_connect(&user_conn);
#endif
    }
}


const char* _line_sep = "===============\r\n";
void ICACHE_FLASH_ATTR _LINE_DESP()
{
	 ets_printf("%s",_line_sep);
}



#if ESP_DEBUG_MODE
void* ICACHE_FLASH_ATTR user_GetUserPConn()
{
	return (&user_conn);
}


#endif





#endif
