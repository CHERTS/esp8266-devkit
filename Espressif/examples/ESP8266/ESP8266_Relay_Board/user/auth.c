/*
HTTP auth implementation. Only does basic authentication for now.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd.h"
#include "cgi.h"
#include "auth.h"
#include "io.h"
#include "base64.h"
#include "espmissingincludes.h"
#include <ip_addr.h>
#include "config.h"
static uint8_t remoteip[4];

int ICACHE_FLASH_ATTR authBasic(HttpdConnData *connData) {
	const char *forbidden="401 Forbidden.";
	int no=0;
	int r;
	char hdr[(AUTH_MAX_USER_LEN+AUTH_MAX_PASS_LEN+2)*10];
	char userpass[AUTH_MAX_USER_LEN+AUTH_MAX_PASS_LEN+2];
	char user[AUTH_MAX_USER_LEN];
	char pass[AUTH_MAX_PASS_LEN];
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	struct espconn *pespconn = connData->conn;
	
	r=httpdGetHeader(connData, "Authorization", hdr, sizeof(hdr));
	if (r && strncmp(hdr, "Basic", 5)==0) {
		r=base64_decode2(strlen(hdr)-6, hdr+6, sizeof(userpass), (unsigned char *)userpass);
		if (r<0) r=0; //just clean out string on decode error
		userpass[r]=0; //zero-terminate user:pass string
//		os_printf("Auth: %s\n", userpass);
		while (((AuthGetUserPw)(connData->cgiArg))(connData, no,
				user, AUTH_MAX_USER_LEN, pass, AUTH_MAX_PASS_LEN)) {
			//Check user/pass against auth header
			if (strlen(userpass)==strlen(user)+strlen(pass)+1 &&
					os_strncmp(userpass, user, strlen(user))==0 &&
					userpass[strlen(user)]==':' &&
					os_strcmp(userpass+strlen(user)+1, pass)==0) {
				//Authenticated. Yay!

				os_memcpy(&remoteip,pespconn->proto.tcp->remote_ip,4);
				return HTTPD_CGI_AUTHENTICATED;
			}
			no++; //Not authenticated with this user/pass. Check next user/pass combo.
		}
	}
	  
  
	if(sysCfg.httpd_auth==1 && (os_memcmp(remoteip,pespconn->proto.tcp->remote_ip,4)!=0) ) {
		//Not authenticated. Go bug user with login screen.
		httpdStartResponse(connData, 401);
		httpdHeader(connData, "Content-Type", "text/plain");
		httpdHeader(connData, "WWW-Authenticate", "Basic realm=\""HTTP_AUTH_REALM"\"");
		httpdEndHeaders(connData);
		httpdSend(connData, forbidden, -1);
		//Okay, all done.
		return HTTPD_CGI_DONE;
	}
	else 
		return HTTPD_CGI_AUTHENTICATED;
}

