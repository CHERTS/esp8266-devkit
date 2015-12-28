#ifndef CGI_H
#define CGI_H

#include "httpd.h"

int cgiGPIO(HttpdConnData *connData);
void tplGPIO(HttpdConnData *connData, char *token, void **arg);
int cgiReadFlash(HttpdConnData *connData);
void tplCounter(HttpdConnData *connData, char *token, void **arg);
void tplDHT(HttpdConnData *connData, char *token, void **arg);
int cgiDHT22(HttpdConnData *connData);
void tplDS18b20(HttpdConnData *connData, char *token, void **arg);
int cgiDS18b20(HttpdConnData *connData);
int cgiState(HttpdConnData *connData);
int cgiUI(HttpdConnData *connData);
void tplUI(HttpdConnData *connData, char *token, void **arg);
void tplMQTT(HttpdConnData *connData, char *token, void **arg);
int cgiMQTT(HttpdConnData *connData);
void tplHTTPD(HttpdConnData *connData, char *token, void **arg);
int cgiHTTPD(HttpdConnData *connData);
void tplBroadcastD(HttpdConnData *connData, char *token, void **arg);
int cgiBroadcastD(HttpdConnData *connData);
void tplNTP(HttpdConnData *connData, char *token, void **arg);
int cgiNTP(HttpdConnData *connData);
int cgiReset(HttpdConnData *connData);
void tplRLYSettings(HttpdConnData *connData, char *token, void **arg);
int cgiRLYSettings(HttpdConnData *connData);
void tplSensorSettings(HttpdConnData *connData, char *token, void **arg);
int cgiSensorSettings(HttpdConnData *connData);

#endif
