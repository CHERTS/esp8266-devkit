#ifndef _ESP_SEND_H
#define _ESP_SEND_H

#include "ringbuf.h"

typedef enum {
    ESP_DATA,   //send data to server
	ESP_BEACON, //send heart beat / keep alive
	ESP_RPC,    //send RPC command to server
	ESP_RSP,    //send response to server 
} EspPlatformMsgType;

typedef struct {
    struct espconn* pConn;
    uint16     dataLen;
	uint8      dType;
	uint8      dTgt;
}EspSendFrame;

typedef enum{
	SEND_OK,
	SEND_RETRY,
	
}EspSendState;

typedef enum{
	TO_SERVER,
	TO_LOCAL,
}EspSendTgt;


#define ESP_SEND_TASK_QUEUE_SIZE  2
#define ESP_SEND_TASK_PRIO 0
RINGBUF* espSendGetRingbuf();
bool espSendQueueIsEmpty(RINGBUF* r);
void espSendQueueInit();
sint8 espSendQueueUpdate(RINGBUF* r);
void espSendAck(RINGBUF* r);



#endif
