#include "os_type.h"
#include "eagle_soc.h"
#include "c_types.h"
#include "osapi.h"
#include "ets_sys.h"
#include "mem.h"
#include "user_light_action.h"
#include "user_light_adj.h"
#include "user_light.h"
#include "user_interface.h"
#include "espnow.h"
#include "user_esp_platform.h"
#include "spi_flash.h"

#if ESP_NOW_SUPPORT
//The espnow packet struct used for light<->switch intercommunications
typedef struct{
    uint16 battery_voltage_mv;
    uint16 battery_status;
}EspnowBatStatus;

typedef enum{
    COLOR_SET = 0,
    COLOR_CHG ,
    COLOR_TOGGLE,
    COLOR_LEVEL,
    LIGHT_RESET,
    LIGHT_TIMER,
}EspnowCmdCode;


typedef struct {
	uint16 pwm_num;
	uint32 period;
	uint32 duty[8];
	uint32 cmd_code;
    EspnowBatStatus batteryStat;
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
    ACT_RSP =4,
}EspnowMsgStatus;

typedef enum{
    ACT_TYPE_DATA = 0,
    ACT_TYPE_ACK = 1,//NOT USED, REPLACED BY MAC ACK
    ACT_TYPE_SYNC = 2,
    ACT_TYPE_RSP = 3,
    ACT_TYPE_PAIR_REQ = 4,
    ACT_TYPE_PAIR_ACK = 5,
}EspnowMsgType;


typedef enum {
	ACT_BAT_NA = 0,
	ACT_BAT_OK = 1,
	ACT_BAT_EMPTY = 2
}ACT_BAT_TYPE;

typedef struct{
    uint8 csum;
    uint8 type;
    uint32 token;//for encryption, add random number
    uint16 cmd_index;
	uint8 wifiChannel;
	uint16 sequence;
	uint16 io_val;
	uint8 rsp_if;

    union{
    EspnowLightCmd lightCmd;
    EspnowLightRsp lightRsp;
    EspnowLightSync lightSync;
    };
}EspnowProtoMsg;

typedef struct {
	char mac[6];
	int status;
	int battery_mv;
} SwitchBatteryStatus;

//Just save the state of one switch for now.
SwitchBatteryStatus switchBattery;

#define ACT_DEBUG 1

bool ICACHE_FLASH_ATTR
light_EspnowCmdValidate(EspnowProtoMsg* EspnowMsg)
{
    uint8* data = (uint8*)EspnowMsg;
    uint8 csum_cal = 0;
    int i;
    for(i=1;i<sizeof(EspnowProtoMsg);i++){
        csum_cal+= *(data+i);
    }
	
	ESPNOW_DBG("csum cal: %d ; csum : %d \r\n",csum_cal,EspnowMsg->csum);
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
	
	ESPNOW_DBG("csum cal: %d ; csum : %d \r\n",csum_cal,EspnowMsg->csum);
}



static void ICACHE_FLASH_ATTR light_EspnowStoreBatteryStatus(char *mac, int status, int voltage_mv) {
    os_memcpy(switchBattery.mac, mac, 6);
    switchBattery.status=status;
    switchBattery.battery_mv=voltage_mv;
}

//ToDo: add support for more switches
int ICACHE_FLASH_ATTR light_EspnowGetBatteryStatus(int idx, char *mac, int *status, int *voltage_mv) {
    if (idx>0) return 0;
    os_memcpy(mac, switchBattery.mac, 6);
    *status=switchBattery.status;
    *voltage_mv=switchBattery.battery_mv;
}


#if LIGHT_DEVICE

#if 0
#define ESPNOW_PARAM_MAGIC 0x5c5caacc
typedef struct{
    //uint8 csum;
    uint32 magic;
    uint16 MasterNum;
    uint8 MasterMac[SWITCH_DEV_NUM][DEV_MAC_LEN];
    uint8 esp_now_key[ESPNOW_KEY_LEN];
	
}LightEspnowParam;

LightEspnowParam lightEspnowParam;
//load mac list from flash
//load ESPNOW key from flash
void ICACHE_FLASH_ATTR light_EspnowMasterMacInit()
{

#if 1
	system_param_load(LIGHT_MASTER_MAC_LIST_ADDR, 0, &lightEspnowParam, sizeof(lightEspnowParam));

	int i;
	//debug
    ESPNOW_DBG("lightEspnowParam.MasterNum: %d \r\n",lightEspnowParam.MasterNum);
	for(i=0;i<SWITCH_DEV_NUM;i++){
		ESPNOW_DBG("MAC[%d]:"MACSTR"\r\n",i,MAC2STR(lightEspnowParam.MasterMac[i]));
	}
    ESPNOW_DBG("lightEspnowParam.esp_now_key: \r\n");
	for(i=0;i<ESPNOW_KEY_LEN;i++) ESPNOW_DBG("%02X ",lightEspnowParam.esp_now_key[i]);
	os_printf("\r\n");
	ESPNOW_DBG("magic: 0x%08x \r\n\n\n\n",lightEspnowParam.magic);
	//end of debug

	if(lightEspnowParam.magic == ESPNOW_PARAM_MAGIC){
		ESPNOW_DBG("ESPNOW FLASH DATA OK...\r\n");
    	for(i=0;i<lightEspnowParam.MasterNum;i++){
    		ESPNOW_DBG("MAC[%d]:"MACSTR"\r\n",i,MAC2STR(lightEspnowParam.MasterMac[i]));
    	}
	}else{
		ESPNOW_DBG("ESPNOW FLASH DATA ERROR...\r\n");
		os_memset(lightEspnowParam.esp_now_key,0,sizeof(lightEspnowParam.esp_now_key));
		lightEspnowParam.MasterNum = 0;
		lightEspnowParam.magic = ESPNOW_PARAM_MAGIC;
		system_param_save_with_protect(LIGHT_MASTER_MAC_LIST_ADDR, &lightEspnowParam, sizeof(lightEspnowParam));
	}
#else
    uint8 espnow_key_tmp[ESPNOW_KEY_LEN] = {0x10,0xfe,0x94, 0x7c,0xe6,0xec,0x19,0xef,0x33, 0x9c,0xe6,0xdc,0xa8,0xff,0x94, 0x7d};//key
    /*MAC LIST FOR THE CONTROLLER DEVICES,HARD CODED NOW*/
    uint8 switch_mac_tmp[SWITCH_DEV_NUM][DEV_MAC_LEN] = {{0x18,0xfe,0x34, 0x9a,0xb3,0xfe},
    															  {0x18,0xfe,0x34, 0xa5,0x3d,0x68},
    															  {0x18,0xfe,0x34, 0xa5,0x3d,0x66},
    															  {0x18,0xfe,0x34, 0xa5,0x3d,0x7b},
    															  
    															  {0x18,0xfe,0x34, 0xa5,0x3d,0x84}};
    
    
    os_memcpy(lightEspnowParam.esp_now_key , espnow_key_tmp , ESPNOW_KEY_LEN);
    os_memcpy(lightEspnowParam.MasterMac,switch_mac_tmp,sizeof(switch_mac_tmp));
    lightEspnowParam.MasterNum = 5;
    
    
    //debug
    int i;
    ESPNOW_DBG("lightEspnowParam.MasterNum: %d \r\n",lightEspnowParam.MasterNum);
    for(i=0;i<SWITCH_DEV_NUM;i++){
    	ESPNOW_DBG("MAC[%d]:"MACSTR"\r\n",i,MAC2STR(lightEspnowParam.MasterMac[i]));
    }
    ESPNOW_DBG("lightEspnowParam.esp_now_key: \r\n");
    for(i=0;i<ESPNOW_KEY_LEN;i++) ESPNOW_DBG("%02X ",lightEspnowParam.esp_now_key[i]);
    os_printf("\r\n");
    ESPNOW_DBG("magic: 0x%08x \r\n\n\n\n",lightEspnowParam.magic);
    //end of debug
#endif
}
#endif


LOCAL uint32 last_token = 0;






void ICACHE_FLASH_ATTR light_EspnowRcvCb(u8 *macaddr, u8 *data, u8 len)
{
    int i;
    uint32 duty_set[PWM_CHANNEL] = {0};
    uint32 period=1000;
    LOCAL uint8 color_bit=1;
    LOCAL uint8 toggle_flg = 0;
    
    EspnowProtoMsg EspnowMsg;
    os_memcpy((uint8*)(&EspnowMsg), data,sizeof(EspnowProtoMsg));
    ESPNOW_DBG("RCV DATA: \r\n");
    for(i=0;i<len;i++) ESPNOW_DBG("%02x ",data[i]);
    ESPNOW_DBG("\r\n-----------------------\r\n");
    
    
    
    if(light_EspnowCmdValidate(&EspnowMsg) ){
        ESPNOW_DBG("cmd check sum ok\r\n");
        //if(light_cmd.wifiChannel == wifi_get_channel()){
            //ACT_TYPE_DATA: this is a set of data or command.
            ESPNOW_DBG("ESPNOW CMD CODE : %d \r\n",EspnowMsg.lightCmd.cmd_code);
            if(EspnowMsg.type == ACT_TYPE_DATA){
				
				if(EspnowMsg.token == last_token){
					ESPNOW_DBG("Double cmd ... return...\r\n");
					ESPNOW_DBG("last token: %d ; cur token: %d ;\r\n",last_token,EspnowMsg.token);
					return;
				}

                ESPNOW_DBG("period: %d ; channel : %d \r\n",EspnowMsg.lightCmd.period,EspnowMsg.lightCmd.pwm_num);
                for(i=0;i<EspnowMsg.lightCmd.pwm_num;i++){
                    ESPNOW_DBG(" duty[%d] : %d \r\n",i,EspnowMsg.lightCmd.duty[i]);
                    duty_set[i] = EspnowMsg.lightCmd.duty[i];
                }
                
                ESPNOW_DBG("SOURCE MAC CHEK OK \r\n");
                ESPNOW_DBG("cmd channel : %d \r\n",EspnowMsg.wifiChannel);
                ESPNOW_DBG("SELF CHANNEL: %d \r\n",wifi_get_channel());
                ESPNOW_DBG("Battery status %d voltage %dmV\r\n", EspnowMsg.lightCmd.batteryStat.battery_status, EspnowMsg.lightCmd.batteryStat.battery_voltage_mv);
                light_EspnowStoreBatteryStatus(macaddr, EspnowMsg.lightCmd.batteryStat.battery_status, EspnowMsg.lightCmd.batteryStat.battery_voltage_mv);
                
                ESPNOW_DBG("***************\r\n");
                ESPNOW_DBG("EspnowMsg.lightCmd.colorChg: %d \r\n",EspnowMsg.lightCmd.cmd_code);
                if(EspnowMsg.lightCmd.cmd_code==COLOR_SET){
					//replace with timer , for demo
					//light_TimerAdd(EspnowMsg.lightCmd.duty);
					//light_ShowDevLevel(1);
					
					light_hint_abort();
					light_TimerAdd(NULL);
					ESPNOW_DBG("SHUT DOWN ADD 20 SECONDS...\r\n");
					//end 
					
					#if 0
                    color_bit = 1;
                    toggle_flg= 1;
                    ESPNOW_DBG("set color : %d %d %d %d %d %d \r\n",duty_set[0],duty_set[1],duty_set[2],duty_set[3],duty_set[4],EspnowMsg.lightCmd.period);
                    light_set_aim(duty_set[0],duty_set[1],duty_set[2],duty_set[3],duty_set[4],EspnowMsg.lightCmd.period,0);
					#endif
				}else if(EspnowMsg.lightCmd.cmd_code==COLOR_CHG){
				    light_hint_abort();
				    light_TimerStop();
				    toggle_flg= 1;
                    ESPNOW_DBG("light change color cmd \r\n");
                    for(i=0;i<PWM_CHANNEL;i++){
						if(i<3)
                        	duty_set[i] = ((color_bit>>i)&0x1)*22222;
						else
							duty_set[i] = ((color_bit>>i)&0x1)*8000;
                    }
                    ESPNOW_DBG("set color : %d %d %d %d %d %d\r\n",duty_set[0],duty_set[1],duty_set[2],duty_set[3],duty_set[4],EspnowMsg.lightCmd.period);
                    light_set_aim( duty_set[0],duty_set[1],duty_set[2],duty_set[3],duty_set[4],1000,0);	
                    color_bit++;
                    if(color_bit>=32) color_bit=1;
                }else if(EspnowMsg.lightCmd.cmd_code==COLOR_TOGGLE){
                    light_hint_abort();
                    light_TimerStop();
    				color_bit = 1;
    				//toggle_flg= 1;
                    if(toggle_flg == 0){
                        light_set_aim( 0,0,0,0,0,1000,0); 
                        toggle_flg = 1;
                    }else{
                        light_set_aim( 0,0,0,22222,22222,1000,0); 
                        toggle_flg = 0;
                    }
                }else if(EspnowMsg.lightCmd.cmd_code==COLOR_LEVEL){
                    light_hint_abort();
                    light_TimerStop();

					
                    struct ip_info sta_ip;
                    wifi_get_ip_info(STATION_IF,&sta_ip);
                #if ESP_MESH_SUPPORT						
                    if( espconn_mesh_local_addr(&sta_ip.ip)){
                        ESPNOW_DBG("THIS IS A MESH SUB NODE..\r\n");
                        uint32 mlevel = sta_ip.ip.addr&0xff;
                        light_ShowDevLevel(mlevel);
                    }else if(sta_ip.ip.addr!= 0){
                        ESPNOW_DBG("THIS IS A MESH ROOT..\r\n");
                        light_ShowDevLevel(1);
                        
                    }else{
                        ESPNOW_DBG("THIS IS A MESH ROOT..\r\n");
                        light_ShowDevLevel(0);
                    }
                #else
                    if(sta_ip.ip.addr!= 0){
                        ESPNOW_DBG("wifi connected..\r\n");
                        light_ShowDevLevel(1);
                    }else{
                        ESPNOW_DBG("wifi not connected..\r\n");
                        light_ShowDevLevel(0);
                    }
                #endif
                    
                
                }else if(EspnowMsg.lightCmd.cmd_code==LIGHT_RESET){
                    system_restore();
                    extern struct esp_platform_saved_param esp_param;
                    esp_param.reset_flg=0;
                    esp_param.activeflag = 0;
                    os_memset(esp_param.token,0,40);
                    system_param_save_with_protect(ESP_PARAM_START_SEC, &esp_param, sizeof(esp_param));
                    ESPNOW_DBG("--------------------\r\n");
                    ESPNOW_DBG("SYSTEM PARAM RESET !\r\n");
                    ESPNOW_DBG("RESET: %d ;  ACTIVE: %d \r\n",esp_param.reset_flg,esp_param.activeflag);
                    ESPNOW_DBG("--------------------\r\n\n\n");
                    UART_WaitTxFifoEmpty(0,3000);
                    system_restart();
                }else if(EspnowMsg.lightCmd.cmd_code==LIGHT_TIMER){
					light_TimerAdd(EspnowMsg.lightCmd.duty);
					ESPNOW_DBG("SHUT DOWN ADD 20 SECONDS...\r\n");
                }
				
				last_token = EspnowMsg.token;

				if(EspnowMsg.rsp_if ==1){
					EspnowMsg.type=ACT_TYPE_RSP;
				}else{
                    ESPNOW_DBG("do not send response\r\n");
                    return;
				}
            }
            //ACT_TYPE_SYNC: AFTER LIGHT CONFIGURED TO ROUTER, ITS CHANNEL CHANGES
            //               NEED TO SET THE CHANNEL FOR EACH LIGHT.
            else if(EspnowMsg.type == ACT_TYPE_SYNC && (EspnowMsg.lightSync.switchChannel == wifi_get_channel())){
                EspnowMsg.type = ACT_TYPE_SYNC;
                ESPNOW_DBG("cmd rcv channel : %d \r\n",EspnowMsg.wifiChannel);
                EspnowMsg.wifiChannel = wifi_get_channel();
                EspnowMsg.lightSync.lightChannel = wifi_get_channel();
                ESPNOW_DBG("send sync, self channel : %d  \r\n",wifi_get_channel());
                light_Espnow_ShowSyncSuc();
            }else{
                ESPNOW_DBG(" data type %d \r\n",EspnowMsg.type);
                ESPNOW_DBG("data channel :%d \r\n",EspnowMsg.wifiChannel);
                ESPNOW_DBG("data self channel: %d \r\n",wifi_get_channel());
                ESPNOW_DBG("data type or channel error\r\n");
                return;
            }
			
			EspnowMsg.rsp_if=0;
			EspnowMsg.token=os_random();
            light_EspnowSetCsum(&EspnowMsg);
            
        #if ACT_DEBUG
            int j;
            for(j=0;j<sizeof(EspnowLightCmd);j++) ESPNOW_DBG("%02x ",*((uint8*)(&EspnowMsg)+j));
            ESPNOW_DBG("\r\n");
        #endif
		    ESPNOW_DBG("SEND TO MAC: "MACSTR"\r\n",MAC2STR(macaddr));
            esp_now_send(macaddr, (uint8*)(&EspnowMsg), sizeof(EspnowProtoMsg)); //send ack
        //}
    }else{
        ESPNOW_DBG("cmd check sum error\r\n");
    }
}


LOCAL void ICACHE_FLASH_ATTR espnow_send_cb(u8 *mac_addr, u8 status)
{
	os_printf("send_cb: mac[%02x:%02x:%02x:%02x:%02x:%02x], status: [%d]\n", MAC2STR(mac_addr),status); 
}


//ESPNOW INIT,if encryption is enabled, the slave device number is limited.
void ICACHE_FLASH_ATTR light_EspnowInit()
{
    _LINE_DESP();
    ESPNOW_DBG("CHANNEL : %d \r\n",wifi_get_channel());
    _LINE_DESP();
    
    if (esp_now_init()==0) {
        ESPNOW_DBG("direct link  init ok\n");
        esp_now_register_recv_cb(light_EspnowRcvCb);
		esp_now_register_send_cb(espnow_send_cb);	 
    } else {
        ESPNOW_DBG("esp-now already init\n");
    }
    //send data from station interface of switch to softap interface of light
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);  //role 1: switch   ;  role 2 : light;
    //-------chg 151008------------
    //send ESPNOW through STATION interface
    //esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER); 
	//----------------------------

	
	PairedButtonParam* sp_dev = (PairedButtonParam*)sp_GetPairedParam();
	
    #if ESPNOW_KEY_HASH
    //esp_now_set_kok((uint8*)esp_now_key, ESPNOW_KEY_LEN);
    //esp_now_set_kok((uint8*)lightEspnowParam.esp_now_key, ESPNOW_KEY_LEN);
    esp_now_set_kok((uint8*)(sp_dev->PairedList.key_t), ESPNOW_KEY_LEN);
    #endif
    
    int j,res;
    //for(j=0;j<lightEspnowParam.MasterNum;j++){
    for(j=0;j<sp_dev->PairedNum;j++){
    #if ESPNOW_ENCRYPT
	//res = esp_now_add_peer((uint8*)SWITCH_MAC[j], (uint8)ESP_NOW_ROLE_CONTROLLER,(uint8)WIFI_DEFAULT_CHANNEL, (uint8*)esp_now_key, (uint8)ESPNOW_KEY_LEN);
	res = esp_now_add_peer((uint8*)(sp_dev->PairedList[j].mac_t), (uint8)ESP_NOW_ROLE_CONTROLLER,(uint8)WIFI_DEFAULT_CHANNEL, (uint8*)(sp_dev->PairedList[j].key_t), (uint8)ESPNOW_KEY_LEN);
    #else
	//res = esp_now_add_peer((uint8*)SWITCH_MAC[j], (uint8)ESP_NOW_ROLE_CONTROLLER,(uint8)WIFI_DEFAULT_CHANNEL, NULL, (uint8)ESPNOW_KEY_LEN);
	res = esp_now_add_peer((uint8*)(sp_dev->PairedList[j].mac_t), (uint8)ESP_NOW_ROLE_CONTROLLER,(uint8)WIFI_DEFAULT_CHANNEL, NULL, (uint8)ESPNOW_KEY_LEN);
    #endif
	    if(res == 0){
		    //ESPNOW_DBG("INIT OK: MAC[%d]:"MACSTR"\r\n",j,MAC2STR(((uint8*)SWITCH_MAC[j])));
		    ESPNOW_DBG("INIT OK: MAC[%d]:"MACSTR"\r\n",j,MAC2STR(((uint8*)(sp_dev->PairedList[j].mac_t))));
	    }else{
		    //ESPNOW_DBG("INIT OK: MAC[%d]:"MACSTR"\r\n",j,MAC2STR(((uint8*)SWITCH_MAC[j])));
		    ESPNOW_DBG("INIT OK: MAC[%d]:"MACSTR"\r\n",j,MAC2STR(((uint8*)(sp_dev->PairedList[j].mac_t))));
	    }
    }
	

}

void ICACHE_FLASH_ATTR light_EspnowDeinit()
{
	ESPNOW_DBG("%s\r\n",__func__);
    esp_now_unregister_recv_cb(); 
    esp_now_deinit();
}






#endif

#endif




