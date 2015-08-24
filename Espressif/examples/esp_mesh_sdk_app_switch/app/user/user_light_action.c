
#include "os_type.h"
#include "eagle_soc.h"
#include "c_types.h"
#include "osapi.h"
#include "ets_sys.h"
#include "mem.h"
#include "user_light_action.h"
#include "user_switch.h"
#include "user_interface.h"
#include "user_configstore.h"
#include "espnow.h"

//The espnow packet struct used for light<->switch intercommunications
typedef struct {
	uint8 source_mac[6];
	uint16 wifiChannel;
	uint32 cmd_index;
	uint32 sequence;
	uint16 type;
	uint16 pwm_num;
	uint32 period;
	uint32 duty[8];
	uint16 battery_voltage_mv;
	uint16 battery_status;
	uint32 check_sum;
} LightActionCmd;

typedef enum{
	ACT_IDLE = 0,
	ACT_ACK = 1,
	ACT_REQ = 2,
	ACT_TIME_OUT =3,
}ActionStatus;

typedef enum{
	ACT_DATA = 0,
	ACT_TYPE_ACK = 1,
	ACT_TYPE_SYNC = 2,
}ACTION_TYPE;


typedef enum {
	ACT_BAT_NA = 0,
	ACT_BAT_OK = 1,
	ACT_BAT_EMPTY = 2
}ACT_BAT_TYPE;

#define BAT_EMPTY_MV 2100

#define ACT_DEBUG 1
#define ACT_PRINT os_printf
#define WIFI_DEFAULT_CHANNEL 1

/*right now , all the MAC address is hard coded and can not be changed */
/*One controller is bind to the certain groups of MAC address.*/
/*next , we will add a encryption for ESP-NOW.*/
/* and send broadcast instead on unicast.*/
#define ESPNOW_ENCRYPT  1

#define ESPNOW_KEY_LEN 16
uint8 esp_now_key[ESPNOW_KEY_LEN] = {0x10,0xfe,0x94, 0x7c,0xe6,0xec,0x19,0xef,0x33, 0x9c,0xe6,0xdc,0xa8,0xff,0x94, 0x7d};//key

const uint8 SWITCH_MAC[6] = {0x18,0xfe,0x34, 0xa5,0x3d,0x68};
const uint8 LIGHT_MAC[LIGHT_DEV_NUM][6] = {
	                                                 {0x1a,0xfe,0x34, 0xa1,0x32,0xaa},											   
												     {0x1a,0xfe,0x34, 0x9a,0xa3,0xcd},
	                                                 {0x1a,0xfe,0x34, 0xa0,0xd6,0x97},
	                                                 {0x1a,0xfe,0x34, 0x9f,0xf3,0x68},	                                                 
	                                                 {0x1a,0xfe,0x34, 0xa0,0xac,0x19},
	                                                 {0x1a,0xfe,0x34, 0xa1,0x08,0x25},
	                                                 {0x1a,0xfe,0x34, 0xa1,0x09,0x3a},
	                                                 {0x1a,0xfe,0x34, 0xa1,0x06,0x66},												
	                                                 {0x1a,0xfe,0x34, 0xa1,0x07,0x47},
	                                                 {0x1a,0xfe,0x34, 0xa1,0x08,0x1a}
                                            };





bool ICACHE_FLASH_ATTR
	light_EspnowCmdValidate(LightActionCmd* light_act_data)
{
	uint8* data = (uint8*)light_act_data;
	uint32 csum_cal = 0;
	int i;
	for(i=0;i<(sizeof(LightActionCmd)-4);i++){
		csum_cal+= *(data+i);
	}
	if(csum_cal == light_act_data->check_sum){
	    return true;
	}else{
	    return false;
	}
}

void ICACHE_FLASH_ATTR
	light_EspnowSetCsum(LightActionCmd* light_act_data)
{
	uint8* data = (uint8*)light_act_data;
	uint32 csum_cal = 0;
	int i;
	for(i=0;i<(sizeof(LightActionCmd)-4);i++){
		csum_cal+= *(data+i);
	}
	light_act_data->check_sum = csum_cal;
}


#define ACTION_RETRY_NUM  3
#define ACTION_RETRY_TIMER_MS  200
typedef void (*ActionToutCallback)(uint32* act_idx);


typedef struct{
	uint32 sequence;
	ActionStatus status;
	os_timer_t req_timer;
	uint32 retry_num;
	uint32 retry_expire;
	ActionToutCallback actionToutCb;
	LightActionCmd lightActionCmd;
	uint16 wifichannel;
} Action_SendStatus;



os_timer_t action_retry_t;
Action_SendStatus actionReqStatus[LIGHT_DEV_NUM] = {0};

LOCAL uint8 channel_group[LIGHT_DEV_NUM];
LOCAL uint8 channel_num = 0;
LOCAL uint8 channel_cur = 1;
LOCAL uint8 pwm_chn_num = 5;
LOCAL uint32 pwm_duty[8];
LOCAL uint32 pwm_period;

bool ICACHE_FLASH_ATTR
	switch_CheckCmdResult()
{
	int i;
	for(i=0;i<channel_num;i++){
	    if(actionReqStatus[channel_group[i]].status == ACT_REQ) return false;
	}
	return true;
}


void ICACHE_FLASH_ATTR
	switch_EspnowAckCb()
{
	if( switch_CheckCmdResult() ){
		if(channel_cur == 14){
			ACT_PRINT("release power\r\n");
			_SWITCH_GPIO_RELEASE();
		}else{
			ACT_PRINT("SEND NEXT CHANNEL: %d \r\n",++channel_cur);
			switch_EspnowSendCmdByChnl(channel_cur,pwm_chn_num, pwm_duty, pwm_period);
		}
	}
}

bool ICACHE_FLASH_ATTR
	switch_CheckSyncResult()
{
	int i;
	for(i=0;i<CMD_NUM;i++){ //CMD NUM EQUALS PRACTICAL LIGHT NUMBER
	    if(actionReqStatus[i].status == ACT_REQ ) return false;
	}
	return true;
}


void ICACHE_FLASH_ATTR
	switch_EspnowSyncCb()
{
	if( switch_CheckSyncResult() ){
		os_printf("SYNC FINISHED ...\r\n");
		#if 0
		UART_WaitTxFifoEmpty(0,100000);
		_SWITCH_GPIO_RELEASE();
		#else
		if(channel_cur == 14){
			ACT_PRINT("release power\r\n");

			int i;
			for(i=0;i<CMD_NUM;i++){
				myConfig.wlanChannel[i]=actionReqStatus[i].wifichannel;
			}
			configSave();

			initAPMode();
//			_SWITCH_GPIO_RELEASE();
		}else{
			ACT_PRINT("SYNC NEXT CHANNEL: %d \r\n",++channel_cur);
			//switch_EspnowSendCmdByChnl(channel_cur,pwm_chn_num, pwm_duty, pwm_period);
			switch_EspnowSendChnSync(channel_cur);
		}
		#endif

	}

}




 void ICACHE_FLASH_ATTR
 	switch_EspnowSendRetry(void* arg)
{
	uint32 _idx = *((uint32*)arg);
	LightActionCmd* action_retry_cmd = &(actionReqStatus[_idx].lightActionCmd);
	Action_SendStatus* action_status = &(actionReqStatus[_idx]);
	
	if((action_retry_cmd->sequence == action_status->sequence)){
	    if(action_status->status== ACT_REQ){
	        esp_now_send((uint8*)LIGHT_MAC[_idx], (uint8*)action_retry_cmd, sizeof(LightActionCmd));
	        action_status->retry_num++;
			if(action_status->retry_num < ACTION_RETRY_NUM){
	            os_timer_arm( &action_status->req_timer, action_status->retry_expire,0);
			}else{
	            ACT_PRINT("retry num exceed..stop retry, cmd type: %d\r\n",action_retry_cmd->type);
				action_status->status = ACT_TIME_OUT;
				
				if(action_retry_cmd->type==ACT_TYPE_SYNC){
					switch_EspnowSyncCb();
				}else if(action_retry_cmd->type==ACT_DATA){
					switch_EspnowAckCb();
				}
			}
		}else{
	        ACT_PRINT("STATUS error : %d\r\n",action_status->status);
		}
	    
	}else if(action_retry_cmd->sequence   <   action_status->sequence){
	    ACT_PRINT("action updated...,cancel retry ...\r\n");
	}


}

void ICACHE_FLASH_ATTR switch_EspnowSendCmdByChnl(uint8 chn,uint32 channelNum, uint32* duty, uint32 period)
{
	int i = 0;
	os_memset(channel_group, 0 ,sizeof(channel_group));
	channel_num = 0;
	channel_cur = chn;
	
	pwm_period = period;
	os_memcpy(pwm_duty,duty,sizeof(pwm_duty));
	pwm_chn_num = channelNum;
	
	
	for(i=0;i<CMD_NUM;i++){
		if(actionReqStatus[i].wifichannel == chn){
			channel_group[channel_num++]=i;
			ACT_PRINT("CHANNEL %d : add idx %d\r\n",chn,i);
		}
	}

	if(channel_num>0){
		ACT_PRINT("WIFI SET CHANNEL : %d \r\n",channel_cur);
		wifi_set_channel(channel_cur);
		ACT_PRINT("WIFI GET CHANNEL : %d \r\n",wifi_get_channel());

		for(i=0;i<channel_num;i++){
			switch_EspnowSendLightCmd(channel_group[i], channelNum, duty, period);
			
		}
	}else{
		switch_EspnowAckCb();//next channel;
	}
}


extern uint32 user_GetBatteryVoltageMv();

void ICACHE_FLASH_ATTR switch_EspnowSendLightCmd(uint8 idx, uint32 channelNum, uint32* duty, uint32 period)
{
	os_timer_disarm(&actionReqStatus[idx].req_timer); //disarm retry timer;
	actionReqStatus[idx].sequence+=1 ;//send another seq of cmd
	actionReqStatus[idx].status= ACT_REQ;
	actionReqStatus[idx].lightActionCmd.cmd_index = idx;
	actionReqStatus[idx].retry_num = 0;
	
	LightActionCmd light_cmd;
	light_cmd.cmd_index = idx;
	light_cmd.sequence = actionReqStatus[idx].sequence; //send another seq of cmd
	light_cmd.type = ACT_DATA;
	light_cmd.pwm_num = channelNum;
	light_cmd.wifiChannel = wifi_get_channel();
	light_cmd.battery_voltage_mv=user_GetBatteryVoltageMv();
	if (light_cmd.battery_voltage_mv==0) {
		light_cmd.battery_status=ACT_BAT_NA;
	} else if (light_cmd.battery_voltage_mv<BAT_EMPTY_MV) {
		light_cmd.battery_status=ACT_BAT_EMPTY;
	} else {
		light_cmd.battery_status=ACT_BAT_OK;
	}
	os_memcpy(light_cmd.duty,duty,sizeof(uint32)*channelNum);
	uint8 mac_buf[6] = {0};
	wifi_get_macaddr(STATION_IF,mac_buf);
	os_printf("source mac: %02x %02x %02x %02x %02x %02x\r\n",mac_buf[0],mac_buf[1],mac_buf[2],mac_buf[3],mac_buf[4],mac_buf[5]);
	os_memcpy(light_cmd.source_mac,mac_buf,sizeof(mac_buf));

	light_cmd.period = period;
	light_EspnowSetCsum(&light_cmd);

	#if ACT_DEBUG
	ACT_PRINT("send to :\r\n");
	ACT_PRINT("MAC: %02X %02X %02X %02X %02X %02X\r\n",LIGHT_MAC[idx][0],LIGHT_MAC[idx][1],LIGHT_MAC[idx][2],
			                                           LIGHT_MAC[idx][3],LIGHT_MAC[idx][4],LIGHT_MAC[idx][5]);
	int j;
	for(j=0;j<sizeof(LightActionCmd);j++) ACT_PRINT("%02x ",*((uint8*)(&light_cmd)+j));
	ACT_PRINT("\r\n");
	#endif
	esp_now_send((uint8*)LIGHT_MAC[idx], (uint8*)&light_cmd, sizeof(LightActionCmd));

	os_memcpy(  &(actionReqStatus[idx].lightActionCmd), &light_cmd, sizeof(LightActionCmd));
	os_timer_arm( &actionReqStatus[idx].req_timer, actionReqStatus[idx].retry_expire,0);
	
}

void ICACHE_FLASH_ATTR 
	switch_EspnowSendChnSync(uint8 channel)
{
	ACT_PRINT("SYNC AT CHANNEL %d \r\n",channel);
	wifi_set_channel(channel);
	ACT_PRINT("TEST SIZEOF actionReqStatus: %d \r\n",sizeof(actionReqStatus));
	int idx;
	bool skip_flg = true;
	for(idx=0;idx<CMD_NUM;idx++){
		if(actionReqStatus[idx].wifichannel == 0){
			skip_flg = false;
	        os_timer_disarm(&actionReqStatus[idx].req_timer); //disarm retry timer;
	        actionReqStatus[idx].sequence+=1 ;//send another seq of cmd
	        actionReqStatus[idx].status= ACT_REQ;
	        actionReqStatus[idx].lightActionCmd.cmd_index = idx;
	        actionReqStatus[idx].retry_num = 0;
	    
	        LightActionCmd light_cmd;
	        light_cmd.cmd_index = idx;
	        light_cmd.sequence = actionReqStatus[idx].sequence; //send another seq of cmd
	        light_cmd.type = ACT_TYPE_SYNC;
	    	light_cmd.wifiChannel= wifi_get_channel();
			ACT_PRINT("CMD SEND CHANNLE: %d \r\n",light_cmd.wifiChannel);
	   	
	    	uint8 mac_buf[6] = {0};
	    	wifi_get_macaddr(STATION_IF,mac_buf);
	    	os_printf("source mac: %02x %02x %02x %02x %02x %02x\r\n",mac_buf[0],mac_buf[1],mac_buf[2],mac_buf[3],mac_buf[4],mac_buf[5]);
	        os_memcpy(light_cmd.source_mac,mac_buf,sizeof(mac_buf));
	        light_EspnowSetCsum(&light_cmd);
	    
	        #if ACT_DEBUG
	    	ACT_PRINT("send to :\r\n");
	    	ACT_PRINT("MAC: %02X %02X %02X %02X %02X %02X\r\n",LIGHT_MAC[idx][0],LIGHT_MAC[idx][1],LIGHT_MAC[idx][2],
	    		                                               LIGHT_MAC[idx][3],LIGHT_MAC[idx][4],LIGHT_MAC[idx][5]);
	    	int j;
	        for(j=0;j<sizeof(LightActionCmd);j++) ACT_PRINT("%02x ",*((uint8*)(&light_cmd)+j));
	    	ACT_PRINT("\r\n");
	        #endif
	        esp_now_send((uint8*)LIGHT_MAC[idx], (uint8*)&light_cmd, sizeof(LightActionCmd));
	        os_memcpy(  &(actionReqStatus[idx].lightActionCmd), &light_cmd, sizeof(LightActionCmd));
	        os_timer_arm( &actionReqStatus[idx].req_timer, actionReqStatus[idx].retry_expire,0);
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

void ICACHE_FLASH_ATTR switch_EspnowRcvCb(u8 *macaddr, u8 *data, u8 len)
{
	int i;
	#if ACT_DEBUG

	ACT_PRINT("recv mac : \r\n");
	for(i = 0; i<6;i++){
	        ACT_PRINT("%02x ",macaddr[i]);
	}
	ACT_PRINT("\r\n");
	

	ACT_PRINT("recv data: ");
	for (i = 0; i < len; i++)
		ACT_PRINT("%02X, ", data[i]);
	ACT_PRINT("\n");
	#endif

	LightActionCmd light_data ;
	os_memcpy( (uint8*)(&light_data),data, len);
	uint32 light_index=light_data.cmd_index;


	if(light_EspnowCmdValidate(&light_data) ){
		ACT_PRINT("cmd check sum OK\r\n");

	   if(0 == os_memcmp(light_data.source_mac+1, LIGHT_MAC[light_index]+1,sizeof(SWITCH_MAC)-1)){
	           ACT_PRINT("switch MAC match...\r\n");
	   	uint32 _idx = light_data.cmd_index;
	   	if(light_data.sequence == actionReqStatus[_idx].sequence && light_data.type == ACT_TYPE_ACK ){
	   		actionReqStatus[_idx].status = ACT_ACK;
	   		ACT_PRINT("cmd %d ack \r\n",_idx);
			ACT_PRINT("cmd channel : %d \r\n",light_data.wifiChannel);
			ACT_PRINT("SELF CHANNEL: %d \r\n",wifi_get_channel());
			switch_EspnowAckCb();
	   	}else if(light_data.sequence == actionReqStatus[_idx].sequence && light_data.type == ACT_TYPE_SYNC){
			actionReqStatus[_idx].status = ACT_ACK;
			if(wifi_get_channel()==light_data.wifiChannel){
				actionReqStatus[_idx].wifichannel = light_data.wifiChannel;
	   		    ACT_PRINT("cmd %d sync,@ CHANNEL %d \r\n",_idx,actionReqStatus[_idx].wifichannel);
				switch_EspnowSyncCb();
			}else{
				ACT_PRINT("MESH SYNC CHANNEL ERROR, get channel : %d , data_channel : %d\r\n",wifi_get_channel,light_data.wifiChannel);
			}
		}else{
	        if(light_data.sequence != actionReqStatus[_idx].sequence) ACT_PRINT("seq error\r\n");
	   	    if(light_data.type != ACT_TYPE_ACK ) ACT_PRINT("already ack \r\n");
	   	}
	   
	       }else{
				ACT_PRINT("SOURCE MAC: "MACSTR,MAC2STR(light_data.source_mac));
				ACT_PRINT("LIGHT RECORD MAC: "MACSTR,MAC2STR(LIGHT_MAC[light_index]));
				ACT_PRINT("LIGHT IDX: %d \r\n",light_data.cmd_index); 
			   ACT_PRINT("switch MAC mismatch...\r\n");
	    	}
	}else{
		ACT_PRINT("cmd check sum error\r\n");
	}

}

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
	esp_now_set_kok(esp_now_key, ESPNOW_KEY_LEN);


	for(i=0;i<CMD_NUM;i++){
		#if ESPNOW_ENCRYPT
    		e_res = esp_now_add_peer((uint8*)LIGHT_MAC[i], (uint8)ESP_NOW_ROLE_SLAVE,(uint8)WIFI_DEFAULT_CHANNEL, esp_now_key, (uint8)ESPNOW_KEY_LEN);//wjl
			if(e_res){
				os_printf("ERROR!!!!!!!!!!\r\n");
				return;
			}
		#else
    		esp_now_add_peer((uint8*)LIGHT_MAC[i], (uint8)ESP_NOW_ROLE_SLAVE,(uint8)WIFI_DEFAULT_CHANNEL, NULL, (uint8)ESPNOW_KEY_LEN);//wjl
		#endif
	    actionReqStatus[i].actionToutCb = (ActionToutCallback)switch_EspnowSendRetry;
	    
	    os_memset(&(actionReqStatus[i].lightActionCmd),0,sizeof(LightActionCmd));
	    os_timer_disarm(&actionReqStatus[i].req_timer);
	    os_timer_setfn(&actionReqStatus[i].req_timer,  actionReqStatus[i].actionToutCb , &(actionReqStatus[i].lightActionCmd.cmd_index)  );
	    
	    actionReqStatus[i].wifichannel = myConfig.wlanChannel[i];
	    ACT_PRINT("LIGHT %d : channel %d \r\n",i,actionReqStatus[i].wifichannel);
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

