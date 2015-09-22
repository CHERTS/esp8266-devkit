#ifndef _USER_WIFI_CONNECT_H
#define _USER_WIFI_CONNECT_H
#include "c_types.h"

typedef void (*WifiCallback)(uint8);

void WIFI_Connect(uint8* ssid, uint8* pass, WifiCallback cb);
void WIFI_StartCheckIp();







#endif
