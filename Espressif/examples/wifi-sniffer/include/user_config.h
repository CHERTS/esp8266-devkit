#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define CHANNEL_HOP_INTERVAL	5000
#define user_procTaskPrio       0
#define user_procTaskQueueLen   1

#define printmac(buf, i) os_printf("\t%02X:%02X:%02X:%02X:%02X:%02X", buf[i+0], buf[i+1], buf[i+2], \
                                                   buf[i+3], buf[i+4], buf[i+5])

#endif
