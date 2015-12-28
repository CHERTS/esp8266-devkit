/*
Some random cgi routines.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

#include <osapi.h>
#include "user_interface.h"
#include "httpd.h"
#include "cgi.h"
#include "espmissingincludes.h"
#include "pwm.h"

int ICACHE_FLASH_ATTR cgiPWM(HttpdConnData *connData) {
	int len;
	char buff[128];
	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/json");
	httpdEndHeaders(connData);

	len=httpdFindArg(connData->getArgs, "duty", buff, sizeof(buff));
	if (len>0) {
		pwm_set_duty(atoi(buff), 0);
		pwm_start();	
	
		len=os_sprintf(buff, "OK");
		httpdSend(connData, buff, -1);
		return HTTPD_CGI_DONE;
	} else { //with no parameters returns JSON with state

		len=os_sprintf(buff, "{\"pwm\": %d\n}\n", pwm_get_duty(0) );
		httpdSend(connData, buff, -1);
		return HTTPD_CGI_DONE;	
	}
}

