/*
Some flash handling cgi routines. Used for reading the existing flash and updating the ESPFS image.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include <esp8266.h>
#include "cgiflash.h"
#include "espfs.h"
#include <osapi.h>
#include "cgiflash.h"
#include "espfs.h"

#include <osapi.h>
#include "cgiflash.h"
#include "espfs.h"

#define ESPFS_SIZE 0
#define FIRMWARE_SIZE 0


// Check that the header of the firmware blob looks like actual firmware...
static char* ICACHE_FLASH_ATTR checkBinHeader(void *buf) {
	uint8_t *cd = (uint8_t *)buf;
	if (cd[0] != 0xEA) return "IROM magic missing";
	if (cd[1] != 4 || cd[2] > 3 || cd[3] > 0x40) return "bad flash header";
	if (((uint16_t *)buf)[3] != 0x4010) return "Invalid entry addr";
	if (((uint32_t *)buf)[2] != 0) return "Invalid start offset";
	return NULL;
}

static char* ICACHE_FLASH_ATTR checkEspfsHeader(void *buf) {
	if (os_memcmp(buf, "ESfs", 4)!=0) return "Bad ESPfs header";
	return NULL;
}


// Cgi to query which firmware needs to be uploaded next
int ICACHE_FLASH_ATTR cgiGetFirmwareNext(HttpdConnData *connData) {
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	uint8 id = system_upgrade_userbin_check();
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/plain");
	httpdHeader(connData, "Content-Length", "9");
	httpdEndHeaders(connData);
	char *next = id == 1 ? "user1.bin" : "user2.bin";
	httpdSend(connData, next, -1);
	os_printf("Next firmware: %s (got %d)\n", next, id);
	return HTTPD_CGI_DONE;
}


//Cgi that reads the SPI flash. Assumes 512KByte flash.
//ToDo: Figure out real flash size somehow?
int ICACHE_FLASH_ATTR cgiReadFlash(HttpdConnData *connData) {
	int *pos=(int *)&connData->cgiData;
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	if (*pos==0) {
		os_printf("Start flash download.\n");
		httpdStartResponse(connData, 200);
		httpdHeader(connData, "Content-Type", "application/bin");
		httpdEndHeaders(connData);
		*pos=0x40200000;
		return HTTPD_CGI_MORE;
	}
	//Send 1K of flash per call. We will get called again if we haven't sent 512K yet.
	espconn_sent(connData->conn, (uint8 *)(*pos), 1024);
	*pos+=1024;
	if (*pos>=0x40200000+(512*1024)) return HTTPD_CGI_DONE; else return HTTPD_CGI_MORE;
}


//Cgi that allows the firmware to be replaced via http POST
int ICACHE_FLASH_ATTR cgiUploadFirmware(HttpdConnData *connData) {
	CgiUploadFlashDef *def=(CgiUploadFlashDef*)connData->cgiArg;
	uint32_t address;
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	int offset = connData->post->received - connData->post->buffLen;
	if (offset == 0) {
		connData->cgiPrivData = NULL;
	} else if (connData->cgiPrivData != NULL) {
		// we have an error condition, do nothing
		return HTTPD_CGI_DONE;
	}

	// assume no error yet...
	char *err = NULL;
	int code = 400;

	if (connData->post==NULL) err="No POST request.";
	if (def==NULL) err="Flash def = NULL ?";

	// check overall size
	os_printf("Max img sz 0x%X, post len 0x%X\n", def->fwSize, connData->post->len);
	if (err==NULL && connData->post->len > def->fwSize) err = "Firmware image too large";

	// check that data starts with an appropriate header
	if (err == NULL && offset == 0 && def->type==CGIFLASH_TYPE_FW) err = checkBinHeader(connData->post->buff);
	if (err == NULL && offset == 0 && def->type==CGIFLASH_TYPE_ESPFS) err = checkEspfsHeader(connData->post->buff);

	// make sure we're buffering in 1024 byte chunks
	if (err == NULL && offset % 1024 != 0) {
		err = "Buffering problem";
		code = 500;
	}

	// return an error if there is one
	if (err != NULL) {
		os_printf("Error %d: %s\n", code, err);
		httpdStartResponse(connData, code);
		httpdHeader(connData, "Content-Type", "text/plain");
		httpdEndHeaders(connData);
		httpdSend(connData, "Firmware image error:\r\n", -1);
		httpdSend(connData, err, -1);
		httpdSend(connData, "\r\n", -1);
		connData->cgiPrivData = (void *)1;
		return HTTPD_CGI_DONE;
	}

	// let's see which partition we need to flash and what flash address that puts us at
	int id=system_upgrade_userbin_check();
	if (id==1) address=def->fw1Pos; else address=def->fw2Pos;
	address += offset;
	// erase next flash block if necessary
	if (address % SPI_FLASH_SEC_SIZE == 0){
		// We need to erase this block
		os_printf("Erasing flash at 0x%05x (id=%d)\n", (unsigned int)address, 2-id);
		spi_flash_erase_sector(address/SPI_FLASH_SEC_SIZE);
	}

	// Write the data
	os_printf("Writing %d bytes at 0x%05x (%d of %d)\n", connData->post->buffSize, (unsigned int)address,
			connData->post->received, connData->post->len);
	spi_flash_write(address, (uint32 *)connData->post->buff, connData->post->buffLen);

	if (connData->post->received == connData->post->len){
		httpdStartResponse(connData, 200);
		httpdEndHeaders(connData);
		return HTTPD_CGI_DONE;
	} else {
		return HTTPD_CGI_MORE;
	}
}

//static ETSTimer flash_reboot_timer;

// Handle request to reboot into the new firmware
int ICACHE_FLASH_ATTR cgiRebootFirmware(HttpdConnData *connData) {
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	// TODO: sanity-check that the 'next' partition actually contains something that looks like
	// valid firmware

	// This should probably be forked into a separate task that waits a second to let the
	// current HTTP request finish...
	system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
	system_upgrade_reboot();
	httpdStartResponse(connData, 200);
	httpdEndHeaders(connData);
	return HTTPD_CGI_DONE;
}

