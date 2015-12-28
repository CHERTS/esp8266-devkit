/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Shanghai)
 *
 * Description: Routines to use simple_pair to connect the button to a light.
*******************************************************************************/


/*
Procedure for simple pairing:

    Button (station mode)
    1. initial "simple pair" as same as Light's
        void sp_cb(u8 state)          
            simple_pair_init();
           register_simple_pair_status_cb(sp_cb);

    2. enter in scan mode,
        simple_pair_sta_enter_scan_mode()

    3. scan ap then choose a light from ap list (which bss_link->simple_pair == 1, indcate the AP allow button to pair with)
        wifi_station_scan()
       
    4. set parameters and start negotiate
        simple_pair_set_espnow_peer_ref(peer_addr, peer_role, 1,  NULL, 0); // Button don't know the espnow key, so don't set
            simple_pair_set_tmp_key(tmpkey, key_len); //key len must be 16 bytes

            simple_pair_ap_start_negotiate();
       
    5. wait for sp_cb call back function with the status value == 0 (indicate finish, other value means error except 1)
   
    6. when sp_cb success, APP layer send esp-now package to certify that the esp-now is OK.
        if all finished, call simple_pair_deinit();       
*/


#include "ets_sys.h"
#include "osapi.h"
#include "user_switch.h"
#include "user_interface.h"
#include "user_buttonsimplepair.h"
#include "espnow.h"


static buttonSimplePairDoneCb doneCb;

//===========================================================
//PAIRED DEVICE RECORD
//============================
/*----------------------------------------button_info_pool----------------------------------------*/

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
}

void ICACHE_FLASH_ATTR sp_PairedDevParamReset(PairedButtonParam* buttonParam,uint8 max_num)
{
    //uint8 i=0;
    //for(i=0;i<depth;i++){
    //    memset(pairingInfo->button_mac[i],0,6);
    //}
    os_memset(buttonParam,0,sizeof(PairedButtonParam));
	buttonParam->MaxPairedDevNum = (max_num>MAX_BUTTON_NUM)?MAX_BUTTON_NUM:max_num;
    buttonParam->PairedNum=0;
	sp_PairedParamSave(buttonParam);
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
    #if 0
    uint8 i = idx;
	for(i;i<buttonParam->PairedNum-1;i++){
		os_memcpy(buttonParam->button_mac[i],buttonParam->button_mac[i+1],DEV_MAC_LEN);
	}
	#endif
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

//uint8 ICACHE_FLASH_ATTR GetPoolBlankNodeCnt(PairedButtonParam* buttonParam)
//{
//    return (buttonParam->blank_node_cnt);
//}
uint8 ICACHE_FLASH_ATTR sp_GetPairedNum(PairedButtonParam* buttonParam)
{
    return (buttonParam->PairedNum);
}

#if 0
bool ICACHE_FLASH_ATTR GetPoolNodeIsEmpty(PairedButtonParam* buttonParam,uint8 i)
{
    uint8 c[6]={0,0,0,0,0,0};
	if(i>=MAX_BUTTON_NUM){
        os_printf("GetPoolNodeIsEmpty tht param unsafe i=%d\n",i);
		return false;
	}
    if(!os_memcmp(buttonParam->button_mac[i],c,6)){
        return true;
	}
	return false;
}
#endif

#if 0
//not used
uint16 ICACHE_FLASH_ATTR GetButtonsMacInPoolPos(PairedButtonParam* buttonParam)
{

    uint8 i=0;
	uint16 PosFlag=0;//Pool?D¦Ì?MAC BUFFER x????¨®D¨ºy?Y¡ê?PosFlag¦Ì?x???¨ª¡À?????
	for(i=0;i<MAX_BUTTON_NUM;i++){
        if(!GetPoolNodeIsEmpty(buttonParam,i)){
           SetBit(PosFlag,i);
        }
	}
	return PosFlag;
}
#endif


#if 0
#define MACSTR_PRIVATE "%02X%02X%02X%02X%02X%02X"
bool ICACHE_FLASH_ATTR sp_PairedDevMac2Str(PairedButtonParam* buttonParam,uint8* SaveStrBuffer,uint16 buf_len)
{ 
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
#endif

#if 0
//send button mac list to phone
void ICACHE_FLASH_ATTR ResonseUserInquireButtonMacBody(uint8* SaveStrBuffer)
{
       uint8 MacListBuf[MAX_BUTTON_NUM*6*2+1];
	uint8 cnt=sp_GetPairedNum(&PairedDev);
	os_bzero(MacListBuf,sizeof(MacListBuf));
	sp_PairedDevMac2Str(&PairedDev,MacListBuf,sizeof(MacListBuf)-1);
	os_sprintf(SaveStrBuffer,"{\"status\":200,\"mac_len\":%d,\"mac\":\"%s\"}",cnt,MacListBuf);
	//os_printf("ResonseUserInquireButtonMacBody:\n%s",SaveStrBuffer);
}
#endif




#if 0
void ICACHE_FLASH_ATTR sp_PrintBuf(uint8* data,uint8 len)
{
   uint8 i=0;
   for(i=0;i<len;i++){
   	 os_printf("%02x ",data[i]);
   }	
}
#endif

//bianli-traverse
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

void ICACHE_FLASH_ATTR
sp_MacInit()
{
	os_printf("SP MAC INIT: %d\r\n",sizeof(PairedDev));
	config_ParamLoadWithProtect(LIGHT_PAIRED_DEV_PARAM_ADDR,0,(uint8*)&PairedDev,sizeof(PairedDev));
	if(config_ParamCsumCheck((uint8*)&PairedDev, sizeof(PairedDev)) && PairedDev.magic == SP_PARAM_MAGIC){
		os_printf("Paired dev check ok...\r\n");
	}else{
		os_printf("Paired dev not valid...\r\n");
		sp_PairedDevParamReset(&PairedDev,MAX_BUTTON_NUM);
	}
	sp_DispPairedDev(&PairedDev);
    //sp_PairedDevParamReset(&PairedDev,MAX_BUTTON_NUM);
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



//END OF PAIRED DEVICE RECORD
//-----------------------------------------------------------



static void ICACHE_FLASH_ATTR simplePairCallback(u8 *sa, uint8_t state) {
	os_printf("simplePairState %d\n",state);
	if (state!=SP_ST_AP_RECV_NEG) {
		if(state==SP_ST_STA_FINISH){
			uint8 PairingKey[ESPNOW_KEY_LEN];
			simple_pair_get_peer_ref(NULL, NULL, PairingKey);
			sp_AddPairedDev(&PairedDev,sa,PairingKey,wifi_get_channel());
			esp_now_add_peer(sa,ESP_NOW_ROLE_SLAVE,0,PairingKey,ESPNOW_KEY_LEN);
			sp_DispPairedDev(&PairedDev);
			

		}
		if(doneCb)
		    doneCb(state==SP_ST_STA_FINISH);
		simple_pair_deinit();
		_SWITCH_GPIO_RELEASE();
	}else{
		os_printf("ST: ST_SP_AP_RECV_NEG\r\n");
		simple_pair_deinit();
		os_printf("RELEASE GPIO...\r\n");
		_SWITCH_GPIO_RELEASE();
	}
}


os_timer_t PairTout_t;
void ICACHE_FLASH_ATTR ButtonPairTout()
{
	os_timer_disarm(&PairTout_t);
	os_printf("Pair timeout...\r\n");
	UART_WaitTxFifoEmpty(0,50000);
	_SWITCH_GPIO_RELEASE();
}

void ICACHE_FLASH_ATTR ButtonPairToutStart()
{
	os_timer_disarm(&PairTout_t);
	os_timer_setfn(&PairTout_t,ButtonPairTout,NULL);
	os_timer_arm(&PairTout_t,20*1000,0);

}
static void ICACHE_FLASH_ATTR ButtonStatrNegotiate(uint8* peer_mac,uint8*tempkey){
	
	////esp_now_del_peer(peer_mac);
	 simple_pair_set_peer_ref(peer_mac, tempkey, NULL);
    //simple_pair_set_tmp_key(tmpkey, 16); //key len must be 16 bytes
    simple_pair_sta_start_negotiate();

	ButtonPairToutStart();
}
static void ICACHE_FLASH_ATTR wifiScanDoneCb(void *arg, STATUS status) {
	uint8 tmpkey[16] = {0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
	struct bss_info *bss_link = (struct bss_info *)arg;
	struct bss_info *bestbss=NULL;
	while (bss_link != NULL) {
		if (bss_link->simple_pair==1) {
			if (bestbss==NULL || bestbss->rssi<bss_link->rssi) {
				bestbss=bss_link;
			}
		}
		os_printf("ssid: %s ; rssi : %d ; channel: %d \r\n",bss_link->ssid,bss_link->rssi,bss_link->channel);
		
		bss_link=bss_link->next.stqe_next;
	}
	if (bestbss==NULL) {
		os_printf("Simple pair failed: no simple pair capable AP found.\r\n");
		//Simple pair failed: no simple pair capable AP found.
		if(doneCb)
		    doneCb(0);
		
		simple_pair_deinit();
		os_printf("RELEASE GPIO...\r\n");
		_SWITCH_GPIO_RELEASE();
	} else {
        if(wifi_set_channel(bestbss->channel)){
            os_printf("Sta set channel OK!\n");
        }
    #if 0
    simple_pair_set_espnow_peer_ref(bestbss->bssid, ESP_NOW_ROLE_CONTROLLER, 1,  NULL, 0);
    simple_pair_set_tmp_key(tmpkey, 16);
    simple_pair_sta_start_negotiate();
    #endif
	sp_PairedDevRmAll(&PairedDev);
    ButtonStatrNegotiate(bestbss->bssid,tmpkey);
    os_printf("Button will Pair light MAC"MACSTR",channel=%d\n",MAC2STR(bestbss->bssid),bestbss->channel);
	}
}



void ICACHE_FLASH_ATTR buttonSimplePairStart(buttonSimplePairDoneCb cb) {
	doneCb=cb;
	simple_pair_init();
	register_simple_pair_status_cb(simplePairCallback);
	simple_pair_sta_enter_scan_mode();
	wifi_station_scan(NULL, wifiScanDoneCb);
}

