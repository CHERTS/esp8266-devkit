/*
 * httpd.h
 *
 *  Created on: Nov 19, 2014
 *      Author: frans-willem
 */

#ifndef CONFIG_HTTPD_H_
#define CONFIG_HTTPD_H_
#include <c_types.h>
#include <ip_addr.h>
#include <espconn.h>

#define HTTPD_MAX_CONN			8
#define HTTPD_BUFFER_SIZE		512
#define HTTPD_URI_SIZE			256

#define HTTPD_STATE_PARSEVERB	0 //Waiting for POST ... or GET ...
#define HTTPD_STATE_PARSEHDRS	1 //Processing headers
#define HTTPD_STATE_WAITPOST	2 //Waiting for post data
#define HTTPD_STATE_READY		3 //HTTP request was parsed completely and is ready for handling.
#define HTTPD_STATE_RESPONDING	4 //Done, no more data should be processed

#define HTTPD_VERB_GET	0
#define HTTPD_VERB_POST	1

struct HttpdConnectionSlot;
typedef uint8_t (* httpd_slot_sent_callback)(struct HttpdConnectionSlot *slot, void *data);
void httpd_slot_send(struct HttpdConnectionSlot *slot, uint8_t *data, uint16_t len);
void httpd_slot_setsentcb(struct HttpdConnectionSlot *slot, httpd_slot_sent_callback sentcb, void *data);
void httpd_slot_setdone(struct HttpdConnectionSlot *slot);

void httpd_init();

#endif /* CONFIG_HTTPD_H_ */
