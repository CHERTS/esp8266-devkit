
#include "ets_sys.h"
#include "osapi.h"

#include "user_interface.h"
#include "user_webserver.h"
#include "user_config.h"
#include "user_simplepair.h"
#include "user_light_hint.h"
#include "mem.h"


void ICACHE_FLASH_ATTR sp_PrintBuf(uint8* data,uint8 len);
PairedButtonParam PairedDev;


void ICACHE_FLASH_ATTR sp_PairedDevRmAll(PairedButtonParam* buttonParam)
{
    int i;
	for(i=0;i<buttonParam->PairedNum;i++){
		esp_now_del_peer(buttonParam->PairedList[i].mac_t);
	}
}

void ICACHE_FLASH_ATTR sp_PairedDevLoadAll(PairedButtonParam* buttonParam)
{
    int i;
	for(i=0;i<buttonParam->PairedNum;i++){
		esp_now_add_peer(buttonParam->PairedList[i].mac_t,ESP_NOW_ROLE_CONTROLLER,0,buttonParam->PairedList[i].key_t,ESPNOW_KEY_LEN);
	}

}

void ICACHE_FLASH_ATTR sp_PairedParamSave(PairedButtonParam* buttonParam)
{
	buttonParam->magic = SP_PARAM_MAGIC;
	config_ParamCsumSet(buttonParam,&(buttonParam->csum),sizeof(PairedButtonParam));
	config_ParamSaveWithProtect(LIGHT_PAIRED_DEV_PARAM_ADDR,(uint8*)buttonParam ,sizeof(PairedButtonParam));
	os_printf("debug: check again: \r\n");
	
	config_ParamCsumCheck(buttonParam,sizeof(PairedButtonParam));
}



void ICACHE_FLASH_ATTR sp_PairedDevParamReset(PairedButtonParam* buttonParam,uint8 max_num)
{
    os_memset(buttonParam,0,sizeof(PairedButtonParam));
	buttonParam->MaxPairedDevNum = (max_num>MAX_BUTTON_NUM)?MAX_BUTTON_NUM:max_num;
    buttonParam->PairedNum=0;
	sp_PairedParamSave(buttonParam);
}

void ICACHE_FLASH_ATTR
sp_MacInit()
{
	os_printf("SP MAC INIT...\r\n");
	config_ParamLoadWithProtect(LIGHT_PAIRED_DEV_PARAM_ADDR,0,(uint8*)&PairedDev,sizeof(PairedDev));
	if(config_ParamCsumCheck((uint8*)&PairedDev, sizeof(PairedDev)) && PairedDev.magic == SP_PARAM_MAGIC){
		os_printf("Paired dev check ok...\r\n");
	}else{
		os_printf("Paired dev not valid...\r\n");
		sp_PairedDevParamReset(&PairedDev,MAX_BUTTON_NUM);
	}
    //sp_PairedDevParamReset(&PairedDev,MAX_BUTTON_NUM);
    os_printf("===============\r\n");
}

int ICACHE_FLASH_ATTR 
	sp_FindPairedDev(PairedButtonParam* buttonParam,uint8* button_mac)
{
    uint8 i=0;
    for(i=0;i<buttonParam->MaxPairedDevNum;i++){
        if(0 == os_memcmp(&buttonParam->PairedList[i].mac_t,button_mac,6)){
            return i; 
        }
    } 
	return -1;

}


bool ICACHE_FLASH_ATTR sp_AddPairedDev(PairedButtonParam* buttonParam,uint8* button_mac,uint8* key,uint8 channel)
{
    if((buttonParam==NULL)||(buttonParam->PairedNum == buttonParam->MaxPairedDevNum)){
        return  false;
    }
	
    int i=sp_FindPairedDev(buttonParam,button_mac);
    if(i>-1 && i< buttonParam->PairedNum){
		os_memcpy(&buttonParam->PairedList[i].mac_t,button_mac,DEV_MAC_LEN);
		os_memcpy(&buttonParam->PairedList[i].key_t,key,ESPNOW_KEY_LEN);
		buttonParam->PairedList[i].channel_t = channel;
		sp_PairedParamSave(buttonParam);
		return true;
    }	
	uint8 idx = buttonParam->PairedNum;
	os_memcpy(&buttonParam->PairedList[idx].mac_t,button_mac,DEV_MAC_LEN);
	os_memcpy(&buttonParam->PairedList[idx].key_t,key,ESPNOW_KEY_LEN);
	buttonParam->PairedList[idx].channel_t = channel;
	buttonParam->PairedNum++;
	sp_PairedParamSave(buttonParam);
    return true;
}

LOCAL void ICACHE_FLASH_ATTR 
	sp_PopPairedDev(PairedButtonParam* buttonParam,uint8 idx)
{
	int i_last = buttonParam->PairedNum-1;
	if(idx<buttonParam->PairedNum-1){
		os_memcpy(&buttonParam->PairedList[idx],&buttonParam->PairedList[i_last],sizeof(PairedSingleDev));
	}
	//os_memset(buttonParam->button_mac[buttonParam->PairedNum-1],0,DEV_MAC_LEN);
	os_memset(&buttonParam->PairedList[i_last],0,sizeof(PairedSingleDev));
	buttonParam->PairedNum-=1;
}

bool ICACHE_FLASH_ATTR sp_DelPairedDev(PairedButtonParam* buttonParam,uint8* button_mac)
{
    if((buttonParam==NULL)||((buttonParam->PairedNum)==0)){
        return  false;
    }
    uint8 i=0;
    for(i=0;i<buttonParam->PairedNum;i++){
        if(0 == os_memcmp(&buttonParam->PairedList[i],button_mac,DEV_MAC_LEN)){
			sp_PopPairedDev(buttonParam,i);
			sp_PairedParamSave(buttonParam);
            return true;	
        }
    }
    return false;	 
}

uint8 ICACHE_FLASH_ATTR sp_GetPairedNum(PairedButtonParam* buttonParam)
{
    return (buttonParam->PairedNum);
}

bool ICACHE_FLASH_ATTR sp_PairedDevMac2Str(PairedButtonParam* buttonParam,uint8* SaveStrBuffer,uint16 buf_len)
{ 
#define MACSTR_PRIVATE "%02X%02X%02X%02X%02X%02X"
	uint8 i=0;
    uint8 cnt=sp_GetPairedNum(buttonParam);
	
	os_printf("PairedDevNum = %d\n",cnt);
	//os_printf("the Mac In pool pos =0x%x\n",pos_flag);
	if(cnt==0x00){
	    return true;
	}

	uint16 len_cnt = 0;
	for(i=0;i<cnt;i++){
		if(len_cnt>buf_len-12){
			os_printf("length error in sp_PairedDevMac2Str\r\n");
			return false;
		}
        os_sprintf(SaveStrBuffer+len_cnt,MACSTR_PRIVATE,MAC2STR(buttonParam->PairedList[i].mac_t));
        len_cnt+=12;

	}
	return true;
#undef MACSTR_PRIVATE
}

void ICACHE_FLASH_ATTR sp_DispPairedDev(PairedButtonParam* pairingInfo)
{
    uint8 i=0;
    for(i=0;i<pairingInfo->MaxPairedDevNum;i++){
        os_printf("Cnt=%d,PairedNum=%d------------------------\n",i,pairingInfo->PairedNum);
        os_printf("MAC:");
        sp_PrintBuf(pairingInfo->PairedList[i].mac_t,6);
		os_printf("KEY:");
        sp_PrintBuf(pairingInfo->PairedList[i].key_t,16);
    }
}



PairedButtonParam* ICACHE_FLASH_ATTR
	sp_GetPairedParam()
{
	return &PairedDev;
}



void sp_PrintBuf(uint8 * ch,uint8 len)
{
    uint8 i=0;
    for(i=0;i<len;i++) os_printf("%x ",ch[i]);
    os_printf("\n");
}

void ICACHE_FLASH_ATTR
sp_LightPairRefuse()
{
    simple_pair_set_peer_ref(buttonPairingInfo.button_mac, buttonPairingInfo.tempkey, NULL);
    simple_pair_ap_refuse_negotiate();
	simple_pair_deinit();//check
}


void ICACHE_FLASH_ATTR
sp_LightPairGenKey(uint8* key_buf)// length must be 16 bytes
{
	int i;
	uint32 r_v;
	for(i=0;i<4;i++){
		r_v = os_random();
		os_memcpy(key_buf+4*i,&r_v,4);
	}
}

void ICACHE_FLASH_ATTR
sp_LightPairStart()
{
    sp_LightPairGenKey(buttonPairingInfo.espnowKey);

    simple_pair_set_peer_ref(buttonPairingInfo.button_mac, buttonPairingInfo.tempkey, buttonPairingInfo.espnowKey);
    simple_pair_ap_start_negotiate();
    os_printf("send mac\n");
    sp_PrintBuf(buttonPairingInfo.button_mac,6);
    os_printf("channel =%d\n",wifi_get_channel());
    os_printf("tmpkey\n");
    sp_PrintBuf(buttonPairingInfo.tempkey,16);
    //sp_PrintBuf(tmpkey,16);
    os_printf("tmpkey lenght=%d\n",sizeof(buttonPairingInfo.tempkey));
	os_printf("espnow key: \r\n");
	sp_PrintBuf(buttonPairingInfo.espnowKey,16);
}





void ICACHE_FLASH_ATTR
sp_LightPairRequestPermission()
{
#if 1
    char data_body[200];
    os_bzero(data_body,sizeof(data_body));
    uint8 mac_sta[6] = {0};
    wifi_get_macaddr(STATION_IF, mac_sta);
    os_sprintf(data_body,"{\"device_mac\":\"%02X%02X%02X%02X%02X%02X\",\"button_mac\":\"%02X%02X%02X%02X%02X%02X\",\"path\":\"%s\"}",MAC2STR(mac_sta),MAC2STR(buttonPairingInfo.button_mac),PAIR_FOUND_REQUEST);
	
	if (!mesh_json_add_elem(data_body, sizeof(data_body), pair_sip, ESP_MESH_JSON_IP_ELEM_LEN)) {
    	return;
    }
    if (!mesh_json_add_elem(data_body, sizeof(data_body), pair_sport, ESP_MESH_JSON_PORT_ELEM_LEN)) {
	    return;
    }
    char* dev_mac = (char*)mesh_GetMdevMac();
    if (!mesh_json_add_elem(data_body, sizeof(data_body), dev_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN)) {
	    return;
    }
    #if ESP_MESH_SUPPORT
	response_send_str((struct espconn*)user_GetUserPConn(),true,data_body,os_strlen(data_body),NULL,0,0,0);
	#else
	response_send_str((struct espconn*)user_GetWebPConn(),true,data_body,os_strlen(data_body),NULL,0,0,0);
	#endif
#endif
}


void ICACHE_FLASH_ATTR
sp_LightPairReportResult(bool res)
{
#if 1
	os_printf("sp_LightPairReportResult %d \r\n",res);
    char data_body[300];
    os_bzero(data_body,sizeof(data_body));
    uint8 mac_sta[6] = {0};
    wifi_get_macaddr(STATION_IF, mac_sta);
    os_sprintf(data_body,"{\"device_mac\":\"%02X%02X%02X%02X%02X%02X\",\"button_mac\":\"%02X%02X%02X%02X%02X%02X\",\"result\":%d,\"path\":\"%s\"}",MAC2STR(mac_sta),MAC2STR(buttonPairingInfo.button_mac),res,PAIR_RESULT);
	//{"device_mac":"...","result":1,"path":"/device/button/pair/result"}
	if (!mesh_json_add_elem(data_body, sizeof(data_body), pair_sip, ESP_MESH_JSON_IP_ELEM_LEN)) {
    	return;
    }
    if (!mesh_json_add_elem(data_body, sizeof(data_body), pair_sport, ESP_MESH_JSON_PORT_ELEM_LEN)) {
	    return;
    }
    char* dev_mac = (char*)mesh_GetMdevMac();
    if (!mesh_json_add_elem(data_body, sizeof(data_body), dev_mac, ESP_MESH_JSON_DEV_MAC_ELEM_LEN)) {
	    return;
    }
    #if ESP_MESH_SUPPORT
	response_send_str((struct espconn*)user_GetUserPConn(),true,data_body,os_strlen(data_body),NULL,0,0,0);
	#else
	response_send_str((struct espconn*)user_GetWebPConn(),true,data_body,os_strlen(data_body),NULL,0,0,0);
	#endif
#endif
}


void ICACHE_FLASH_ATTR
sp_LightPairReplyKeepAlive()
{
	char data_resp[100];
	os_bzero(data_resp,sizeof(data_resp));
	os_sprintf(data_resp, "{\"status\":200,\"path\":\"%s\"}",PAIR_KEEP_ALIVE);
	os_printf("response send str...\r\n");
	#if ESP_MESH_SUPPORT
	response_send_str((struct espconn*)user_GetUserPConn(), true, data_resp,os_strlen(data_resp),NULL,0,1,0);
	#else
	response_send_str((struct espconn*)user_GetWebPConn(), true, data_resp,os_strlen(data_resp),NULL,0,1,0);
	#endif
}


void ICACHE_FLASH_ATTR
sp_LightReplyPairedDev()
{
	uint8* MacListBuf = (uint8*)os_zalloc(PairedDev.PairedNum*DEV_MAC_LEN*2+1);
    uint8 cnt=sp_GetPairedNum(&PairedDev);
	
    sp_PairedDevMac2Str(&PairedDev,MacListBuf,sizeof(MacListBuf)-1);
    #if ESP_MESH_SUPPORT
	response_send_str((struct espconn*)user_GetUserPConn(), true, MacListBuf,os_strlen(MacListBuf),NULL,0,1,1);
    #else
	response_send_str((struct espconn*)user_GetWebPConn(), true, MacListBuf,os_strlen(MacListBuf),NULL,0,1,1);
    #endif

	if(MacListBuf){
		os_free(MacListBuf);
		MacListBuf = NULL;
	
	}

}



void ICACHE_FLASH_ATTR
sp_LightPairStateCb(uint8 *mac,uint8 state)
{
    os_printf("state=%d\n",state);
    os_printf("receive mac\n");
    sp_PrintBuf(mac,6);
    if(state==SP_ST_AP_RECV_NEG){   //receive pairing request, ask phone for permission   
		os_printf("SHOULD WE CHECK MAC HERE...\r\n");
		os_memcpy(buttonPairingInfo.button_mac,mac,DEV_MAC_LEN);
		buttonPairingInfo.simple_pair_state=LIGHT_RECV_BUTTON_REQUSET;
             
    }else if(state==SP_ST_AP_FINISH){
        buttonPairingInfo.simple_pair_state=LIGHT_SIMPLE_PAIR_SUCCED;
	    os_printf("pair succeed\n");
    }else{
	    buttonPairingInfo.simple_pair_state=LIGHT_SIMPLE_PAIR_FAIL;
    }
	sp_LightPairState(); 
}



void ICACHE_FLASH_ATTR
sp_LightPairAccept(void)
{
	simple_pair_deinit();
    simple_pair_init();
    register_simple_pair_status_cb(sp_LightPairStateCb);
    simple_pair_ap_enter_scan_mode();
}

//CSC
LOCAL os_timer_t lightTout_t;
LOCAL enum SimplePairStatus light_state = SP_LIGHT_IDLE;
void ICACHE_FLASH_ATTR
sp_LightPairTout(void)
{
    if(LIGHT_SIMPLE_PAIR_SUCCED!=buttonPairingInfo.simple_pair_state){
        os_printf("sp_LightPairTout the simple pair fail  timeout\n");
        //simple_pair_deinit();
        //light_state=SP_LIGHT_IDLE;
        buttonPairingInfo.simple_pair_state=SP_LIGHT_ERROR_HANDLE;
		sp_LightPairState();
    }else{
        os_printf("sp_LightPairTout the simple pair succed\n");
    }
}

void ICACHE_FLASH_ATTR
	sp_LightPairTimerStart()
{
	os_timer_disarm(&lightTout_t);
	os_timer_setfn(&lightTout_t, (os_timer_func_t *)sp_LightPairTout, NULL);
	os_timer_arm(&lightTout_t, 60*1000, 0);
}

void ICACHE_FLASH_ATTR
	sp_LightPairTimerStop()
{
	os_timer_disarm(&lightTout_t);

}


void ICACHE_FLASH_ATTR
sp_LightPairState(void)
{

#if 1
    switch(light_state){
        case (SP_LIGHT_IDLE):
            os_printf("status:SP_LIGHT_IDLE\n");
			//pair start command, correct
            if(USER_PBULIC_BUTTON_INFO==buttonPairingInfo.simple_pair_state){
                os_printf("statue:Get button Info,next wait button request\n");
				sp_LightPairTimerStart();
                sp_LightPairAccept();
                light_state=SP_LIGHT_WAIT_BUTTON_REQUEST;
				light_shadeStart(HINT_WHITE,500,3,1,NULL);
            }
			//exception
            else{
                light_state=SP_LIGHT_ERROR_HANDLE;
				light_hint_stop(HINT_RED);
                os_printf("buttonPairingInfo.simple_pair_state=%d\n",buttonPairingInfo.simple_pair_state);
				sp_LightPairState();
            }
            break;
        case (SP_LIGHT_WAIT_BUTTON_REQUEST):
            os_printf("status:SP_LIGHT_WAIT_BUTTON_REQUEST\n");
			light_shadeStart(HINT_GREEN,500,3,0,NULL);
			
			//receive button pair request, send req to phone
            if(LIGHT_RECV_BUTTON_REQUSET==buttonPairingInfo.simple_pair_state){
                os_printf("statue:Get button request,next wait user permit or refuse\n"); 
                sp_LightPairRequestPermission();
                light_state=SP_LIGHT_WAIT_USER_INDICATE_PERMIT;
            }
			//receive pair start command again before timeout , right now, restart state machine
			else if(USER_PBULIC_BUTTON_INFO==buttonPairingInfo.simple_pair_state){
                os_printf("statue:Get button Info,restart state machine,wait button request\n");
				sp_LightPairTimerStart();
                sp_LightPairAccept();
                light_state=SP_LIGHT_WAIT_BUTTON_REQUEST;
            }
			//error if other states
			else{
                //simple_pair_deinit();
                light_state=SP_LIGHT_ERROR_HANDLE;
				light_hint_stop(HINT_RED);
                os_printf("err in SP_LIGHT_WAIT_BUTTON_REQUEST ;simple_pair_state=%d\n",buttonPairingInfo.simple_pair_state);
				sp_LightPairState();
            }
            break;
        case (SP_LIGHT_WAIT_USER_INDICATE_PERMIT):
			light_shadeStart(HINT_BLUE,500,3,0,NULL);
            os_printf("status:SP_LIGHT_WAIT_USER_INDICATE_PERMIT\n");
			//phone user permit pairing
            if(USER_PERMIT_SIMPLE_PAIR==buttonPairingInfo.simple_pair_state){
                os_printf("statue:User permit simple pair,next wait simple result\n"); 
				sp_PairedDevRmAll(&PairedDev);//
                sp_LightPairStart();
                light_state=SP_LIGHT_WAIT_SIMPLE_PAIR_RESULT;
            }
			//phone user refuse pairing, END
			else if(USER_REFUSE_SIMPLE_PAIR==buttonPairingInfo.simple_pair_state){
                os_printf("statue:User refuse simple pair,state clear\n");
                //check state
                sp_LightPairRefuse();
                //light_state=SP_LIGHT_IDLE;
                //buttonPairingInfo.simple_pair_state=LIGHT_PAIR_IDLE;
                light_state = SP_LIGHT_END;
				light_shadeStart(HINT_RED,500,2,0,NULL);
				sp_LightPairState();
				
            }
			//exceptions
			else{
                //simple_pair_deinit();
                light_state=SP_LIGHT_ERROR_HANDLE;
				light_hint_stop(HINT_RED);
                os_printf("buttonPairingInfo.simple_pair_state=%d\n",buttonPairingInfo.simple_pair_state);
				sp_LightPairState();
            }
            break;
        case (SP_LIGHT_WAIT_SIMPLE_PAIR_RESULT):
            os_printf("status:SP_LIGHT_WAIT_SIMPLE_PAIR_RESULT\n");
			//pairing finished , END
            if(LIGHT_SIMPLE_PAIR_SUCCED==buttonPairingInfo.simple_pair_state){
                os_printf("status:sp_LightPairSucced\n");  
				light_shadeStart(HINT_WHITE,500,2,0,NULL);

				//1.get key
				//buttonPairingInfo.espnowKey
				//2.add peer and save
            	int res = esp_now_add_peer((uint8*)(buttonPairingInfo.button_mac), (uint8)ESP_NOW_ROLE_CONTROLLER,(uint8)wifi_get_channel(), (uint8*)(buttonPairingInfo.espnowKey), (uint8)ESPNOW_KEY_LEN);
            	os_printf("INIT RES: %d ; MAC:"MACSTR"\r\n",res,MAC2STR(((uint8*)(buttonPairingInfo.button_mac))));
				//3.save
                sp_AddPairedDev(&PairedDev,buttonPairingInfo.button_mac,buttonPairingInfo.espnowKey,wifi_get_channel());
                sp_DispPairedDev(&PairedDev);

                //simple_pair_deinit();//check
                //light_state=SP_LIGHT_IDLE;
                //buttonPairingInfo.simple_pair_state=LIGHT_PAIR_IDLE;
                sp_LightPairReportResult(true);
				light_state = SP_LIGHT_END;
				sp_LightPairState();
            }
			//pairing failed , END
			else if(LIGHT_SIMPLE_PAIR_FAIL==buttonPairingInfo.simple_pair_state){
                os_printf("status:sp_LightPairFail\n");
                //simple_pair_deinit();//check
                //light_state=SP_LIGHT_IDLE;
                //buttonPairingInfo.simple_pair_state=LIGHT_PAIR_IDLE;
                sp_LightPairReportResult(false);
				light_state = SP_LIGHT_END;
				light_hint_stop(HINT_RED);
				sp_LightPairState();
            }
			//exception
			else{
                simple_pair_deinit();
				light_hint_stop(HINT_RED);
                light_state=SP_LIGHT_ERROR_HANDLE;
                os_printf("buttonPairingInfo.simple_pair_state=%d\n",buttonPairingInfo.simple_pair_state);
				sp_LightPairState();
            }
            break; 
			
		case (SP_LIGHT_ERROR_HANDLE):
			os_printf("status:simpaire in Err!!!\r\nBACK TO IDLE\r\n"); 
			light_hint_stop(HINT_RED);
        case (SP_LIGHT_END):
			//reset state, deinit simplepair, stop tout timer
			sp_LightPairTimerStop();
			simple_pair_deinit();
			light_state=SP_LIGHT_IDLE;
			buttonPairingInfo.simple_pair_state=LIGHT_PAIR_IDLE;
			sp_PairedDevLoadAll(&PairedDev);
			break;
			  
        default:
            os_printf("status:unsafe param: %d\n",light_state);
            break;
    }

#endif
}









