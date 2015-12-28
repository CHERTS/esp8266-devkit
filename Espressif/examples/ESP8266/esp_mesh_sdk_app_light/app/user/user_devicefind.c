/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_devicefind.c
 *
 * Description: Find your hardware's information while working any mode.
 *
 * Modification history:
 *     2014/3/12, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

#include "espconn.h"
#include "user_json.h"
#include "user_devicefind.h"

const char *device_find_request = "Are You Espressif IOT Smart Device?";
#if PLUG_DEVICE
const char *device_find_response_ok = "I'm Plug.";
#elif LIGHT_DEVICE
	#if ESP_DEBUG_MODE
        char *device_find_mesh_light = "I'm Light with mesh.";
        char *device_find_light = "I'm Light.";
        #if ESP_MESH_SUPPORT
    	    uint8* device_find_response_ok=NULL;
        #else
        	uint8* device_find_response_ok = "I'm Light.";
        #endif
	#else
        #if ESP_MESH_SUPPORT
        	    uint8* device_find_response_ok = "I'm Light with mesh.";
        #else
            	uint8* device_find_response_ok = "I'm Light.";
        #endif
	#endif
#elif SENSOR_DEVICE
#if HUMITURE_SUB_DEVICE
const char *device_find_response_ok = "I'm Humiture.";
#elif FLAMMABLE_GAS_SUB_DEVICE
const char *device_find_response_ok = "I'm Flammable Gas.";
#endif
#endif

/*---------------------------------------------------------------------------*/
LOCAL struct espconn ptrespconn;


#if ESP_DEBUG_MODE&&ESP_MESH_SUPPORT
void ICACHE_FLASH_ATTR
user_DeviceFindRespSet(bool mesh_if)
{
	if(device_find_response_ok) os_free(device_find_response_ok);

	if(mesh_if){
		device_find_response_ok = (uint8*)os_zalloc(os_strlen(device_find_mesh_light)+1);
		os_printf("malloc len: %d \r\n",os_strlen(device_find_mesh_light));
		os_strcpy(device_find_response_ok,device_find_mesh_light);
		//os_memcpy(device_find_response_ok,device_find_mesh_light,os_strlen(device_find_mesh_light));
	}else{
		device_find_response_ok = (uint8*)os_zalloc(os_strlen(device_find_light)+1);
		os_printf("malloc len: %d \r\n",os_strlen(device_find_mesh_light));
		os_strcpy(device_find_response_ok,device_find_light);
		//os_memcpy(device_find_response_ok,device_find_light,os_strlen(device_find_light));
	}

}
#endif
/******************************************************************************
 * FunctionName : user_devicefind_recv
 * Description  : Processing the received data from the host
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_devicefind_recv(void *arg, char *pusrdata, unsigned short length)
{
    char DeviceBuffer[40] = {0};
    char Device_mac_buffer[60] = {0};
    char hwaddr[6];

    struct ip_info ipconfig;

    if (wifi_get_opmode() != STATION_MODE) {
        wifi_get_ip_info(SOFTAP_IF, &ipconfig);
        wifi_get_macaddr(SOFTAP_IF, hwaddr);

        if (!ip_addr_netcmp((struct ip_addr *)ptrespconn.proto.udp->remote_ip, &ipconfig.ip, &ipconfig.netmask)) {
            wifi_get_ip_info(STATION_IF, &ipconfig);
            wifi_get_macaddr(STATION_IF, hwaddr);
        }
    } else {
        wifi_get_ip_info(STATION_IF, &ipconfig);
        wifi_get_macaddr(STATION_IF, hwaddr);
    }

    if (pusrdata == NULL) {
        return;
    }

    if (length == os_strlen(device_find_request) &&
            os_strncmp(pusrdata, device_find_request, os_strlen(device_find_request)) == 0) {
        os_sprintf(DeviceBuffer, "%s" MACSTR " " IPSTR, device_find_response_ok,
                   MAC2STR(hwaddr), IP2STR(&ipconfig.ip));

        os_printf("%s\n", DeviceBuffer);
        length = os_strlen(DeviceBuffer);


        //==================================
        //This is add in sdk lib v1.4.0
        os_printf("--------DEBUG----------\r\n");
		remote_info* pcon_info = NULL;
		os_printf("link num: %d \r\n",ptrespconn.link_cnt);
		espconn_get_connection_info(&ptrespconn, &pcon_info, 0);
		os_printf("remote ip: %d.%d.%d.%d \r\n",pcon_info->remote_ip[0],pcon_info->remote_ip[1],
			                                    pcon_info->remote_ip[2],pcon_info->remote_ip[3]);
		os_printf("remote port: %d \r\n",pcon_info->remote_port);
        //=================================
        ptrespconn.proto.udp->remote_port = pcon_info->remote_port;
		os_memcpy(ptrespconn.proto.udp->remote_ip,pcon_info->remote_ip,4);
		
        //espconn_sent(&ptrespconn, DeviceBuffer, length);
		espconn_sendto(&ptrespconn, DeviceBuffer, length);
    } else if (length == (os_strlen(device_find_request) + 18)) {
        os_sprintf(Device_mac_buffer, "%s " MACSTR , device_find_request, MAC2STR(hwaddr));
        os_printf("%s", Device_mac_buffer);

        if (os_strncmp(Device_mac_buffer, pusrdata, os_strlen(device_find_request) + 18) == 0) {
            //os_printf("%s\n", Device_mac_buffer);
            length = os_strlen(DeviceBuffer);
            os_sprintf(DeviceBuffer, "%s" MACSTR " " IPSTR, device_find_response_ok,
                       MAC2STR(hwaddr), IP2STR(&ipconfig.ip));

            os_printf("%s\n", DeviceBuffer);
            length = os_strlen(DeviceBuffer);
			
            //espconn_sent(&ptrespconn, DeviceBuffer, length);
			espconn_sendto(&ptrespconn, DeviceBuffer, length);
        } else {
            return;
        }
    }
}

/******************************************************************************
 * FunctionName : user_devicefind_init
 * Description  : the espconn struct parame init
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_devicefind_init(void)
{
	#if ESP_DEBUG_MODE && ESP_MESH_SUPPORT
 	user_DeviceFindRespSet(true);
	os_printf("device find string: len: %d ;  %s \r\n",os_strlen(device_find_response_ok),device_find_response_ok);
	
	#endif
    ptrespconn.type = ESPCONN_UDP;
    ptrespconn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
    ptrespconn.proto.udp->local_port = 1025;
    espconn_regist_recvcb(&ptrespconn, user_devicefind_recv);
    espconn_create(&ptrespconn);
}
