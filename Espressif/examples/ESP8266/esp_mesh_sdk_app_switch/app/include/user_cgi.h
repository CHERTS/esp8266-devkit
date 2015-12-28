#ifndef __USER_CGI_H__
#define __USER_CGI_H__

#include "httpd.h"

int ICACHE_FLASH_ATTR cgiGetConfig(HttpdConnData *connData);
int ICACHE_FLASH_ATTR cgiSetConfig(HttpdConnData *connData);

#endif