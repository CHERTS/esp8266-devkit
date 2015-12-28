#include <esp8266.h>
#include "httpd.h"
#include "sha1.h"
#include "base64.h"
#include "cgiwebsocket.h"

#define WS_KEY_IDENTIFIER "Sec-WebSocket-Key: "
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

/* from IEEE RFC6455 sec 5.2
      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+
*/

#define FLAG_FIN (1 << 7)

#define OPCODE_CONTINUE 0x0
#define OPCODE_TEXT 0x1
#define OPCODE_BINARY 0x2
#define OPCODE_CLOSE 0x8
#define OPCODE_PING 0x9
#define OPCODE_PONG 0xA

#define FLAGS_MASK ((uint8_t)0xF0)
#define OPCODE_MASK ((uint8_t)0x0F)
#define IS_MASKED ((uint8_t)(1<<7))
#define PAYLOAD_MASK ((uint8_t)0x7F)

typedef struct WebsockFrame WebsockFrame;

#define ST_FLAGS 0
#define ST_LEN0 1
#define ST_LEN1 2
#define ST_LEN2 3
//...
#define ST_LEN8 9
#define ST_MASK1 10
#define ST_MASK4 13
#define ST_PAYLOAD 14

struct WebsockFrame {
	uint8_t flags;
	uint8_t len8;
	uint64_t len;
	uint8_t mask[4];
};

struct WebsockPriv {
	struct WebsockFrame fr;
	uint8_t maskCtr;
	uint8 frameCont;
	uint8 mustClose;
	int wsStatus;
	Websock *next; //in linked list
};

static Websock *llStart=NULL;

static int ICACHE_FLASH_ATTR sendFrameHead(Websock *ws, int opcode, int len) {
	char buf[14];
	int i=0;
	buf[i++]=opcode;
	if (len>65535) {
		buf[i++]=127;
		buf[i++]=0; buf[i++]=0; buf[i++]=0; buf[i++]=0; 
		buf[i++]=len>>24;
		buf[i++]=len>>16;
		buf[i++]=len>>8;
		buf[i++]=len;
	} else if (len>125) {
		buf[i++]=126;
		buf[i++]=len>>8;
		buf[i++]=len;
	} else {
		buf[i++]=len;
	}
	return httpdSend(ws->conn, buf, i);
}

int ICACHE_FLASH_ATTR cgiWebsocketSend(Websock *ws, char *data, int len, int flags) {
	int r;
	int fl=0;
	if (flags&WEBSOCK_FLAG_BIN) fl=OPCODE_BINARY; else fl=OPCODE_TEXT;
	if (!(flags&WEBSOCK_FLAG_CONT)) fl|=FLAG_FIN;
	sendFrameHead(ws, fl, len);
	r=httpdSend(ws->conn, data, len);
	httpdFlushSendBuffer(ws->conn);
	return r;
}

//Broadcast data to all websockets at a specific url. Returns the amount of connections sent to.
int ICACHE_FLASH_ATTR cgiWebsockBroadcast(char *resource, char *data, int len, int flags) {
	Websock *lw=llStart;
	int ret=0;
	while (lw!=NULL) {
		if (os_strcmp(lw->conn->url, resource)==0) {
			cgiWebsocketSend(lw, data, len, flags);
			ret++;
		}
		lw=lw->priv->next;
	}
	return ret;
}


void ICACHE_FLASH_ATTR cgiWebsocketClose(Websock *ws) {
	sendFrameHead(ws, FLAG_FIN|OPCODE_CLOSE, 0);
	httpdFlushSendBuffer(ws->conn);
}


int ICACHE_FLASH_ATTR cgiWebSocketRecv(HttpdConnData *connData, char *data, int len) {
	int i, j, sl;
	Websock *ws=(Websock*)connData->cgiPrivData;
	for (i=0; i<len; i++) {
//		os_printf("Ws: State %d byte 0x%02X\n", ws->priv->wsStatus, data[i]);
		if (ws->priv->wsStatus==ST_FLAGS) {
			ws->priv->maskCtr=0;
			ws->priv->frameCont=0;
			ws->priv->fr.flags=(uint8_t)data[i];
			ws->priv->wsStatus=ST_LEN0;
		} else if (ws->priv->wsStatus==ST_LEN0) {
			ws->priv->fr.len8=(uint8_t)data[i];
			if ((ws->priv->fr.len8&127)>=126) {
				ws->priv->fr.len=0;
				ws->priv->wsStatus=ST_LEN1;
			} else {
				ws->priv->fr.len=ws->priv->fr.len8&127;
				ws->priv->wsStatus=(ws->priv->fr.len8&IS_MASKED)?ST_MASK1:ST_PAYLOAD;
			}
		} else if (ws->priv->wsStatus<=ST_LEN8) {
			ws->priv->fr.len=(ws->priv->fr.len<<8)|data[i];
			if (((ws->priv->fr.len8&127)==126 && ws->priv->wsStatus==ST_LEN2) || ws->priv->wsStatus==ST_LEN8) {
				 if (ws->priv->fr.len8&IS_MASKED) ws->priv->wsStatus=ST_MASK1; else ws->priv->wsStatus=ST_PAYLOAD;
			} else {
				ws->priv->wsStatus++;
			}
		} else if (ws->priv->wsStatus<=ST_MASK4) {
			ws->priv->fr.mask[ws->priv->wsStatus-ST_MASK1]=data[i];
			ws->priv->wsStatus++;
		} else if (ws->priv->wsStatus==ST_PAYLOAD) {
			//Okay, header is in; this is a data byte. We're going to process all the data bytes we have 
			//received here at the same time; no more byte iterations till the end of this frame.
			//First, unmask the data
			sl=len-i;
			for (j=0; j<sl; j++) data[i+j]^=(ws->priv->fr.mask[(ws->priv->maskCtr++)&3]);

			//Inspect the header to see what we need to do.
			if ((ws->priv->fr.flags&OPCODE_MASK)==OPCODE_PING) {
				if (!ws->priv->frameCont) sendFrameHead(ws, OPCODE_PONG, 0);
				if (ws->priv->fr.len>0) httpdSend(ws->conn, data+i, sl);
			} else if ((ws->priv->fr.flags&OPCODE_MASK)==OPCODE_TEXT || 
						(ws->priv->fr.flags&OPCODE_MASK)==OPCODE_BINARY ||
						(ws->priv->fr.flags&OPCODE_MASK)==OPCODE_CONTINUE) {
				if (sl>ws->priv->fr.len) sl=ws->priv->fr.len;
				if (!(ws->priv->fr.len8&IS_MASKED)) {
					//We're a server; client should send us masked packets.
					cgiWebsocketClose(ws);
				} else {
					int flags=0;
					if ((ws->priv->fr.flags&OPCODE_MASK)==OPCODE_BINARY) flags|=WEBSOCK_FLAG_BIN;
					if ((ws->priv->fr.flags&FLAG_FIN)==0) flags|=WEBSOCK_FLAG_CONT;
					if (ws->recvCb) ws->recvCb(ws, data+i, sl, flags);
				}
			} else if ((ws->priv->fr.flags&OPCODE_MASK)==OPCODE_CLOSE) {
				ws->priv->mustClose=1;
			}
			ws->priv->frameCont=1; //next payload is continuation of this one.
			i+=sl; //skip remaining bytes of framepayload, we have handled this
			ws->priv->fr.len-=sl;
			if (ws->priv->fr.len==0) ws->priv->wsStatus=ST_FLAGS;
		}
	}
	httpdFlushSendBuffer(ws->conn);
	return 0;
}

//Websocket 'cgi' implementation
int ICACHE_FLASH_ATTR cgiWebsocket(HttpdConnData *connData) {
	char buff[256];
	int i;
	sha1nfo s;
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
//		os_printf("WS: Cleanup\n");
		if (connData->cgiPrivData) {
			Websock *ws=(Websock*)connData->cgiPrivData;
			if (ws->closeCb) ws->closeCb(ws);
			//Clean up linked list
			if (llStart==ws) {
				llStart=ws->priv->next;
			} else if (llStart) {
				Websock *lws=llStart;
				//Find ws that links to this one.
				while (lws!=NULL && lws->priv->next!=ws) lws=lws->priv->next;
				if (lws!=NULL) lws->priv->next=ws->priv->next;
			}
			if (ws->priv) os_free(ws->priv);
			os_free(connData->cgiPrivData);
			connData->cgiPrivData=NULL;
		}
		return HTTPD_CGI_DONE;
	}
	
	if (connData->cgiPrivData==NULL) {
//		os_printf("WS: First call\n");
		//First call here. Check if client headers are OK, send server header.
		i=httpdGetHeader(connData, "Upgrade", buff, sizeof(buff)-1);
//		os_printf("WS: Upgrade: %s\n", buff);
		if (i && os_strcmp(buff, "websocket")==0) {
			i=httpdGetHeader(connData, "Sec-WebSocket-Key", buff, sizeof(buff)-1);
			if (i) {
//				os_printf("WS: Key: %s\n", buff);
				//Seems like a WebSocket connection.
				// Alloc structs
				connData->cgiPrivData=os_malloc(sizeof(Websock));
				os_memset(connData->cgiPrivData, 0, sizeof(Websock));
				Websock *ws=(Websock*)connData->cgiPrivData;
				ws->priv=os_malloc(sizeof(WebsockPriv));
				os_memset(ws->priv, 0, sizeof(WebsockPriv));
				ws->conn=connData;
				//Reply with the right headers.
				os_strcat(buff, WS_GUID);
				sha1_init(&s);
				sha1_write(&s, buff, os_strlen(buff));
				httpdStartResponse(connData, 101);
				httpdHeader(connData, "Upgrade", "websocket");
				httpdHeader(connData, "Connection", "upgrade");
				base64_encode(20, sha1_result(&s), sizeof(buff), buff);
				httpdHeader(connData, "Sec-WebSocket-Accept", buff);
				httpdEndHeaders(connData);
				//Set data receive handler
				connData->recvHdl=cgiWebSocketRecv;
				//Inform CGI function we have a connection
				WsConnectedCb connCb=connData->cgiArg;
				connCb(ws);
				//Insert ws into linked list
				if (llStart==NULL) {
					llStart=ws;
				} else {
					Websock *lw=llStart;
					while (lw->priv->next) lw=lw->priv->next;
					lw->priv->next=ws;
				}
				return HTTPD_CGI_MORE;
			}
		}
		//No valid websocket connection
		httpdStartResponse(connData, 500);
		httpdEndHeaders(connData);
		return HTTPD_CGI_DONE;
	}
	
	//Sending is done. Call the sent callback if we have one.
	Websock *ws=(Websock*)connData->cgiPrivData;
	if (ws && ws->sentCb) ws->sentCb(ws);
	
	if (ws && ws->priv->mustClose) return HTTPD_CGI_DONE;
	
	return HTTPD_CGI_MORE;
}

