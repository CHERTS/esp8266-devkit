/******************************************************************************
 * Copyright 2013-2014 Espressif Systems
 *
 * FileName: user_config.h
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2015/9/24, v1.0 create this file.
*******************************************************************************/

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

enum {
	TRANS_CLIENT_CONNECTED = 0,
	TRANS_CLIENT_DISCONNECTED,
	TRANS_RECV_DATA_FROM_UART,
	TRANS_SEND_DATA_TO_UART_OVER,
};
#define TRANS_TASK_PROI	0
#endif
