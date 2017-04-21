/*
Some flash handling cgi routines. Used for updating the ESPFS/OTA image.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

//This doesn't work yet on the ESP32. ToDo: figure out how to make it work.
#ifndef ESP32

#include <esp8266.h>
#include "cgiflash.h"
#include "espfs.h"
#include "cgiflash.h"
#include "espfs.h"

//#include <osapi.h>
#include "cgiflash.h"
#include "espfs.h"

#ifndef UPGRADE_FLAG_FINISH
#define UPGRADE_FLAG_FINISH     0x02
#endif

// Check that the header of the firmware blob looks like actual firmware...
static int ICACHE_FLASH_ATTR checkBinHeader(void *buf) {
	uint8_t *cd = (uint8_t *)buf;
	if (cd[0] != 0xEA) return 0;
	if (cd[1] != 4 || cd[2] > 3 || cd[3] > 0x40) return 0;
	if (((uint16_t *)buf)[3] != 0x4010) return 0;
	if (((uint32_t *)buf)[2] != 0) return 0;
	return 1;
}

static int ICACHE_FLASH_ATTR checkEspfsHeader(void *buf) {
	if (memcmp(buf, "ESfs", 4)!=0) return 0;
	return 1;
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
	httpd_printf("Next firmware: %s (got %d)\n", next, id);
	return HTTPD_CGI_DONE;
}


//Cgi that allows the firmware to be replaced via http POST This takes
//a direct POST from e.g. Curl or a Javascript AJAX call with either the
//firmware given by cgiGetFirmwareNext or an OTA upgrade image.

//Because we don't have the buffer to allocate an entire sector but will 
//have to buffer some data because the post buffer may be misaligned, we 
//write SPI data in pages. The page size is a software thing, not
//a hardware one.
#define PAGELEN 64

#define FLST_START 0
#define FLST_WRITE 1
#define FLST_SKIP 2
#define FLST_DONE 3
#define FLST_ERROR 4

#define FILETYPE_ESPFS 0
#define FILETYPE_FLASH 1
#define FILETYPE_OTA 2
typedef struct {
	int state;
	int filetype;
	int flashPos;
	char pageData[PAGELEN];
	int pagePos;
	int address;
	int len;
	int skip;
	char *err;
} UploadState;

typedef struct __attribute__((packed)) {
	char magic[4];
	char tag[28];
	int32_t len1;
	int32_t len2;
} OtaHeader;


int ICACHE_FLASH_ATTR cgiUploadFirmware(HttpdConnData *connData) {
	CgiUploadFlashDef *def=(CgiUploadFlashDef*)connData->cgiArg;
	UploadState *state=(UploadState *)connData->cgiData;
	int len;
	char buff[128];

	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		if (state!=NULL) free(state);
		return HTTPD_CGI_DONE;
	}

	if (state==NULL) {
		//First call. Allocate and initialize state variable.
		httpd_printf("Firmware upload cgi start.\n");
		state=malloc(sizeof(UploadState));
		if (state==NULL) {
			httpd_printf("Can't allocate firmware upload struct!\n");
			return HTTPD_CGI_DONE;
		}
		memset(state, 0, sizeof(UploadState));
		state->state=FLST_START;
		connData->cgiData=state;
		state->err="Premature end";
	}
	
	char *data=connData->post->buff;
	int dataLen=connData->post->buffLen;
	
	while (dataLen!=0) {
		if (state->state==FLST_START) {
			//First call. Assume the header of whatever we're uploading already is in the POST buffer.
			if (def->type==CGIFLASH_TYPE_FW && memcmp(data, "EHUG", 4)==0) {
				//Type is combined flash1/flash2 file
				OtaHeader *h=(OtaHeader*)data;
				strncpy(buff, h->tag, 27);
				buff[27]=0;
				if (strcmp(buff, def->tagName)!=0) {
					httpd_printf("OTA tag mismatch! Current=`%s` uploaded=`%s`.\n",
										def->tagName, buff);
					len=httpdFindArg(connData->getArgs, "force", buff, sizeof(buff));
					if (len!=-1 && atoi(buff)) {
						httpd_printf("Forcing firmware flash.\n");
					} else {
						state->err="Firmware not intended for this device!\n";
						state->state=FLST_ERROR;
					}
				}
				if (state->state!=FLST_ERROR && connData->post->len > def->fwSize*2+sizeof(OtaHeader)) {
					state->err="Firmware image too large";
					state->state=FLST_ERROR;
				}
				if (state->state!=FLST_ERROR) {
					//Flash header seems okay.
					dataLen-=sizeof(OtaHeader); //skip header when parsing data
					data+=sizeof(OtaHeader);
					if (system_upgrade_userbin_check()==1) {
						httpd_printf("Flashing user1.bin from ota image\n");
						state->len=h->len1;
						state->skip=h->len2;
						state->state=FLST_WRITE;
						state->address=def->fw1Pos;
					} else {
						httpd_printf("Flashing user2.bin from ota image\n");
						state->len=h->len2;
						state->skip=h->len1;
						state->state=FLST_SKIP;
						state->address=def->fw2Pos;
					}
				}
			} else if (def->type==CGIFLASH_TYPE_FW && checkBinHeader(connData->post->buff)) {
				if (connData->post->len > def->fwSize) {
					state->err="Firmware image too large";
					state->state=FLST_ERROR;
				} else {
					state->len=connData->post->len;
					state->address=def->fw1Pos;
					state->state=FLST_WRITE;
				}
			} else if (def->type==CGIFLASH_TYPE_ESPFS && checkEspfsHeader(connData->post->buff)) {
				if (connData->post->len > def->fwSize) {
					state->err="Firmware image too large";
					state->state=FLST_ERROR;
				} else {
					state->len=connData->post->len;
					state->address=def->fw1Pos;
					state->state=FLST_WRITE;
				}
			} else {
				state->err="Invalid flash image type!";
				state->state=FLST_ERROR;
				httpd_printf("Did not recognize flash image type!\n");
			}
		} else if (state->state==FLST_SKIP) {
			//Skip bytes without doing anything with them
			if (state->skip>dataLen) {
				//Skip entire buffer
				state->skip-=dataLen;
				dataLen=0;
			} else {
				//Only skip part of buffer
				dataLen-=state->skip;
				data+=state->skip;
				state->skip=0;
				if (state->len) state->state=FLST_WRITE; else state->state=FLST_DONE;
			}
		} else if (state->state==FLST_WRITE) {
			//Copy bytes to page buffer, and if page buffer is full, flash the data.
			//First, calculate the amount of bytes we need to finish the page buffer.
			int lenLeft=PAGELEN-state->pagePos;
			if (state->len<lenLeft) lenLeft=state->len; //last buffer can be a cut-off one
			//See if we need to write the page.
			if (dataLen<lenLeft) {
				//Page isn't done yet. Copy data to buffer and exit.
				memcpy(&state->pageData[state->pagePos], data, dataLen);
				state->pagePos+=dataLen;
				state->len-=dataLen;
				dataLen=0;
			} else {
				//Finish page; take data we need from post buffer
				memcpy(&state->pageData[state->pagePos], data, lenLeft);
				data+=lenLeft;
				dataLen-=lenLeft;
				state->pagePos+=lenLeft;
				state->len-=lenLeft;
				//Erase sector, if needed
				if ((state->address&(SPI_FLASH_SEC_SIZE-1))==0) {
					spi_flash_erase_sector(state->address/SPI_FLASH_SEC_SIZE);
				}
				//Write page
				//httpd_printf("Writing %d bytes of data to SPI pos 0x%x...\n", state->pagePos, state->address);
				spi_flash_write(state->address, (uint32 *)state->pageData, state->pagePos);
				state->address+=PAGELEN;
				state->pagePos=0;
				if (state->len==0) {
					//Done.
					if (state->skip) state->state=FLST_SKIP; else state->state=FLST_DONE;
				}
			}
		} else if (state->state==FLST_DONE) {
			httpd_printf("Huh? %d bogus bytes received after data received.\n", dataLen);
			//Ignore those bytes.
			dataLen=0;
		} else if (state->state==FLST_ERROR) {
			//Just eat up any bytes we receive.
			dataLen=0;
		}
	}
	
	if (connData->post->len==connData->post->received) {
		//We're done! Format a response.
		httpd_printf("Upload done. Sending response.\n");
		httpdStartResponse(connData, state->state==FLST_ERROR?400:200);
		httpdHeader(connData, "Content-Type", "text/plain");
		httpdEndHeaders(connData);
		if (state->state!=FLST_DONE) {
			httpdSend(connData, "Firmware image error:", -1);
			httpdSend(connData, state->err, -1);
			httpdSend(connData, "\n", -1);
		}
		free(state);
		return HTTPD_CGI_DONE;
	}

	return HTTPD_CGI_MORE;
}



static os_timer_t resetTimer;

static void ICACHE_FLASH_ATTR resetTimerCb(void *arg) {
	system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
	system_upgrade_reboot();
}

// Handle request to reboot into the new firmware
int ICACHE_FLASH_ATTR cgiRebootFirmware(HttpdConnData *connData) {
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	// TODO: sanity-check that the 'next' partition actually contains something that looks like
	// valid firmware

	//Do reboot in a timer callback so we still have time to send the response.
	os_timer_disarm(&resetTimer);
	os_timer_setfn(&resetTimer, resetTimerCb, NULL);
	os_timer_arm(&resetTimer, 200, 0);

	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/plain");
	httpdEndHeaders(connData);
	httpdSend(connData, "Rebooting...", -1);
	return HTTPD_CGI_DONE;
}

#endif