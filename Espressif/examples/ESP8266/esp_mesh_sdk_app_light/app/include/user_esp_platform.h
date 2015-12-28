#ifndef __USER_DEVICE_H__
#define __USER_DEVICE_H__
#include "user_config.h"
#if 0  //moved to user_config.h
/* NOTICE---this is for 512KB spi flash.
 * you can change to other sector if you use other size spi flash. */
#define ESP_PARAM_START_SEC		0x7D

#define ESP_PARAM_SAVE_0    1
#define ESP_PARAM_SAVE_1    2
#define ESP_PARAM_FLAG      3

#define packet_size   (2 * 1024)

#define TOKEN_SIZE 41
#endif

#define IOT_BIN_VERSION_LEN (15)


enum{
	MODE_NORMAL,
	MODE_RESET,
	MODE_APMODE,
};

struct esp_platform_saved_param {
    uint8 devkey[40];
    uint8 token[40];
    uint8 activeflag;
	uint8 reset_flg;
    uint8 pad[2];
	uint32 ota_start_time;
	uint32 ota_finish_time;
};

struct esp_platform_sec_flag_param {
    uint8 flag; 
    uint8 pad[3];
};

enum {
    DEVICE_CONNECTING = 40,
    DEVICE_ACTIVE_DONE,
    DEVICE_ACTIVE_FAIL,
    DEVICE_CONNECT_SERVER_FAIL
};
struct dhcp_client_info {
	ip_addr_t ip_addr;
	ip_addr_t netmask;
	ip_addr_t gw;
	uint8 flag;
	uint8 pad[3];
};

enum {
    CMD_TYPE_MULITCAST,
	CMD_TYPE_UNICAST,
	CMD_TYPE_BROADCAST,
};


char * user_json_find_section(const char *pbuf, u16 len, const char *section);
uint32 user_JsonGetValueInt(const char *pbuffer, uint16 buf_len, const uint8 *json_key);



void user_esp_platform_init(void);
void user_esp_platform_init_peripheral(void);
void user_esp_platform_connect_ap_cb();
//void user_platform_flow_init(void);
void user_esp_platform_sent_data();
void user_mdns_conf(void);
void user_esp_platform_reset_default();
void user_esp_platform_set_reset_flg(uint32 rst);





#endif
