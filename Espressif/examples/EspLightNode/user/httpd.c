/*
 * httpd.c

 *
 *  Created on: Nov 19, 2014
 *      Author: frans-willem
 */
#include <math.h>
#include <string.h>
#include <osapi.h>
#include "httpd.h"
#include "config.h"

#define HTTPD_PORT	80

struct HttpdConnectionSlot {
	struct espconn *conn;
	uint8_t state;
	uint8_t buffer[HTTPD_BUFFER_SIZE];
	uint16_t bufferLen;
	uint8_t verb;
	char uri[HTTPD_URI_SIZE];
	uint16_t contentLen;
	uint8_t sending;
	httpd_slot_sent_callback sentcb;
	void *sentcbdata;
};

struct HttpdConnectionSlot httpd_conns[HTTPD_MAX_CONN];

struct HttpdConnectionSlot *httpd_find_slot(struct espconn *conn) {
	unsigned int i;
	for (i=0; i<HTTPD_MAX_CONN; i++) {
		if (httpd_conns[i].conn == conn) {
			return &httpd_conns[i];
		}
	}
	return NULL;
}

struct HttpdConnectionSlot *httpd_find_free_slot() {
	return httpd_find_slot(NULL);
}

void httpd_debug(struct HttpdConnectionSlot *slot, char *data) {
	espconn_sent(slot->conn, data, strlen(data));
}

void httpd_add_buffer(struct HttpdConnectionSlot *slot, uint8_t *data, uint16_t len) {
	uint16_t copy = HTTPD_BUFFER_SIZE - slot->bufferLen;
	if (copy > len) copy = len;
	memcpy(&slot->buffer[slot->bufferLen], data, copy);
	slot->bufferLen+=copy;
}

void httpd_add_buffer_string(struct HttpdConnectionSlot *slot, char *str) {
	httpd_add_buffer(slot,str,strlen(str));
}

void httpd_set_error_state(struct HttpdConnectionSlot *slot, uint16_t code, char *codemessage, char *message) {
	char strTemp[32];
	uint16_t messagelen=strlen(message);
	slot->bufferLen = 0;
	httpd_add_buffer_string(slot,"HTTP/1.0 ");
	os_sprintf(strTemp,"%d ",code);
	httpd_add_buffer_string(slot,strTemp);
	httpd_add_buffer_string(slot,codemessage);
	httpd_add_buffer_string(slot,"\r\nContent-Length: ");
	os_sprintf(strTemp,"%d ",messagelen);
	httpd_add_buffer_string(slot,strTemp);
	httpd_add_buffer_string(slot,"\r\nContent-Type: text/plain\r\n\r\n");
	httpd_add_buffer_string(slot,message);

	slot->state = HTTPD_STATE_RESPONDING;
	espconn_sent(slot->conn, slot->buffer, slot->bufferLen);
}

void httpd_process_verb(struct HttpdConnectionSlot *slot, uint8_t *line, uint16_t len) {
	uint16_t spaces[4];
	uint16_t numspaces=0;
	uint16_t i;
	for (i=0; i<len && numspaces<3; i++) {
		if (line[i] == ' ') {
			spaces[numspaces]=i;
			numspaces++;
		}
	}
	//Set final space (not counted in numspaces) to length of line, to ease calculation.
	spaces[numspaces]=len;
	if (numspaces < 2) {
		httpd_set_error_state(slot, 400, "Bad Request","Invalid number of parts to VERB");
		return;
	}
	//Parse actual verb
	if (spaces[0] == 3 && memcmp(line,"GET",3)==0) {
		slot->verb = HTTPD_VERB_GET;
	} else if (spaces[0] == 4 && memcmp(line,"POST",4)==0) {
		slot->verb = HTTPD_VERB_POST;
	} else {
		httpd_set_error_state(slot, 400, "Bad Request","Invalid verb");
		return;
	}
	//Parse URL
	uint16_t urilen = spaces[1] - (spaces[0]+1);
	if (urilen + 1 > HTTPD_URI_SIZE) {
		httpd_set_error_state(slot, 414, "Request-URI Too Long", "Request-URI Too Long");
		return;
	}
	memcpy(slot->uri, &line[spaces[0]+1], urilen);
	slot->uri[urilen]='\0';
	//Parse headers now :)
	slot->state = HTTPD_STATE_PARSEHDRS;
}

void httpd_process_header(struct HttpdConnectionSlot *slot, uint8_t *line, uint16_t len) {
	if (len == 0) {
		if (slot->verb == HTTPD_VERB_POST && slot->contentLen > HTTPD_BUFFER_SIZE) httpd_set_error_state(slot,400,"Bad Request","Too much post data");
		else if (slot->verb == HTTPD_VERB_POST && slot->contentLen > 0) slot->state = HTTPD_STATE_WAITPOST;
		else slot->state = HTTPD_STATE_READY;
		return;
	}
	uint16_t split;
	for (split=0; split<len && line[split]!=':'; split++);
	if (split == len) {
		//Not sure what to do with this, error?
		return;
	}
	line[split]='\0';
	uint16_t valuestart = split+1;
	if (valuestart < len && line[valuestart]==' ') valuestart++;

	if (strcmpi(line,"Content-length")) {
		slot->contentLen=atoi(&line[valuestart]);
	}
}

//Returns how much data was removed
uint16_t httpd_process_buffer(struct HttpdConnectionSlot *slot) {
	uint16_t processed = 0, oldprocessed,endline,startline;
	do {
		oldprocessed = processed;
		switch (slot->state) {
		case HTTPD_STATE_RESPONDING:
			return 0; //Don't bother doing anything else, buffer now contains an error message, socket should be closed ASAP.
		case HTTPD_STATE_PARSEVERB:
		case HTTPD_STATE_PARSEHDRS:
			for (endline=processed; endline < slot->bufferLen && slot->buffer[endline] != '\n'; endline++);
			if (endline != slot->bufferLen) {
				//Remember the line starting point
				startline = processed;
				//Mark as processed
				processed = endline+1;
				//If the final character is a linefeed, ignore it.
				if (endline>startline && slot->buffer[endline-1]=='\r') endline--;
				//Overwrite newline or linefeed with \0 so we can use string functions inside header parsing.
				slot->buffer[endline]='\0';
				//Process verb or header
				if (slot->state == HTTPD_STATE_PARSEVERB)
					httpd_process_verb(slot, &slot->buffer[startline], endline-startline);
				else
					httpd_process_header(slot, &slot->buffer[startline], endline-startline);
			}
			break;
		case HTTPD_STATE_WAITPOST:
			//Do we have enough data for the post request?
			if (slot->bufferLen >= slot->contentLen) {
				//If so, signal that we are ready to handle this request.
				slot->state = HTTPD_STATE_READY;
			}
			break;
		}
	} while (processed != oldprocessed);
	//Discard all processed data
	if (processed > 0) {
		slot->bufferLen -= processed;
		memcpy(slot->buffer, &slot->buffer[processed], slot->bufferLen);
	}
	return processed;
}

void httpd_recv_callback(void *arg, char *pdata, unsigned short len) {
	struct HttpdConnectionSlot *slot = httpd_find_slot((struct espconn *)arg);
	if (slot == NULL) {
		espconn_disconnect((struct espconn *)arg);
		return;
	}
	uint16_t copy;
	while (len > 0 && slot->state != HTTPD_STATE_READY && slot->state != HTTPD_STATE_RESPONDING) {
		copy = HTTPD_BUFFER_SIZE - slot->bufferLen;
		if (copy == 0) {
			httpd_set_error_state(slot,400,"Bad Request","Buffer overflow");
			break;
		}
		if (copy > len) copy = len;
		memcpy(&slot->buffer[slot->bufferLen], pdata, copy);
		slot->bufferLen+=copy;
		pdata+=copy;
		len-=copy;
		httpd_process_buffer(slot);
	}
	if (slot->state == HTTPD_STATE_READY) {
		//Means all input has been parsed, and we should do something with it.
		if (strcmp(slot->uri,"/") == 0 || strcmp(slot->uri,"/index.html")==0) {
			slot->state = HTTPD_STATE_RESPONDING;
			config_html(slot);
		} else {
			httpd_set_error_state(slot,404,"Not found","Not found");
		}
	}
}

void httpd_sent_callback(void *arg) {
	struct HttpdConnectionSlot *slot = httpd_find_slot((struct espconn *)arg);
	if (slot == NULL) {
		espconn_disconnect((struct espconn *)arg);
		return;
	}
	slot->sending = 0; //TODO: Will sent_callback be called once, or for each espconn_sent ?
	if (slot->state == HTTPD_STATE_RESPONDING) {
		if (slot->sentcb) {
			slot->sentcb(slot, slot->sentcbdata);
		}
		if (slot->sending == 0) {
			espconn_disconnect(slot->conn);
			slot->conn = NULL;
		}
	}
}

void httpd_slot_send(struct HttpdConnectionSlot *slot, uint8_t *data, uint16_t len) {
	slot->sending = 1;
	espconn_sent(slot->conn, (uint8 *)data, (uint16)len);
}
void httpd_slot_setsentcb(struct HttpdConnectionSlot *slot, httpd_slot_sent_callback sentcb, void *data) {
	slot->sentcb = sentcb;
	slot->sentcbdata = data;
}
void httpd_slot_setdone(struct HttpdConnectionSlot *slot) {
	httpd_slot_setsentcb(slot, NULL, 0);
}

void httpd_disconnect_callback(void *arg) {
	//arg is wrong, so we need to manually loop over all slots.
	unsigned int i;
	for (i=0; i<HTTPD_MAX_CONN; i++) {
		if (httpd_conns[i].conn && (httpd_conns[i].conn->state == ESPCONN_NONE || httpd_conns[i].conn->state == httpd_conns[i].conn->state == ESPCONN_CLOSE)) {
			httpd_conns[i].conn = NULL;
		}
	}
}

void httpd_connect_callback(void *arg) {
	struct espconn *conn=(struct espconn *)arg;
	struct HttpdConnectionSlot *slot = httpd_find_free_slot();
	if (slot == NULL) {
		//If no slot was found, we've reached the maximum number of connections. Bummer.
		espconn_disconnect(conn);
		return;
	}
	slot->conn = conn;
	slot->bufferLen = 0;
	slot->state = HTTPD_STATE_PARSEVERB;
	slot->contentLen = 0;
	slot->sentcb = NULL;
	slot->sending = 0;
	espconn_regist_recvcb(conn, httpd_recv_callback);
	espconn_regist_disconcb(conn, httpd_disconnect_callback);
	espconn_regist_sentcb(conn, httpd_sent_callback);
}

void httpd_init() {
	static struct espconn httpdconn;
	static esp_tcp httpdtcp;

	memset(&httpdconn,0,sizeof(struct espconn));
	memset(&httpdtcp,0,sizeof(esp_tcp));
	httpdconn.type=ESPCONN_TCP;
	httpdconn.state=ESPCONN_NONE;
	httpdtcp.local_port=HTTPD_PORT;
	httpdconn.proto.tcp=&httpdtcp;

	espconn_regist_connectcb(&httpdconn, httpd_connect_callback);
	espconn_accept(&httpdconn);

	#ifdef PLATFORM_DEBUG
	ets_uart_printf("HTTP server init done.\r\n");
	#endif
}
