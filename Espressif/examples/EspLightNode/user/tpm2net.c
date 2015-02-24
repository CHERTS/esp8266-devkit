/*
 * tpm2net.c
 *
 *  Created on: Nov 18, 2014
 *      Author: frans-willem
 */
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"

#include "ws2801.h"

static void ICACHE_FLASH_ATTR tpm2net_recv(void *arg, char *pusrdata, unsigned short length) {
	unsigned char *data =(unsigned char *)pusrdata;
	if (data && length>=6 && data[0]==0x9C) {
		uint8_t blocktype = data[1];
		uint16_t framelength = ((uint16_t)data[2] << 8) | (uint16_t)data[3];
		uint8_t packagenum = data[4];
		uint8_t numpackages = data[5];
		if (length >= framelength + 7 && data[6+framelength]==0x36) {
			unsigned char *frame = &data[6];
			if (blocktype == 0xDA) {
				ws2801_strip(frame,framelength);
			}
		} else {
			//Invalid length or invalid end byte
		}
	} else {
		//Not a valid header
	}
}

void tpm2net_init() {
	static struct espconn tpm2conn;
	static esp_udp tpm2udp;

	tpm2conn.type = ESPCONN_UDP;
	tpm2conn.state = ESPCONN_NONE;
	tpm2conn.proto.udp = &tpm2udp;
	tpm2udp.local_port=0xFFE2;
	tpm2conn.reverse = NULL;
	espconn_regist_recvcb(&tpm2conn, tpm2net_recv);
	espconn_create(&tpm2conn);
	#ifdef PLATFORM_DEBUG
	ets_uart_printf("TPM2Net init done.\r\n");
	#endif
}
