//////////////////////////////////////////////////
// rBoot Sming sample project.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include <rboot-smingota.h>

static void ICACHE_FLASH_ATTR OtaUpdate_CallBack(bool result, uint8 rom_slot) {

	if(result == true) {
		// success
		if (rom_slot == FLASH_BY_ADDR) {
			Serial.println("Write successful.");
		} else {
			// set to boot new rom and then reboot
			Serial.printf("Firmware updated, rebooting to rom %d...\r\n", rom_slot);
			rboot_set_current_rom(rom_slot);
			System.restart();
		}
	} else {
		// fail
		Serial.println("Firmware update failed!");
	}
}

static void OtaUpdate() {
	
	// start the upgrade process
	if (rboot_ota_start((ota_callback)OtaUpdate_CallBack)) {
		Serial.println("Updating...");
	} else {
		Serial.println("Updating failed!\r\n");
	}
}

void Switch() {
	uint8 before, after;
	before = rboot_get_current_rom();
	if (before == 0) after = 1; else after = 0;
	Serial.printf("Swapping from rom %d to rom %d.\r\n", before, after);
	rboot_set_current_rom(after);
	Serial.println("Restarting...\r\n");
	System.restart();
}

void ShowInfo() {
    Serial.printf("\r\nSDK: v%s\r\n", system_get_sdk_version());
    Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
    Serial.printf("CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
    Serial.printf("System Chip ID: 0x%x\r\n", system_get_chip_id());
    Serial.printf("SPI Flash ID: 0x%x\r\n", spi_flash_get_id());
    //Serial.printf("SPI Flash Size: %d\r\n", (1 << ((spi_flash_get_id() >> 16) & 0xff)));
}

void serialCallBack(Stream& stream, char arrivedChar, unsigned short availableCharsCount) {
	

	if (arrivedChar == '\n') {
		char str[availableCharsCount];
		for (int i = 0; i < availableCharsCount; i++) {
			str[i] = stream.read();
			if (str[i] == '\r' || str[i] == '\n') {
				str[i] = '\0';
			}
		}
		
		if (!strcmp(str, "connect")) {
			// connect to wifi
			WifiStation.config(WIFI_SSID, WIFI_PWD);
			WifiStation.enable(true);
		} else if (!strcmp(str, "ip")) {
			Serial.printf("ip: %s mac: %s\r\n", WifiStation.getIP().toString().c_str(), WifiStation.getMAC().c_str());
		} else if (!strcmp(str, "ota")) {
			OtaUpdate();
		} else if (!strcmp(str, "switch")) {
			Switch();
		} else if (!strcmp(str, "restart")) {
			System.restart();
		} else if (!strcmp(str, "ls")) {
			Vector<String> files = fileList();
			Serial.printf("filecount %d\r\n", files.count());
			for (unsigned int i = 0; i < files.count(); i++) {
				Serial.println(files[i]);
			}
		} else if (!strcmp(str, "cat")) {
			Vector<String> files = fileList();
			if (files.count() > 0) {
				Serial.printf("dumping file %s:\r\n", files[0].c_str());
				Serial.println(fileGetContent(files[0]));
			} else {
				Serial.println("Empty spiffs!");
			}
		} else if (!strcmp(str, "info")) {
			ShowInfo();
		} else if (!strcmp(str, "help")) {
			Serial.println();
			Serial.println("available commands:");
			Serial.println("  help - display this message");
			Serial.println("  ip - show current ip address");
			Serial.println("  connect - connect to wifi");
			Serial.println("  restart - restart the esp8266");
			Serial.println("  switch - switch to the other rom and reboot");
			Serial.println("  ota - perform ota update, switch rom and reboot");
			Serial.println("  info - show esp8266 info");
			Serial.println("  ls - list files in spiffs");
			Serial.println("  cat - show first file in spiffs");
			Serial.println();
		} else {
			Serial.println("unknown command");
		}
	}
}

void init() {
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	int slot = rboot_get_current_rom();
	if (slot == 0) {
		spiffs_mount_manual(0x40300000, 0x70000);
	} else {
		spiffs_mount_manual(0x40500000, 0x70000);
	}
	
	// disable access point
	WifiAccessPoint.enable(false);
	
	Serial.printf("\r\nCurrently running rom %d.\r\n", slot);
	Serial.println("Type 'help' and press enter for instructions.");
	Serial.println();
	
	Serial.setCallback(serialCallBack);
	
}
