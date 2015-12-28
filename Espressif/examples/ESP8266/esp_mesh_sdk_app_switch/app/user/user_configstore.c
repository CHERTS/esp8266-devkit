#include "user_configstore.h"
#include "spi_flash.h"
#include <c_types.h>
#include <osapi.h>
#include "user_light_action.h"

MyConfig myConfig;
static int confLoc=0;

//Simple additive 8-bit checksum
static char calcChsum(MyConfig *conf) {
	char *p=(char*)conf;
	int x;
	char r=0x5a;
	for (x=1; x<sizeof(MyConfig); x++) r+=p[x];
	return r;
}

#define ESP_PARAM_SEC_A		0x79
#define ESP_PARAM_SEC_B		0x7A

//Load defaults in case no good config can be found (flash corrupted,
//or (more likely) this is the first bootup.)
void configLoadDefaults() {
	int i;
//	LampMacType maca={{0x18,0xfe,0x34, 0xa1,0x32,0xf7}, LST_ACTIVE};
//	LampMacType macb={{0x18,0xfe,0x34, 0xa0,0x3c,0x7c}, 0};
//	os_memcpy(&myConfig.macData[0], &maca, sizeof(LampMacType));
//	os_memcpy(&myConfig.macData[1], &macb, sizeof(LampMacType));
//	myConfig.wifiChan=6;

	//Default button values: red, green, blue, white, off
	myConfig.bval[0].r=22222;
	myConfig.bval[1].g=22222;
	myConfig.bval[2].b=22222;
	myConfig.bval[3].cw=22222;
	myConfig.bval[3].ww=22222;
	for (i=0; i<CONFIG_MAC_CNT; i++) myConfig.wlanChannel[i]=1;
}

void configLoad() {
	uint8_t c;
	int i;
	int aGood=0, bGood=0;
	MyConfig aConfig, bConfig;
	spi_flash_read(ESP_PARAM_SEC_A*SPI_FLASH_SEC_SIZE, (uint32 *)&aConfig, sizeof(MyConfig));
	spi_flash_read(ESP_PARAM_SEC_B*SPI_FLASH_SEC_SIZE, (uint32 *)&bConfig, sizeof(MyConfig));
	if (calcChsum(&aConfig)==aConfig.chsum) aGood=1;
	if (calcChsum(&bConfig)==bConfig.chsum) bGood=1;
	os_printf("Loading config. Status: A - %s (seq %d), b - %s (seq %d)\n", aGood?"OK":"Invalid", (int)aConfig.seq,  bGood?"OK":"Invalid", (int)bConfig.seq);
	if (aGood && bGood) {
		//Both configs are okay. Grab the latest one.
		c=bConfig.seq-aConfig.seq;
		if (c<128) {
			//B is newer.
			os_memcpy(&myConfig, &bConfig, sizeof(MyConfig));
			confLoc=1;
		} else {
			//A is newer.
			os_memcpy(&myConfig, &aConfig, sizeof(MyConfig));
			confLoc=0;
		}
	} else if (aGood) {
		//Only A is good. Get that one.
		os_memcpy(&myConfig, &aConfig, sizeof(MyConfig));
		confLoc=0;
	} else if (bGood) {
		//Only B is good. Get that one.
		os_memcpy(&myConfig, &bConfig, sizeof(MyConfig));
		confLoc=1;
	} else {
		//No good config. Set defaults.`
		os_memset(&myConfig, 0, sizeof(myConfig));
		configLoadDefaults();
		confLoc=1;
	}
	os_printf("Current config loc = %s\n", (confLoc==0)?"A":"B");
}

void configSave() {
	MyConfig tmpConfig;
	//Load the current active config
	if (confLoc==0) {
		spi_flash_read(ESP_PARAM_SEC_A*SPI_FLASH_SEC_SIZE, (uint32 *)&tmpConfig, sizeof(MyConfig));
	} else {
		spi_flash_read(ESP_PARAM_SEC_B*SPI_FLASH_SEC_SIZE, (uint32 *)&tmpConfig, sizeof(MyConfig));
	}
	//Don't save anything if nothing has changed.
	if (os_memcmp(&tmpConfig, &myConfig, sizeof(MyConfig))==0) return;

	//Okay, we need to save the config.
	myConfig.seq++;
	os_printf("MyConfig seq %d\n", (int)myConfig.seq);
	myConfig.chsum=calcChsum(&myConfig);
	//If the config came from sector A, save to sector B and vice-versa.
	if (confLoc==0) {
		spi_flash_erase_sector(ESP_PARAM_SEC_B);
		spi_flash_write(ESP_PARAM_SEC_B*SPI_FLASH_SEC_SIZE, (uint32 *)&myConfig, sizeof(MyConfig));
		confLoc=1;
	} else {
		spi_flash_erase_sector(ESP_PARAM_SEC_A);
		spi_flash_write(ESP_PARAM_SEC_A*SPI_FLASH_SEC_SIZE, (uint32 *)&myConfig, sizeof(MyConfig));
		confLoc=0;
	}
	os_printf("Saved config to loc %s\n", (confLoc==0)?"A":"B");
}

