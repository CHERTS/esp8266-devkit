#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__
#include "c_types.h"

#define ESP_PLATFORM        1

#define USE_OPTIMIZE_PRINTF

#if ESP_PLATFORM
#define PLUG_DEVICE             0
#define LIGHT_DEVICE            1
#define SENSOR_DEVICE			0
#define LIGHT_SWITCH            0

#define ESP_MESH_SUPPORT    1  /*set 1: enable mesh*/
#define ESP_TOUCH_SUPPORT  1   /*set 1: enable esptouch*/
#define ESP_NOW_SUPPORT 1      /*set 1: enable espnow*/
#define ESP_WEB_SUPPORT 0      /*set 1: enable new http webserver*/
#define ESP_MDNS_SUPPORT 0     /*set 1: enable mdns*/

#define ESP_MESH_STRIP  1

#define ESP_DEBUG_MODE  1
#define ESP_RESET_DEBUG_EN 1 

#define MESH_TEST_VERSION   0x4


typedef enum{
    ESP_DEV_PLUG,
    ESP_DEV_PLUGS,
    ESP_DEV_LIGHT,
    ESP_DEV_HUMITURE,
    ESP_DEV_GAS_ALARM,
    ESP_DEV_VOLTAGE,
    ESP_DEV_REMOTE,
    ESP_DEV_SWITCH,
}DevType;


//flash data address.
#define FLASH_PARAM_ADDR_0 0x7C  //SAVE PLATFORM PARAM(DEVKEY,TOKEN,...)
#define FLASH_PARAM_ADDR_1 0X7D  //SAVE LIGHT PARAM(DUTY,PERIOD...)
#define FLASH_PARAM_ADDR_2 0X79  //SAVE SWITCH MAC LIST AND ESPNOW KEY
#define FLASH_PARAM_ADDR_3 0xF8  //RECORD EXCEPTION PARAM.
#define FLASH_PARAM_ADDR_4 0xF9  //RECORD EXCEPTION INFO.
#define FLASH_PARAM_ADDR_5 0x76  //RECORD MODULE ENABLE FLAGS



/*============================================================*/
/*====                                                    ====*/
/*====        PARAMS FOR KEEPING EXCEPTION INFO           ====*/
/*====                                                    ====*/
/*============================================================*/
#if ESP_DEBUG_MODE
#define Flash_DEBUG_INFO_ADDR  FLASH_PARAM_ADDR_3
#define FLASH_DEBUG_SV_ADDR    FLASH_PARAM_ADDR_4
#define FLASH_DEBUG_SV_SIZE    0x1000  
#endif



/*============================================================*/
/*====                                                    ====*/
/*====        PARAMS FOR ESP-MESH                         ====*/
/*====                                                    ====*/
/*============================================================*/

#if ESP_MESH_SUPPORT

    /*This is the configuration of mesh*/

	
	enum{
		//MULTICAST_CMD_INGROUP,
		//MULTICAST_CMD_EXGROUP,
		//MULTICAST_CMD_NOGROUP,
		//MULTICAST_CMD_NOTROOT,
		MULTICAST_CMD_ROOT_INGROUP,
		MULTICAST_CMD_ROOT_EXGROUP,
		MULTICAST_CMD_NODE_INGROUP,
		MULTICAST_CMD_NODE_EXGROUP,
		MULTICAST_CMD_NOGROUP,
	};


    #define MESH_INFO os_printf
    #define MESH_INIT_RETRY_LIMIT 0       /*number of retry attempts after mesh enable failed;*/
    
    #define MESH_INIT_TIME_LIMIT  120000   /*limit time of mesh init*/
    #define MESH_TIME_OUT_MS   120000
    
    #define MESH_STATUS_CHECK_MS  1000    /*time expire to check mesh init status*/
    #define MESH_UPGRADE_SEC_SIZE 640     /*length of binary upgrade stream in a single packet*/
    #define MESH_SSID_PREFIX "TEST_MESH"   /*SET THE DEFAULT MESH SSID PATTEN;THE FINAL SSID OF SOFTAP WOULD BE "MESH_SSID_PREFIX_X_YYYYYY"*/
    #define MESH_AUTH  AUTH_WPA2_PSK       /*AUTH_MODE OF SOFTAP FOR EACH MESH NODE*/
    #define MESH_PASSWORD "123123123"    /*SET PASSWORD OF SOFTAP FOR EACH MESH NODE*/
    #define MAC2STR5BYTES(a) (*((a)+1)), (*((a)+2)), (*((a)+3)), (*((a)+4)), (*((a)+5))

#endif






/*============================================================*/
/*====                                                    ====*/
/*====        PARAMS FOR ESP-LIGHT                        ====*/
/*====                                                    ====*/
/*============================================================*/

#if LIGHT_DEVICE
    /* TAKE 8Mbit flash as example*/
    /* 0x7C000     |     0x7D000        |        0x7E000      |  0x7F000 */
    /* light_param | platform_param(a)  |  platform_param(b)  |  platform_param_flag */

    /* You can change to other sector if you use other size spi flash. */
    /* Refer to the documentation about OTA support and flash mapping*/
    #define PRIV_PARAM_START_SEC		FLASH_PARAM_ADDR_0
    #define PRIV_PARAM_SAVE     0

	/* NOTICE---this is for 1024KB spi flash.
	 * you can change to other sector if you use other size spi flash. */
    #define ESP_PARAM_START_SEC		FLASH_PARAM_ADDR_1    	
    //#define packet_size   (2 * 1024)
    #define TOKEN_SIZE 41
    
    /*Define the channel number of PWM*/
    /*In this demo, we can set 3 for 3 PWM channels: RED, GREEN, BLUE*/
    /*Or , we can choose 5 channels : RED,GREEN,BLUE,COLD-WHITE,WARM-WHITE*/
    #define PWM_CHANNEL	5  /*5:5channel ; 3:3channel*/
    
    /*Definition of GPIO PIN params, for GPIO initialization*/
    #define PWM_0_OUT_IO_MUX PERIPHS_IO_MUX_MTDI_U
    #define PWM_0_OUT_IO_NUM 12
    #define PWM_0_OUT_IO_FUNC  FUNC_GPIO12
    #define PWM_1_OUT_IO_MUX PERIPHS_IO_MUX_MTDO_U
    #define PWM_1_OUT_IO_NUM 15
    #define PWM_1_OUT_IO_FUNC  FUNC_GPIO15
    #define PWM_2_OUT_IO_MUX PERIPHS_IO_MUX_MTCK_U
    #define PWM_2_OUT_IO_NUM 13
    #define PWM_2_OUT_IO_FUNC  FUNC_GPIO13
    #define PWM_3_OUT_IO_MUX PERIPHS_IO_MUX_MTMS_U
    #define PWM_3_OUT_IO_NUM 14
    #define PWM_3_OUT_IO_FUNC  FUNC_GPIO14
    #define PWM_4_OUT_IO_MUX PERIPHS_IO_MUX_GPIO5_U
    #define PWM_4_OUT_IO_NUM 5
    #define PWM_4_OUT_IO_FUNC  FUNC_GPIO5
    
    //---------------------------------------------------
    /*save RGB params to flash when calling light_set_aim*/
    #define SAVE_LIGHT_PARAM  0                 /*set to 0: do not save color params*/
    
    /*check current consumption and limit the total current for LED driver IC*/
    /*NOTE: YOU SHOULD REPLACE WIHT THE LIMIT CURRENT OF YOUR OWN APPLICATION*/
    #define LIGHT_CURRENT_LIMIT  1              /*set to 0: do not limit total current*/
    #if LIGHT_CURRENT_LIMIT
    #define LIGHT_TOTAL_CURRENT_MAX  (550*1000) /*550000/1000 MA AT MOST*/
    #define LIGHT_CURRENT_MARGIN  (80*1000)     /*80000/1000 MA CURRENT RAISES WHILE TEMPERATURE INCREASING*/
    #define LIGHT_CURRENT_MARGIN_L2  (110*1000) /*110000/1000 MA */
    #define LIGHT_CURRENT_MARGIN_L3  (140*1000) /*140000/1000 MA */
    #endif
#endif

















/*============================================================*/
/*====                                                    ====*/
/*====        PARAMS FOR ESP-NOW                          ====*/
/*====                                                    ====*/
/*============================================================*/
#define DEV_MAC_LEN 6		  /*MAC ADDR LENGTH(BYTES)*/

#if ESP_NOW_SUPPORT



    #define WIFI_DEFAULT_CHANNEL 1
    #define ESPNOW_DBG os_printf  /*debug info for espnow*/
    #define ESPNOW_ENCRYPT  1     /*enable espnow encryption*/
    #define ESPNOW_KEY_HASH 0     /*Use hashed value of given key as the espnow encryption key */
    #define ESPNOW_KEY_LEN 16     /*ESPNOW KEY LENGTH,DO NOT CHANGE IT,FIXED AS 16*/
    //#define LIGHT_DEV_NUM  10   /*LIGHT DEV NUMBER FOR CONTROLLER DEV,NOT USED IN LIGHT APP(already moved to flash param)*/
    #define SWITCH_DEV_NUM 5      /*SET THE NUMBER OF THE CONTROLLER DEVICES,IF SET ESPNOW_ENCRYPT,THE NUMBER IS LIMITED TO 5*/
	//#define ESPNOW_CMD_INTVERVAL  250000  /*MIN INTERVAL BETWEEN TWO ESPNOW COMMAND(us)*/
	
    #define LIGHT_MASTER_MAC_LIST_ADDR  FLASH_PARAM_ADDR_2 /*flash sec num where keeps the controller mac addresses*/
	#define LIGHT_PAIRED_DEV_PARAM_ADDR FLASH_PARAM_ADDR_2
	//uint8 SWITCH_MAC_2[SWITCH_DEV_NUM][6];
    //#define LIGHT_MASTER_MAC_LIST_ADDR (ESP_PARAM_START_SEC*SPI_FLASH_SEC_SIZE+sizeof(struct esp_platform_saved_param))


	#if 0
    /*KEY FOR ESPNOW ENCRYPTION*/
    /*NOTE: THE SAME KEY MUST BE SET ON BOTH SIDES: LIGHT&CONTROLLER*/
    static const uint8 esp_now_key[ESPNOW_KEY_LEN] = {0x10,0xfe,0x94, 0x7c,0xe6,0xec,0x19,0xef,0x33, 0x9c,0xe6,0xdc,0xa8,0xff,0x94, 0x7d};//key
    /*MAC LIST FOR THE CONTROLLER DEVICES,HARD CODED NOW*/
    static const uint8 SWITCH_MAC[SWITCH_DEV_NUM][DEV_MAC_LEN] = {{0x18,0xfe,0x34, 0x9a,0xb3,0xfe},
                                                                  {0x18,0xfe,0x34, 0xa5,0x3d,0x68},
                                                                  {0x18,0xfe,0x34, 0xa5,0x3d,0x66},
                                                                  {0x18,0xfe,0x34, 0xa5,0x3d,0x7b},
                                                                  
                                                                  {0x18,0xfe,0x34, 0xa5,0x3d,0x84}};
    #endif
   
#endif






/*============================================================*/
/*====                                                    ====*/
/*====        PARAMS FOR ESP-TOUCH                        ====*/
/*====                                                    ====*/
/*============================================================*/
#if ESP_TOUCH_SUPPORT
    #define ESPTOUCH_CONNECT_TIMEOUT_MS 40000   /*Time limit for connecting WiFi after ESP-TOUCH figured out the SSID&PWD*/
    #define ESP_TOUCH_TIME_ENTER  60000         /*Time limit for ESP-TOUCH to receive config packets*/
    #define ESP_TOUCH_TIMEOUT_MS 120000         /*Total time limit for ESP-TOUCH*/
#endif





#if SENSOR_DEVICE
#define HUMITURE_SUB_DEVICE         1
#define FLAMMABLE_GAS_SUB_DEVICE    0
#endif

//#define SERVER_SSL_ENABLE
//#define CLIENT_SSL_ENABLE
//#define UPGRADE_SSL_ENABLE


#define USE_DNS
#ifdef USE_DNS
#define ESP_DOMAIN      "iot.espressif.cn"
#endif

//#define SOFTAP_ENCRYPT
#ifdef SOFTAP_ENCRYPT
#define PASSWORD	"v*%W>L<@i&Nxe!"
#endif

#if SENSOR_DEVICE
#define SENSOR_DEEP_SLEEP

#if HUMITURE_SUB_DEVICE
#define SENSOR_DEEP_SLEEP_TIME    30000000
#elif FLAMMABLE_GAS_SUB_DEVICE
#define SENSOR_DEEP_SLEEP_TIME    60000000
#endif
#endif

#if LIGHT_DEVICE
#define USE_US_TIMER
#endif

#if PLUG_DEVICE || LIGHT_DEVICE
#define BEACON_TIMEOUT  150000000
#define BEACON_TIME     50000

#endif

#define AP_CACHE           1

#if AP_CACHE
#define AP_CACHE_NUMBER    5
#endif

#endif

#endif

