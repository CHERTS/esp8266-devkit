#ifndef USER_BUTTON_SIMPLE_PAIR_H
#define USER_BUTTON_SIMPLE_PAIR_H

#include "user_light_action.h"

typedef void(*buttonSimplePairDoneCb)(int success);

#define MAX_BUTTON_NUM 10

#define SP_PARAM_MAGIC 0x5c5caacc
#define LIGHT_PAIRED_DEV_PARAM_ADDR 0x7D


typedef struct {
uint8 mac_t[DEV_MAC_LEN];
uint8 key_t[ESPNOW_KEY_LEN];
uint8 channel_t;
}PairedSingleDev;


typedef struct {
	//uint8 button_mac[MAX_BUTTON_NUM][6];
	uint8 csum;
	uint32 magic;
	PairedSingleDev PairedList[MAX_BUTTON_NUM];
	uint8 PairedNum;
	uint8 MaxPairedDevNum;
	
}PairedButtonParam;

extern PairedButtonParam PairedDev;



typedef enum{
SP_ST_STA_FINISH=0,   //success when sta finish the neg
SP_ST_AP_FINISH=0,    // success when ap finish the neg
SP_ST_AP_RECV_NEG,    // ap recv sta's neg request
SP_ST_STA_AP_REFUSE_NEG,    // sta recv ap's neg refuse
SP_ST_WAIT_TIMEOUT,
SP_ST_SEND_ERR,
SP_ST_KEY_INSTALL_ERR,
Sp_ST_KEY_OVERLAP_ERR,
SP_ST_UNKOWN_ERR,
SP_ST_MAX,
}SP_ST_t;

typedef void (*simple_pair_status_cb_t) (uint8* sa,uint8 status);
int register_simple_pair_status_cb(simple_pair_status_cb_t);
void unregister_simple_pair_status_cb(void);

int simple_pair_init();
void simple_pair_deinit();

void simple_pair_ap_enter_scan_mode(void);
void simple_pair_sta_enter_scan_mode(void);

void simple_pair_sta_start_negotiate(void);
void simple_pair_ap_start_negotiate(void);
void simple_pair_ap_refuse_negotiate(void);
int simple_pair_set_peer_ref(uint8 *peer_mac,uint8* tmpkey,uint8*espnow_key);
int simple_pair_get_peer_ref(uint8 *peer_mac,uint8* tmpkey,uint8*espnow_key);

void ICACHE_FLASH_ATTR buttonSimplePairStart(buttonSimplePairDoneCb cb);

#endif
