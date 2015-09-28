Introduction
------------
This is a sample project to show how to build roms for use with rBoot and how to
perform an OTA update. You can use rboot-ota.c & rboot-ota.h to add OTA support
to your own projects. There is also some commented out code in rboot_ota_start
that shows how to write non-rom files to arbitrary location on flash (e.g. for
data or embedded filesystems).

To compile
----------
1) If you haven't already compiled rBoot do that first.
2) You will also need a compiled copy of esptool2.
3) Symlink or copy rboot.h, rboot-api.h and rboot-api.c in to this directory.
4) Edit the Makefile to set the paths to the SDK and esptool2.
5) Set WIFI_SSID & WIFI_PWD as env vars or in the makefile.
6) Set OTA server details in rboot-ota.h
7) Flash, as below.
8) Connect a terminal and type 'help'.

All the above are available from GitHub: https://github.com/raburton/esp8266

Once built simply flash with something like this:
  esptool.py --port COM2 write_flash -fs 8m 0x00000 rboot.bin 0x02000 rom0.bin 0x82000 rom1.bin 0xfc000 blank4.bin

Tested with SDK v1.3 on an ESP12 (if using a board with less than 1mb of flash
a change to the second linker script will be required).
