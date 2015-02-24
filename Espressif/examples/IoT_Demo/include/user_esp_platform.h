#ifndef __USER_DEVICE_H__
#define __USER_DEVICE_H__

/* NOTICE---this is for 512KB spi flash.
 * you can change to other sector if you use other size spi flash. */
#define ESP_PARAM_START_SEC		0x3C

#define ESP_PARAM_SAVE_0    1
#define ESP_PARAM_SAVE_1    2
#define ESP_PARAM_FLAG      3

#define packet_size   (2 * 1024)

#define token_size 41

struct esp_platform_saved_param {
    uint8 devkey[40];
    uint8 token[40];
    uint8 activeflag;
    uint8 pad[3];
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
#endif
