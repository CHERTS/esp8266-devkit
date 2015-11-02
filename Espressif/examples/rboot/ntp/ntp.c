//////////////////////////////////////////////////
// Simple NTP client for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#include <c_types.h>
#include <user_interface.h>
#include <espconn.h>
#include <osapi.h>
#include <mem.h>
#include <time.h>

#include "include\ntp.h"

// list of major public servers http://tf.nist.gov/tf-cgi/servers.cgi
uint8 ntp_server[] = {131, 107, 13, 100}; // microsoft

static os_timer_t ntp_timeout;
static struct espconn *pCon = 0;

static void ICACHE_FLASH_ATTR ntp_udp_timeout(void *arg) {
	
	os_timer_disarm(&ntp_timeout);
	uart0_tx_buffer("ntp timout\r\n", 12);

	// clean up connection
	if (pCon) {
		espconn_delete(pCon);
		os_free(pCon->proto.udp);
		os_free(pCon);
		pCon = 0;
	}
}

static void ICACHE_FLASH_ATTR ntp_udp_recv(void *arg, char *pdata, unsigned short len) {
	
	struct tm *dt;
	time_t timestamp;
	ntp_t *ntp;

	os_timer_disarm(&ntp_timeout);

	// extract ntp time
	ntp = (ntp_t*)pdata;
	timestamp = ntp->trans_time[0] << 24 | ntp->trans_time[1] << 16 |ntp->trans_time[2] << 8 | ntp->trans_time[3];
	// convert to unix time
	timestamp -= 2208988800UL;
	// create tm struct
	dt = gmtime(&timestamp);

	// do something with it, like setting an rtc
	//ds1307_setTime(dt);
	// or just print it out
	char timestr[11];
	os_sprintf(timestr, "%02d:%02d:%02d\r\n", dt->tm_hour, dt->tm_min, dt->tm_sec);
	uart0_tx_buffer(timestr, 10);

	// clean up connection
	if (pCon) {
		espconn_delete(pCon);
		os_free(pCon->proto.udp);
		os_free(pCon);
		pCon = 0;
	}
}

void ICACHE_FLASH_ATTR ntp_get_time() {

	ntp_t ntp;

	// set up the udp "connection"
	pCon = (struct espconn*)os_zalloc(sizeof(struct espconn));
	pCon->type = ESPCONN_UDP;
	pCon->state = ESPCONN_NONE;
	pCon->proto.udp = (esp_udp*)os_zalloc(sizeof(esp_udp));
	pCon->proto.udp->local_port = espconn_port();
	pCon->proto.udp->remote_port = 123;
	os_memcpy(pCon->proto.udp->remote_ip, ntp_server, 4);

	// create a really simple ntp request packet
	os_memset(&ntp, 0, sizeof(ntp_t));
	ntp.options = 0b00100011; // leap = 0, version = 4, mode = 3 (client)

	// set timeout timer
	os_timer_disarm(&ntp_timeout);
	os_timer_setfn(&ntp_timeout, (os_timer_func_t*)ntp_udp_timeout, pCon);
	os_timer_arm(&ntp_timeout, NTP_TIMEOUT_MS, 0);

	// send the ntp request
	espconn_create(pCon);
	espconn_regist_recvcb(pCon, ntp_udp_recv);
	espconn_sent(pCon, (uint8*)&ntp, sizeof(ntp_t));
}
