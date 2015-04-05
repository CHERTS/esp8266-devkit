/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Martin d'Allens <martin.dallens@gmail.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */

// FIXME: sprintf->snprintf everywhere.
// FIXME: support null characters in responses.

#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "httpclient.h"

// Debug output.
#ifdef HTTP_DEBUG
#undef HTTP_DEBUG
#define HTTP_DEBUG(...) os_printf(__VA_ARGS__);
#else
#define HTTP_DEBUG(...)
#endif

// enum espconn state, see file /include/lwip/api/err.c
const char *sEspconnErr[] =
{
		"Ok",                    // ERR_OK          0
		"Out of memory error",   // ERR_MEM        -1
		"Buffer error",          // ERR_BUF        -2
		"Timeout",               // ERR_TIMEOUT    -3
		"Routing problem",       // ERR_RTE        -4
		"Operation in progress", // ERR_INPROGRESS -5
		"Illegal value",         // ERR_VAL        -6
		"Operation would block", // ERR_WOULDBLOCK -7
		"Connection aborted",    // ERR_ABRT       -8
		"Connection reset",      // ERR_RST        -9
		"Connection closed",     // ERR_CLSD       -10
		"Not connected",         // ERR_CONN       -11
		"Illegal argument",      // ERR_ARG        -12
		"Address in use",        // ERR_USE        -13
		"Low-level netif error", // ERR_IF         -14
		"Already connected"      // ERR_ISCONN     -15
};

// Internal state.
typedef struct {
	char * path;
	int port;
	char * post_data;
	char * hostname;
	char * buffer;
	int buffer_size;
	http_callback user_callback;
} request_args;

static char * esp_strdup(const char * str)
{
	if (str == NULL) {
		return NULL;
	}
	char * new_str = (char *)os_malloc(os_strlen(str) + 1); // 1 for null character
	if (new_str == NULL) {
		HTTP_DEBUG("esp_strdup: malloc error");
		return NULL;
	}
	os_strcpy(new_str, str);
	return new_str;
}

static void ICACHE_FLASH_ATTR receive_callback(void * arg, char * buf, unsigned short len)
{
	struct espconn * conn = (struct espconn *)arg;
	request_args * req = (request_args *)conn->reverse;

	if (req->buffer == NULL) {
		return;
	}

	// Let's do the equivalent of a realloc().
	const int new_size = req->buffer_size + len;
	char * new_buffer;
	if (new_size > BUFFER_SIZE_MAX || NULL == (new_buffer = (char *)os_malloc(new_size))) {
		HTTP_DEBUG("Response too long %d\n", new_size);
		os_free(req->buffer);
		req->buffer = NULL;
		// TODO: espconn_disconnect(conn) without crashing.
		return;
	}

	os_memcpy(new_buffer, req->buffer, req->buffer_size);
	os_memcpy(new_buffer + req->buffer_size - 1 /*overwrite the null character*/, buf, len); // Append new data.
	new_buffer[new_size - 1] = '\0'; // Make sure there is an end of string.

	os_free(req->buffer);
	req->buffer = new_buffer;
	req->buffer_size = new_size;
}

static void ICACHE_FLASH_ATTR sent_callback(void * arg)
{
	struct espconn * conn = (struct espconn *)arg;
	request_args * req = (request_args *)conn->reverse;

	if (req->post_data == NULL) {
		HTTP_DEBUG("All sent\n");
	}
	else {
		// The headers were sent, now send the contents.
		HTTP_DEBUG("Sending request body\n");
		sint8 espsent_status = espconn_sent(conn, (uint8_t *)req->post_data, strlen(req->post_data));
		if(espsent_status == ESPCONN_OK) {
			HTTP_DEBUG("Request body sent, req->post_data = %s\n", req->post_data);
		} else {
			HTTP_DEBUG("Error while sending request body.\n");
		}
		os_free(req->post_data);
		req->post_data = NULL;
	}
}

static void ICACHE_FLASH_ATTR connect_callback(void * arg)
{
	HTTP_DEBUG("Connected\n");
	struct espconn * conn = (struct espconn *)arg;
	request_args * req = (request_args *)conn->reverse;

	espconn_regist_recvcb(conn, receive_callback);
	espconn_regist_sentcb(conn, sent_callback);

	const char * method = "GET";
	char post_headers[128] = "";

	if (req->post_data != NULL) { // If there is data this is a POST request.
		method = "POST";
		os_sprintf(post_headers,
				   "Content-Type: application/x-www-form-urlencoded\r\n"
				   "Content-Length: %d\r\n", strlen(req->post_data));
	}

	char buf[2048];
	int len = os_sprintf(buf,
						 "%s %s HTTP/1.1\r\n"
						 "Host: %s:%d\r\n"
						 "Connection: close\r\n"
						 "User-Agent: ESP8266\r\n"
						 "%s"
						 "\r\n",
						 method, req->path, req->hostname, req->port, post_headers);

	sint8 espsent_status = espconn_sent(conn, (uint8_t *)buf, len);
	if(espsent_status == ESPCONN_OK) {
		HTTP_DEBUG("Data sent, buf = %s\n", buf);
	} else {
		HTTP_DEBUG("Error while sending data.\n");
	}
	HTTP_DEBUG("Sending request header\n");
}

static void ICACHE_FLASH_ATTR disconnect_callback(void * arg)
{
	HTTP_DEBUG("Disconnected\n");
	struct espconn *conn = (struct espconn *)arg;

	if(conn == NULL) {
		return;
	}

	if(conn->proto.tcp != NULL) {
		os_free(conn->proto.tcp);
	}
	if(conn->reverse != NULL) {
		request_args * req = (request_args *)conn->reverse;
		if (req->buffer != NULL) {
			// FIXME: make sure this is not a partial response, using the Content-Length header.

			const char * version = "HTTP/1.1 ";
			if (os_strncmp(req->buffer, version, strlen(version)) != 0) {
				HTTP_DEBUG("Invalid version in %s\n", req->buffer);
				return;
			}
			int http_status = atoi(req->buffer + strlen(version));

			//char * body = (char *)os_strstr(req->buffer, "\r\n\r\n") + 4;
			char * body = (char *)os_strstr(req->buffer, "\r\n\r\n");
			if (body)
				body+=4;

			if (req->user_callback != NULL) { // Callback is optional.
				req->user_callback(body, http_status, req->buffer);
			}
			os_free(req->buffer);
		}
		os_free(req->hostname);
		os_free(req->path);
		os_free(req);
	}
	os_free(conn);
}

static void ICACHE_FLASH_ATTR reconnect_callback(void * arg, sint8 errType)
{
	struct espconn *conn = (struct espconn *)arg;
	HTTP_DEBUG("RECONNECT\n");
	if (errType != ESPCONN_OK)
		HTTP_DEBUG("Connection error: %d - %s\r\n", errType, ((errType>-16)&&(errType<1))? sEspconnErr[-errType] : "?");
	if(conn->proto.tcp != NULL)
	{
		os_free(conn->proto.tcp);
		HTTP_DEBUG("os_free: conn->proto.tcp\r\n");
	}
	os_free(conn);
	HTTP_DEBUG("os_free: conn\r\n");
}

static void ICACHE_FLASH_ATTR dns_callback(const char * hostname, ip_addr_t * addr, void * arg)
{
	request_args * req = (request_args *)arg;

	if (addr == NULL) {
		HTTP_DEBUG("DNS failed, host: %s\n", hostname);
	}
	else {
		HTTP_DEBUG("DNS found, host: %s, address: " IPSTR "\n", hostname, IP2STR(addr));

		struct espconn * conn = (struct espconn *)os_malloc(sizeof(struct espconn));
		conn->type = ESPCONN_TCP;
		conn->state = ESPCONN_NONE;
		conn->proto.tcp = (esp_tcp *)os_malloc(sizeof(esp_tcp));
		conn->proto.tcp->local_port = espconn_port();
		conn->proto.tcp->remote_port = req->port;
		conn->reverse = req;

		os_memcpy(conn->proto.tcp->remote_ip, addr, 4);

		espconn_regist_connectcb(conn, connect_callback);
		espconn_regist_disconcb(conn, disconnect_callback);
		espconn_regist_reconcb(conn, reconnect_callback);

		// TODO: consider using espconn_regist_reconcb (for timeouts?)
		// cf esp8266_sdk_v0.9.1/examples/at/user/at_ipCmd.c  (TCP ARQ retransmission?)

		sint8 espcon_status = espconn_connect(conn);
		switch(espcon_status)
		{
			case ESPCONN_OK:
				HTTP_DEBUG("TCP created.\r\n");
				break;
			case ESPCONN_RTE:
				HTTP_DEBUG("Error connection, routing problem.\r\n");
				break;
			case ESPCONN_TIMEOUT:
				HTTP_DEBUG("Error connection, timeout.\r\n");
				break;
			default:
				HTTP_DEBUG("Connection error: %d - %s\r\n", espcon_status, ((espcon_status>-16)&&(espcon_status<1))? sEspconnErr[-espcon_status] : "?");
		}
	}
}

void ICACHE_FLASH_ATTR http_raw_request(const char * hostname, int port, const char * path, const char * post_data, http_callback user_callback)
{
	HTTP_DEBUG("DNS request\n");

	request_args *req = (request_args *)os_malloc(sizeof(request_args));
	req->hostname = esp_strdup(hostname);
	req->path = esp_strdup(path);
	req->port = port;
	req->post_data = esp_strdup(post_data);
	req->buffer_size = 1;
	req->buffer = (char *)os_malloc(1);
	req->buffer[0] = '\0'; // Empty string.
	req->user_callback = user_callback;

	ip_addr_t addr;
	err_t error = espconn_gethostbyname((struct espconn *)req, // It seems we don't need a real espconn pointer here.
										hostname, &addr, dns_callback);

	if (error == ESPCONN_INPROGRESS) {
		HTTP_DEBUG("DNS pending\n");
	}
	else if (error == ESPCONN_OK) {
		// Already in the local names table (or hostname was an IP address), execute the callback ourselves.
		dns_callback(hostname, &addr, req);
	}
	else if (error == ESPCONN_ARG) {
		HTTP_DEBUG("DNS error %s\n", hostname);
	}
	else {
		HTTP_DEBUG("DNS error code %d\n", error);
	}
}

/*
 * Parse an URL of the form http://host:port/path
 * <host> can be a hostname or an IP address
 * <port> is optional
 */
void ICACHE_FLASH_ATTR http_post(const char * url, const char * post_data, http_callback user_callback)
{
	// FIXME: handle HTTP auth with http://user:pass@host/
	// FIXME: make https work.
	// FIXME: get rid of the #anchor part if present.

	char hostname[128] = "";
	int port = 80;

	if (os_strncmp(url, "http://", strlen("http://")) != 0) {
		HTTP_DEBUG("URL is not HTTP %s\n", url);
		return;
	}
	url += strlen("http://"); // Get rid of the protocol.

	char * path = os_strchr(url, '/');
	if (path == NULL) {
		path = os_strchr(url, '\0'); // Pointer to end of string.
	}

	char * colon = os_strchr(url, ':');
	if (colon > path) {
		colon = NULL; // Limit the search to characters before the path.
	}

	if (colon == NULL) { // The port is not present.
		os_memcpy(hostname, url, path - url);
		hostname[path - url] = '\0';
	}
	else {
		port = atoi(colon + 1);
		if (port == 0) {
			HTTP_DEBUG("Port error %s\n", url);
			return;
		}

		os_memcpy(hostname, url, colon - url);
		hostname[colon - url] = '\0';
	}


	if (path[0] == '\0') { // Empty path is not allowed.
		path = "/";
	}

	HTTP_DEBUG("hostname=%s\n", hostname);
	HTTP_DEBUG("port=%d\n", port);
	HTTP_DEBUG("path=%s\n", path);
	http_raw_request(hostname, port, path, post_data, user_callback);
}

void ICACHE_FLASH_ATTR http_get(const char * url, http_callback user_callback)
{
	http_post(url, NULL, user_callback);
}

/*
void http_test()
{
	http_get("https://google.com"); // Should fail.
	http_get("http://google.com/search?q=1");
	http_get("http://google.com");
	http_get("http://portquiz.net:8080/");
	http_raw_request("google.com", 80, "/search?q=2", NULL);
	http_get("http://173.194.45.65"); // Fails if not online yet. FIXME: we should wait for DHCP to have finished before connecting.
	http_get("http://wtfismyip.com/text", http_callback_example);
	http_post("http://httpbin.org/post", "first_word=hello&second_word=world", http_callback_example);
}
*/
