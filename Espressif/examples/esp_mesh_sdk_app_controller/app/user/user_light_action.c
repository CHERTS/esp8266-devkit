
#include "os_type.h"
#include "eagle_soc.h"
#include "c_types.h"
#include "osapi.h"
#include "ets_sys.h"
#include "mem.h"
#include "user_light_action.h"
#include "user_switch.h"
#include "user_interface.h"
#include "espnow.h"
#include "user_buttonsimplepair.h"


extern uint8 switch_gpio_val;
//The espnow packet struct used for light<->switch intercommunications
typedef struct{
    uint16 battery_voltage_mv;
    uint16 battery_status;
}EspnowBatStatus;

typedef struct {
	uint16 pwm_num;
	uint32 period;
	uint32 duty[8];
	uint32 cmd_code;
	//uint8 color_idx;
	//uint16 battery_voltage_mv;
	//uint16 battery_status;
    EspnowBatStatus batteryStat;
	//uint32 check_sum; //move to outside
} EspnowLightCmd;

typedef struct{
    uint8 ip_addr[4];
	uint8 channel;
	uint8 meshStatus;
	uint8 meshLevel;
	uint8 platformStatus;
	uint8 espCloudConnIf;
	uint8 espCloudRegIf;
	uint8 devKey[40];
}EspnowLightRsp;

typedef struct{
    uint8 switchChannel;
	uint8 lightChannel;
    EspnowBatStatus batteryStat;
}EspnowLightSync;


typedef enum{
    ACT_IDLE = 0,
    ACT_ACK = 1,
    ACT_REQ = 2,
    ACT_TIME_OUT =3,
    ACT_RSP=4,
}EspnowMsgStatus;

typedef enum{
    ACT_TYPE_DATA = 0,
    ACT_TYPE_ACK = 1,//NOT USED, REPLACED BY MAC ACK
    ACT_TYPE_SYNC = 2,
    ACT_TYPE_RSP = 3,
    ACT_TYPE_SIMPLE_CMD = 4, //CMD 1-32
}EspnowMsgType;


typedef enum {
	ACT_BAT_NA = 0,
	ACT_BAT_OK = 1,
	ACT_BAT_EMPTY = 2
}ACT_BAT_TYPE;

typedef struct{
    uint8 csum;//checksum
    uint8 type;//frame type
    uint32 token;//random value
    uint16 cmd_index;//for retry
	uint8 wifiChannel;//self channel
	uint16 sequence;//sequence
	uint16 io_val;//gpio read value
	uint8 rsp_if;//need response
    union{
    EspnowLightCmd lightCmd;
    EspnowLightRsp lightRsp;
    EspnowLightSync lightSync;
    };
}EspnowProtoMsg;


#if 0
typedef struct{
    //uint8 csum;
    uint32 magic;
    uint16 SlaveNum;
    uint8 SlaveMac[LIGHT_DEV_NUM][DEV_MAC_LEN];
    uint8 esp_now_key[ESPNOW_KEY_LEN];
	uint8 wifiChannel[LIGHT_DEV_NUM];
}SwitchEspnowParam;

SwitchEspnowParam switchEspnowParam;
#endif





#define BAT_EMPTY_MV 2100

#define ACT_DEBUG 1
#define ESPNOW_DBG os_printf
#define WIFI_DEFAULT_CHANNEL 1


#ifdef LIGHT_SWITCH

#define ACTION_RETRY_NUM  2
#define ACTION_RETRY_TIMER_MS  200
#define ACTION_CMD_RSP  0
typedef void (*ActionToutCallback)(uint32* act_idx);

typedef struct{
    uint16 sequence;//chg u32 to u16
    EspnowMsgStatus status;
    os_timer_t req_timer;
    uint32 retry_num;
    uint32 retry_expire;
    ActionToutCallback actionToutCb;
    EspnowProtoMsg EspnowMsg;
    uint16 wifichannel;
}Action_SendStatus;

os_timer_t action_retry_t;
Action_SendStatus actionReqStatus[LIGHT_DEV_NUM] = {0};

LOCAL uint8 channel_group[LIGHT_DEV_NUM];
LOCAL uint8 channel_num = 0;
LOCAL uint8 channel_cur = 1;
LOCAL uint8 pwm_chn_num = 5;
LOCAL uint32 pwm_duty[8];
LOCAL uint32 pwm_period;
LOCAL uint32 cmd_code=0;

typedef enum{
	CUR_CHL_OK = 0,
	ALL_CHL_OK,
	CUR_CHL_WAIT,
}EspnowReqRet;
#endif

bool ICACHE_FLASH_ATTR
	light_EspnowCmdValidate(EspnowProtoMsg* EspnowMsg)
{
    uint8* data = (uint8*)EspnowMsg;
    uint8 csum_cal = 0;
    int i;
    for(i=1;i<sizeof(EspnowProtoMsg);i++){
		csum_cal+= *(data+i);
    }
	
	os_printf("csum cal: %d ; csum : %d \r\n",csum_cal,EspnowMsg->csum);
    if(csum_cal == EspnowMsg->csum){
        return true;
    }else{
        return false;
    }
}


void ICACHE_FLASH_ATTR
	light_EspnowSetCsum(EspnowProtoMsg* EspnowMsg)
{
    uint8* data = (uint8*)EspnowMsg;
    uint8 csum_cal = 0;
    int i;
    for(i=1;i<sizeof(EspnowProtoMsg);i++){
		csum_cal+= *(data+i);
    }
    EspnowMsg->csum= csum_cal;
	os_printf("csum cal: %d ; csum : %d \r\n",csum_cal,EspnowMsg->csum);
	
}

#if LIGHT_SWITCH

EspnowReqRet ICACHE_FLASH_ATTR
switch_CheckCmdResult()
{
    int i;
    EspnowReqRet ret = CUR_CHL_OK;// ALL_CHL_OK;
    //for(i=0;i<switchEspnowParam.SlaveNum;i++){
	for(i=0;i<PairedDev.PairedNum;i++){
        //os_printf("actionReqStatus[channel_group[i]]:%d\r\n",actionReqStatus[channel_group[i]]);
        if(actionReqStatus[channel_group[i]].status == ACT_REQ) return CUR_CHL_WAIT;
		#if ACTION_CMD_RSP
        if(actionReqStatus[channel_group[i]].status == ACT_ACK) return CUR_CHL_WAIT;
		#endif
        if(actionReqStatus[channel_group[i]].status == ACT_TIME_OUT) ret = CUR_CHL_OK;
    }
    return ret;
}


void ICACHE_FLASH_ATTR
switch_EspnowAckCb()
{
    EspnowReqRet ret = switch_CheckCmdResult();
    
    if( ret==CUR_CHL_OK){
        if(channel_cur == 14){
            ESPNOW_DBG("release power\r\n");
            _SWITCH_GPIO_RELEASE();
        }else{
            ESPNOW_DBG("NEXT CHANNEL: %d \r\n",++channel_cur);
            switch_EspnowSendCmdByChnl(channel_cur,pwm_chn_num, pwm_duty, pwm_period,cmd_code);
        }
    
    }else if(ret == ALL_CHL_OK){
        ESPNOW_DBG("ALL CHANNEL OK,release power\r\n");
        _SWITCH_GPIO_RELEASE();
    }
    
}



EspnowReqRet ICACHE_FLASH_ATTR
switch_CheckSyncResult()
{
    int i;
    EspnowReqRet ret = ALL_CHL_OK;
    for(i=0;i<PairedDev.PairedNum;i++){ //CMD NUM QUEALS PRACTICAL LIGHT NUMBER
        //USING MAC ACK CB, SYNC NEED A ACT_RSP
        if(actionReqStatus[i].status == ACT_REQ || actionReqStatus[i].status == ACT_ACK ) return CUR_CHL_WAIT;
        if(actionReqStatus[i].status==ACT_TIME_OUT)	ret=CUR_CHL_OK ;
    }
    os_printf("%s ret: %d \r\n",__func__,ret);
    return ret;
}


void ICACHE_FLASH_ATTR
switch_EspnowSyncExit()
{
    ESPNOW_DBG("release power\r\n");
    int i;
    for(i=0;i<PairedDev.PairedNum;i++){
        //switchEspnowParam.magic = ESPNOW_PARAM_MAGIC;
        PairedDev.PairedList[i].channel_t = actionReqStatus[i].wifichannel;
    }
	
    //system_param_save_with_protect(ESPNOW_PARAM_SEC,&switchEspnowParam,sizeof(switchEspnowParam));
	sp_PairedParamSave(&PairedDev);
    _SWITCH_GPIO_RELEASE();
}


void ICACHE_FLASH_ATTR
switch_EspnowSyncCb()
{
    EspnowReqRet ret = switch_CheckSyncResult();
    os_printf("ACT RET: %d \r\n",ret);
    if(ret==CUR_CHL_OK){
        os_printf("SYNC FINISHED ...\r\n");
        if(channel_cur == 14){
            switch_EspnowSyncExit();
        }else{
            ESPNOW_DBG("SYNC NEXT CHANNEL: %d \r\n",++channel_cur);
            //switch_EspnowSendCmdByChnl(channel_cur,pwm_chn_num, pwm_duty, pwm_period);
            switch_EspnowSendChnSync(channel_cur);
        }
    }else if(ret == ALL_CHL_OK){
        ESPNOW_DBG("ALL CHANNEL OK,\r\n");
        switch_EspnowSyncExit();
    }
}




void ICACHE_FLASH_ATTR
switch_EspnowSendRetry(void* arg)
{
    //ESPNOW_DBG("%s  \r\n",__func__);
    uint16 _idx = *((uint16*)arg);	
    EspnowProtoMsg* EspnowRetryMsg = &(actionReqStatus[_idx].EspnowMsg);
    Action_SendStatus* EspnowSendStatus = &(actionReqStatus[_idx]);
    #if 0
    os_printf("*********************************\r\n");
    os_printf("-----Msg-----\r\n");
    os_printf("actionReqStatus[%d].EspnowMsg.sequence: %d \r\n",_idx,actionReqStatus[_idx].EspnowMsg.sequence);
    os_printf("actionReqStatus[%d].EspnowMsg.type: %d \r\n",_idx,actionReqStatus[_idx].EspnowMsg.type);
    os_printf("-----Status-------\r\n");
    os_printf("actionReqStatus[%d].status : %d \r\n",_idx,actionReqStatus[_idx].status);
    os_printf("actionReqStatus[%d].sequence : %d \r\n",_idx,actionReqStatus[_idx].sequence);
    os_printf("actionReqStatus[%d].retry num: %d \r\n",_idx,actionReqStatus[_idx].retry_num);
    os_printf("*********************************\r\n");
    #endif
    if((EspnowRetryMsg->sequence != EspnowSendStatus->sequence)){
        ESPNOW_DBG("action updated...,cancel retry ...\r\n");
        return;
    }
    if(EspnowRetryMsg->type==ACT_TYPE_DATA){
        //if(EspnowSendStatus->status== ACT_REQ){
        if(EspnowSendStatus->status== ACT_REQ){
            if(EspnowSendStatus->retry_num < ACTION_RETRY_NUM){
                os_printf("retry send data\r\n");
                //esp_now_send((uint8*)switchEspnowParam.SlaveMac[_idx], (uint8*)EspnowRetryMsg, sizeof(EspnowProtoMsg));
                esp_now_send((uint8*)PairedDev.PairedList[_idx].mac_t, (uint8*)EspnowRetryMsg, sizeof(EspnowProtoMsg));
                EspnowSendStatus->retry_num++;
                //os_timer_arm( &action_status->req_timer, action_status->retry_expire,0);
            }
        }
#if ACTION_CMD_RSP
        else if(EspnowSendStatus->status== ACT_ACK){
        	if(EspnowSendStatus->retry_num < ACTION_RETRY_NUM){
        		os_printf("retry send data\r\n");
        		//esp_now_send((uint8*)switchEspnowParam.SlaveMac[_idx], (uint8*)EspnowRetryMsg, sizeof(EspnowProtoMsg));
        		esp_now_send((uint8*)PairedDev.PairedList[_idx].mac_t, (uint8*)EspnowRetryMsg, sizeof(EspnowProtoMsg));
        		EspnowSendStatus->retry_num++;
        		//os_timer_arm( &action_status->req_timer, action_status->retry_expire,0);
        	}
        }
#endif
		else{
            ESPNOW_DBG("[%d] CMD ACKed, STATUS  : %d\r\n",_idx,EspnowSendStatus->status);
        }
    }
    else if(EspnowRetryMsg->type==ACT_TYPE_SYNC){
        if(EspnowSendStatus->status== ACT_REQ || EspnowSendStatus->status== ACT_ACK){
            if(EspnowSendStatus->retry_num < ACTION_RETRY_NUM){
                os_printf("retry send sync\r\n");
                esp_now_send((uint8*)PairedDev.PairedList[_idx].mac_t, (uint8*)EspnowRetryMsg, sizeof(EspnowProtoMsg));
                EspnowSendStatus->retry_num++;
                //os_timer_arm( &action_status->req_timer, action_status->retry_expire,0);
            }
        }else{
            ESPNOW_DBG("[%d] REPed, STATUS  : %d\r\n",_idx,EspnowSendStatus->status);
        }
    }
}

os_timer_t espnow_send_t;
int send_idx;
void ICACHE_FLASH_ATTR
	switch_EspnowSendFunc()
{
	//for(send_idx=0;send_idx<channel_num;send_idx++){
	
	if(send_idx<channel_num){
		os_timer_disarm(&espnow_send_t);
		os_printf("t: %d \r\n",system_get_time());
		switch_EspnowSendLightCmd(channel_group[send_idx], pwm_chn_num, pwm_duty, pwm_period , cmd_code);
		send_idx++;
		os_timer_arm(&espnow_send_t,10,0);
	}else{

	}

}
void ICACHE_FLASH_ATTR 
switch_EspnowSendCmdByChnl(uint16 chn,uint16 channelNum, uint32* duty, uint32 period,uint32 code)
{
    int i = 0;
    os_memset(channel_group, 0 ,sizeof(channel_group));
    channel_num = 0;
    channel_cur = chn;
    cmd_code = code;
    pwm_period = period;
    os_memcpy(pwm_duty,duty,sizeof(pwm_duty));
    pwm_chn_num = channelNum;
    
    for(i=0;i<PairedDev.PairedNum;i++){
        if(actionReqStatus[i].wifichannel == chn){
            channel_group[channel_num++]=i;
            ESPNOW_DBG("CHANNEL %d : add idx %d\r\n",chn,i);
        }
    }
    #if 0
    os_printf("********************\r\n");
    os_printf("cur chn: %d ; chn num: %d \r\n",chn,channel_num);
    os_printf("********************\r\n");
    #endif
    if(channel_num>0){
        ESPNOW_DBG("WIFI SET CHANNEL : %d \r\n",channel_cur);
        wifi_set_channel(channel_cur);
        ESPNOW_DBG("WIFI GET CHANNEL : %d \r\n",wifi_get_channel());
		#if 0
        os_timer_setfn(&espnow_send_t,switch_EspnowSendFunc,NULL);
		send_idx = 0;
		for(i=0;i<channel_num;i++){
			os_timer_disarm(&actionReqStatus[channel_group[i]].req_timer); //disarm retry timer;
			//actionReqStatus[idx].sequence+=1 ;//send another seq of cmd
			actionReqStatus[channel_group[i]].status= ACT_REQ;
			//actionReqStatus[idx].retry_num = 0;
		}
		switch_EspnowSendFunc();
		#else
        for(i=0;i<channel_num;i++){
			os_printf("t: %d \r\n",system_get_time());
            switch_EspnowSendLightCmd(channel_group[i], channelNum, duty, period , code);
        }
		#endif
    }else{
        switch_EspnowAckCb();//next channel;
    }
}


extern uint32 user_GetBatteryVoltageMv();

void ICACHE_FLASH_ATTR
	switch_EspnowBuildPacket()
{


}
void ICACHE_FLASH_ATTR 
switch_EspnowSendLightCmd(uint16 idx, uint16 channelNum, uint32* duty, uint32 period,uint32 code)
{
    os_timer_disarm(&actionReqStatus[idx].req_timer); //disarm retry timer;
    actionReqStatus[idx].sequence+=1 ;//send another seq of cmd
    actionReqStatus[idx].status= ACT_REQ;
    actionReqStatus[idx].retry_num = 0;
    
    EspnowProtoMsg EspnowMsg;
    EspnowMsg.csum = 0;
    EspnowMsg.type = ACT_TYPE_DATA;
    EspnowMsg.token = os_random();
    EspnowMsg.cmd_index=idx;
    EspnowMsg.wifiChannel = wifi_get_channel();
    EspnowMsg.sequence = actionReqStatus[idx].sequence;
	EspnowMsg.io_val = switch_gpio_val;
	#if ACTION_CMD_RSP
	EspnowMsg.rsp_if = 1;
    #else
	EspnowMsg.rsp_if = 0;
	#endif
    EspnowMsg.lightCmd.pwm_num = channelNum;
    EspnowMsg.lightCmd.period = period;
    os_memcpy(EspnowMsg.lightCmd.duty,duty,sizeof(uint32)*channelNum);
    EspnowMsg.lightCmd.cmd_code = code;
    EspnowMsg.lightCmd.batteryStat.battery_voltage_mv=user_GetBatteryVoltageMv();
    if (EspnowMsg.lightCmd.batteryStat.battery_voltage_mv==0) {
        EspnowMsg.lightCmd.batteryStat.battery_status=ACT_BAT_NA;
    } else if (EspnowMsg.lightCmd.batteryStat.battery_voltage_mv<BAT_EMPTY_MV) {
        EspnowMsg.lightCmd.batteryStat.battery_status=ACT_BAT_EMPTY;
    } else {
        EspnowMsg.lightCmd.batteryStat.battery_status=ACT_BAT_OK;
    }
    light_EspnowSetCsum(&EspnowMsg);
    
    //test
    os_printf("***********************\r\n");
    os_printf("EspnowMsg.lightCmd.cmd_code : %d \r\n",EspnowMsg.lightCmd.cmd_code);
    
#if ACT_DEBUG
    ESPNOW_DBG("send to :\r\n");
	ESPNOW_DBG("MAC: "MACSTR"\r\n",MAC2STR(PairedDev.PairedList[idx].mac_t));
    int j;
    for(j=0;j<sizeof(EspnowMsg);j++) ESPNOW_DBG("%02x ",*((uint8*)(&EspnowMsg)+j));
    ESPNOW_DBG("\r\n");
#endif
    os_memcpy(  &(actionReqStatus[idx].EspnowMsg), &EspnowMsg, sizeof(EspnowProtoMsg));

    int res = esp_now_send((uint8*)PairedDev.PairedList[idx].mac_t, (uint8*)&EspnowMsg, sizeof(EspnowProtoMsg));
	os_printf("ESPNOW SEND RES: %d \r\n",res);
    
    //os_timer_arm( &actionReqStatus[idx].req_timer, actionReqStatus[idx].retry_expire,0);
}



void ICACHE_FLASH_ATTR 
switch_EspnowSendChnSync(uint8 channel)
{
    ESPNOW_DBG("SYNC AT CHANNEL %d \r\n",channel);
    wifi_set_channel(channel);
    //ESPNOW_DBG("TEST SIZEOF actionReqStatus: %d \r\n",sizeof(actionReqStatus));
    int idx;
    bool skip_flg = true;
    for(idx=0;idx<PairedDev.PairedNum;idx++){
        if(actionReqStatus[idx].wifichannel == 0){
            skip_flg = false;
            os_timer_disarm(&actionReqStatus[idx].req_timer); //disarm retry timer;
            actionReqStatus[idx].sequence+=1 ;//send another seq of cmd
            actionReqStatus[idx].status= ACT_REQ;
            actionReqStatus[idx].retry_num = 0;
            
            EspnowProtoMsg EspnowMsg;
            EspnowMsg.csum = 0;
            EspnowMsg.type = ACT_TYPE_SYNC;
            EspnowMsg.token= os_random();
            EspnowMsg.cmd_index = idx;
            EspnowMsg.wifiChannel = wifi_get_channel();
            EspnowMsg.sequence = actionReqStatus[idx].sequence;
			EspnowMsg.io_val = switch_gpio_val;
			EspnowMsg.rsp_if = 1;
			
            EspnowMsg.lightSync.switchChannel=wifi_get_channel();
            EspnowMsg.lightSync.lightChannel = 0;
            EspnowMsg.lightSync.batteryStat.battery_voltage_mv=user_GetBatteryVoltageMv();
            if (EspnowMsg.lightCmd.batteryStat.battery_voltage_mv==0) {
                EspnowMsg.lightCmd.batteryStat.battery_status=ACT_BAT_NA;
            } else if (EspnowMsg.lightCmd.batteryStat.battery_voltage_mv<BAT_EMPTY_MV) {
                EspnowMsg.lightCmd.batteryStat.battery_status=ACT_BAT_EMPTY;
            } else {
                EspnowMsg.lightCmd.batteryStat.battery_status=ACT_BAT_OK;
            }
            light_EspnowSetCsum(&EspnowMsg);
            
        #if ACT_DEBUG
            ESPNOW_DBG("send to :\r\n");
            //ESPNOW_DBG("MAC: %02X %02X %02X %02X %02X %02X\r\n",LIGHT_MAC[idx][0],LIGHT_MAC[idx][1],LIGHT_MAC[idx][2],
            //LIGHT_MAC[idx][3],LIGHT_MAC[idx][4],LIGHT_MAC[idx][5]);
			ESPNOW_DBG("MAC: "MACSTR"\r\n",MAC2STR(PairedDev.PairedList[idx].mac_t));
            int j;
            for(j=0;j<sizeof(EspnowProtoMsg);j++) ESPNOW_DBG("%02x ",*((uint8*)(&EspnowMsg)+j));
            ESPNOW_DBG("\r\n");
        #endif
            os_memcpy(  &(actionReqStatus[idx].EspnowMsg), &EspnowMsg, sizeof(EspnowProtoMsg));
            esp_now_send((uint8*)PairedDev.PairedList[idx].mac_t, (uint8*)&EspnowMsg, sizeof(EspnowProtoMsg));
            //os_timer_arm( &actionReqStatus[idx].req_timer, actionReqStatus[idx].retry_expire,0);
        }
    }
    if(skip_flg){
        switch_EspnowSyncCb();
    }
}


void ICACHE_FLASH_ATTR
switch_EspnowChnSyncStart()
{
    int i;
    for(i=0;i<LIGHT_DEV_NUM;i++){
        actionReqStatus[i].wifichannel = 0;
    }
    channel_cur = 1;
    switch_EspnowSendChnSync(channel_cur);
}

void ICACHE_FLASH_ATTR 
switch_EspnowRcvCb(u8 *macaddr, u8 *data, u8 len)
{
    int i;
#if ACT_DEBUG
    ESPNOW_DBG("recv mac : \r\n");
    for(i = 0; i<6;i++){
        ESPNOW_DBG("%02x ",macaddr[i]);
    }
    ESPNOW_DBG("\r\n");
    ESPNOW_DBG("recv data: ");
    for (i = 0; i < len; i++)
    ESPNOW_DBG("%02X, ", data[i]);
    ESPNOW_DBG("\n");
#endif
    
    EspnowProtoMsg EspnowMsg;
    os_memcpy( (uint8*)(&EspnowMsg),data,len);
    if(light_EspnowCmdValidate(&EspnowMsg) ){
        ESPNOW_DBG("cmd check sum OK\r\n");
        uint32 _idx=EspnowMsg.cmd_index;
        if(0 == os_memcmp(macaddr+1, (uint8*)(PairedDev.PairedList[_idx].mac_t)+1,sizeof(PairedDev.PairedList[_idx].mac_t)-1)){
            ESPNOW_DBG("switch MAC match...\r\n");
            if(EspnowMsg.sequence == actionReqStatus[_idx].sequence && EspnowMsg.type == ACT_TYPE_RSP ){  
                //ACT WOULD NOT HAPPEN AFTER WE USE MAC LAYER ACK
                actionReqStatus[_idx].status = ACT_RSP;
				
				#if ACTION_CMD_RSP
				ESPNOW_DBG("cmd %d ack \r\n",_idx);
				ESPNOW_DBG("cmd channel : %d \r\n",EspnowMsg.wifiChannel);
				ESPNOW_DBG("SELF CHANNEL: %d \r\n",wifi_get_channel());
				switch_EspnowAckCb();
				#else
                ESPNOW_DBG("CMD RESPONSE %d  \r\n",_idx);
                ESPNOW_DBG("DO NOTHING...\r\n");
                ESPNOW_DBG("CMD CHANNEL : %d \r\n",EspnowMsg.wifiChannel);
                ESPNOW_DBG("SELF CHANNEL: %d \r\n",wifi_get_channel());
				#endif
            }else if(EspnowMsg.sequence == actionReqStatus[_idx].sequence && EspnowMsg.type == ACT_TYPE_SYNC){
                actionReqStatus[_idx].status = ACT_RSP;
                if(wifi_get_channel()==EspnowMsg.wifiChannel){
                    actionReqStatus[_idx].wifichannel = EspnowMsg.wifiChannel;
                    ESPNOW_DBG("cmd %d sync,@ CHANNEL %d \r\n",_idx,actionReqStatus[_idx].wifichannel);
                    os_timer_disarm(&actionReqStatus[_idx].req_timer);
                    switch_EspnowSyncCb();
                }else{
                    ESPNOW_DBG("MESH SYNC CHANNEL ERROR, get channel : %d , data_channel : %d\r\n",wifi_get_channel(),EspnowMsg.wifiChannel);
                }
            }else{
                if(EspnowMsg.sequence != actionReqStatus[_idx].sequence) ESPNOW_DBG("seq error\r\n");
                if(EspnowMsg.type != ACT_TYPE_RSP ) ESPNOW_DBG("TYPE MISMATCH: %d \r\n",EspnowMsg.type);
            }
        
        }else{
            ESPNOW_DBG("SOURCE MAC: "MACSTR,MAC2STR(macaddr));
            ESPNOW_DBG("LIGHT RECORD MAC: "MACSTR"\r\n",MAC2STR(PairedDev.PairedList[_idx].mac_t));
            ESPNOW_DBG("LIGHT IDX: %d \r\n",EspnowMsg.cmd_index); 
            ESPNOW_DBG("switch MAC mismatch...\r\n");
        }
    }else{
        ESPNOW_DBG("cmd check sum error\r\n");
    }

}



int ICACHE_FLASH_ATTR
switch_GetLightMacIdx(uint8* mac_addr)
{
    int i;
    uint8 m_l = *(mac_addr+DEV_MAC_LEN-1);
    for(i=0;i<LIGHT_DEV_NUM;i++){
        if( m_l== PairedDev.PairedList[i].mac_t[DEV_MAC_LEN-1]){
            if(0==os_memcmp(mac_addr,PairedDev.PairedList[i].mac_t,DEV_MAC_LEN)){
                return i;
            }
        }
    }
    return -1;
}


void ICACHE_FLASH_ATTR
	esp_now_send_cb(u8 *mac_addr, u8 status)
{
    os_printf("====================\r\n");
    os_printf("ESP-NOW SEND CB\r\n");
    os_printf("--------\r\n");
    os_printf("MAC: "MACSTR"\r\n",MAC2STR(mac_addr));
    os_printf("STATUS: %d \r\n",status);
    os_printf("====================\r\n");

    int mac_idx = switch_GetLightMacIdx(mac_addr) ;
    if(mac_idx < 0 && mac_idx>=PairedDev.PairedNum){
        os_printf("MAC idx error: %d \r\n",mac_idx);
        return;
    }

	EspnowProtoMsg* EspnowRetryMsg = &(actionReqStatus[mac_idx].EspnowMsg);
	Action_SendStatus* EspnowSendStatus = &(actionReqStatus[mac_idx]);

    if(status==0){ //send successful
        EspnowSendStatus->status = ACT_ACK;//ACT NOT RESPONSED YET,FOR CMD ACK IS ENOUGH, FOR SYNC , WAITING FOR RESPONSE
        os_printf("data ACKed...\r\n");
        os_timer_disarm(&EspnowSendStatus->req_timer);
        if(EspnowRetryMsg->type==ACT_TYPE_DATA){
        #if ACTION_CMD_RSP
			os_printf("go to set retry:\r\n");
            goto SET_RETRY;
		#else
            switch_EspnowAckCb();
		#endif
        }else if(EspnowRetryMsg->type==ACT_TYPE_SYNC){
            os_printf("go to set retry:\r\n");
            goto SET_RETRY;
        }
    
    }else{ //send fail
SET_RETRY:
    os_timer_disarm(&EspnowSendStatus->req_timer);
    if(EspnowSendStatus->retry_num < ACTION_RETRY_NUM){
        os_printf("data[%d] send failed...retry:%d\r\n",mac_idx,EspnowSendStatus->retry_num);
        os_timer_arm( &EspnowSendStatus->req_timer, EspnowSendStatus->retry_expire,0);
    }else{
        ESPNOW_DBG("retry num exceed..stop retry, type: %d\r\n",EspnowRetryMsg->type);
        EspnowSendStatus->status = ACT_TIME_OUT;        
        if(EspnowRetryMsg->type==ACT_TYPE_SYNC){
            switch_EspnowSyncCb();
        }else if(EspnowRetryMsg->type==ACT_TYPE_DATA){
            switch_EspnowAckCb();
        }
    }
    }
}





#if 0
//load espnow mac list from flash

#define SWITCH_SLAVE_MAC_LIST_ADDR
void ICACHE_FLASH_ATTR switch_EspnowSlaveMacInit()
{
	system_param_load(ESPNOW_PARAM_SEC, 0, &switchEspnowParam, sizeof(switchEspnowParam));

	int i;
	//debug
    ESPNOW_DBG("switchEspnowParam.SlveNum: %d \r\n",switchEspnowParam.SlaveNum);
	
	for(i=0;i<LIGHT_DEV_NUM;i++){
		ESPNOW_DBG("MAC[%d]:"MACSTR"\r\n",i,MAC2STR(switchEspnowParam.SlaveMac[i]));
	}
    ESPNOW_DBG("switchEspnowParam.esp_now_key: \r\n");
	for(i=0;i<ESPNOW_KEY_LEN;i++) ESPNOW_DBG("%02X ",switchEspnowParam.esp_now_key[i]);
	os_printf("\r\n");
	os_printf("switchEspnowParam.wifiChannel:\r\n");
	for(i=0;i<LIGHT_DEV_NUM;i++) ESPNOW_DBG("%02X ",switchEspnowParam.wifiChannel[i]);
	os_printf("\r\n");
	ESPNOW_DBG("magic: 0x%08x \r\n\n\n\n",switchEspnowParam.magic);
	//end of debug

	if(switchEspnowParam.magic == ESPNOW_PARAM_MAGIC){
		ESPNOW_DBG("ESPNOW FLASH DATA OK...\r\n");
    	for(i=0;i<switchEspnowParam.SlaveNum;i++){
    		ESPNOW_DBG("MAC[%d]:"MACSTR"\r\n",i,MAC2STR(switchEspnowParam.SlaveMac[i]));
    	}
	}else{
		ESPNOW_DBG("ESPNOW FLASH DATA ERROR...\r\n");
		os_memset(switchEspnowParam.esp_now_key,0,sizeof(switchEspnowParam.esp_now_key));
		switchEspnowParam.SlaveNum = 0;
		switchEspnowParam.magic = ESPNOW_PARAM_MAGIC;
		system_param_save_with_protect(ESPNOW_PARAM_SEC, &switchEspnowParam, sizeof(switchEspnowParam));
	}
	

}
#endif



void ICACHE_FLASH_ATTR switch_EspnowInit()
{
    uint8 i;
    int e_res;
    if (esp_now_init()==0) {
        os_printf("direct link  init ok\n");
        esp_now_register_recv_cb(switch_EspnowRcvCb);
    } else {
        os_printf("dl init failed\n");
    }
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);  //role 1: switch   ;  role 2 : light;
    esp_now_register_send_cb(esp_now_send_cb);
    //system_param_load(ESPNOW_PARAM_SEC,0,&switchEspnowParam,sizeof(switchEspnowParam));
    ESPNOW_DBG("==============================\r\n");
	ESPNOW_DBG("ADD ESPNOW MAC LIST INIT...\r\n");
    //switch_EspnowSlaveMacInit();
	sp_MacInit();
    ESPNOW_DBG("==============================\r\n");
	

		
    ESPNOW_DBG("MAGIC: %08x\r\n",PairedDev.magic);
	
#if ESPNOW_KEY_HASH
	esp_now_set_kok(PairedDev.PairedList[0].key_t, ESPNOW_KEY_LEN);
#endif

    for(i=0;i<PairedDev.PairedNum;i++){
        
    #if ESPNOW_ENCRYPT
        e_res = esp_now_add_peer((uint8*)(PairedDev.PairedList[i].mac_t), (uint8)ESP_NOW_ROLE_SLAVE,(uint8)WIFI_DEFAULT_CHANNEL, PairedDev.PairedList[i].key_t, (uint8)ESPNOW_KEY_LEN);//wjl
        if(e_res){
            os_printf("ADD PEER ERROR!!!!!MAX NUM!!!!!\r\n");
            return;
        }else{
			os_printf("ADD PEER OK: MAC[%d]:"MACSTR"\r\n",i,MAC2STR(((uint8*)(PairedDev.PairedList[i].mac_t))));
        }
    #else
        esp_now_add_peer((uint8*)(PairedDev.PairedList[i].mac_t), (uint8)ESP_NOW_ROLE_SLAVE,(uint8)WIFI_DEFAULT_CHANNEL, NULL, (uint8)ESPNOW_KEY_LEN);//wjl
    #endif
        actionReqStatus[i].actionToutCb = (ActionToutCallback)switch_EspnowSendRetry;
        
        os_memset(&(actionReqStatus[i].EspnowMsg),0,sizeof(EspnowProtoMsg));
        os_timer_disarm(&actionReqStatus[i].req_timer);
        os_timer_setfn(&actionReqStatus[i].req_timer,  actionReqStatus[i].actionToutCb , &(actionReqStatus[i].EspnowMsg.cmd_index)  );
        actionReqStatus[i].wifichannel = ( (PairedDev.magic==SP_PARAM_MAGIC)?(PairedDev.PairedList[i].channel_t):WIFI_DEFAULT_CHANNEL);
        ESPNOW_DBG("LIGHT %d : channel %d \r\n",i,actionReqStatus[i].wifichannel);
        actionReqStatus[i].retry_num=0;
        actionReqStatus[i].sequence =0;
        actionReqStatus[i].status = ACT_IDLE;
        actionReqStatus[i].retry_expire = ACTION_RETRY_TIMER_MS;  //300ms retry
    }
}

void ICACHE_FLASH_ATTR switch_EspnowDeinit()
{
    esp_now_unregister_recv_cb(); 
    esp_now_deinit();
}

#endif

