#ifndef __USER_WEBSERVER_H__
#define __USER_WEBSERVER_H__
#if ESP_MESH_SUPPORT
#include "mesh.h"
#endif
#define SERVER_PORT 80
#define SERVER_SSL_PORT 443

#define URLSize 40

#define WEB_INFO os_printf

typedef enum Result_Resp {
    RespFail = 0,
    RespSuc,
} Result_Resp;

typedef enum ProtocolType {
    GET = 0,
    POST,
} ProtocolType;

typedef enum _ParmType {
    SWITCH_STATUS = 0,
    INFOMATION,
    WIFI,
    SCAN,
	REBOOT,
    DEEP_SLEEP,
    LIGHT_STATUS,
    BATTERY_STATUS,
    MESH_INFO,
    CONNECT_STATUS,
    USER_BIN
} ParmType;

typedef struct URL_Frame {
    enum ProtocolType Type;
    char pSelect[URLSize];
    char pCommand[URLSize];
    char pFilename[URLSize];
	char pPath[URLSize];
} URL_Frame;

typedef struct _rst_parm {
    ParmType parmtype;
    struct espconn *pespconn;
} rst_parm;

enum sp_Status{
LIGHT_PAIR_IDLE,
USER_PBULIC_BUTTON_INFO,
LIGHT_RECV_BUTTON_REQUSET,
USER_PERMIT_SIMPLE_PAIR,
USER_REFUSE_SIMPLE_PAIR,
LIGHT_SIMPLE_PAIR_SUCCED,
LIGHT_SIMPLE_PAIR_FAIL,
USER_CONFIG_CONTIUE_SIMPLE_PAIR,
USER_CONFIG_STOP_SIMPLE_PAIR
};



typedef struct PairSTAInfo
{
   uint8 button_mac[6];
   uint8 tempkey[16];
   uint8 espnowKey[16];
   enum sp_Status simple_pair_state;

}Button_info;

extern Button_info buttonPairingInfo;

#if ESP_MESH_SUPPORT
extern char pair_sip[1+ESP_MESH_JSON_IP_ELEM_LEN];
extern char pair_sport[1+ESP_MESH_JSON_PORT_ELEM_LEN];
#endif


void user_webserver_init(uint32 port);

#endif
