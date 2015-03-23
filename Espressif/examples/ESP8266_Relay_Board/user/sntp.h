#ifndef __SNTP_H__
#define __SNTP_H__

#include "lwip/ip_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
extern time_t sntp_time;
extern int sntp_tz;

void sntp_init(int tz);
void sntp_stop(void);

extern void sntp_send_request(ip_addr_t *server_addr);
extern void sntp_request(void *arg);


#ifdef __cplusplus
}
#endif

#endif /* __SNTP_H__ */
