spiffy
======

Build a [spiffs](https://github.com/pellepl/spiffs) file system binary for embedding/writing
onto the [nodemcu](https://github.com/nodemcu/nodemcu-firmware) ESP8266 spiffs file system

### What is it

spiffy builds a binary spiffs image for you to write_flash to a esp8266 runing nodemcu so you can
get all the files onto your cool IoT device in one fell swoop.

### usage

#### Clone the repo and build spiffy

```bash
git clone https://github.com/xlfe/spiffy.git
cd spiffy
mkdir build
make
```

#### create a folder with the files you'd like to embed

```bash
mkdir files
```

and add your lua scripts, etc to ./files/

#### run spiffy to build the rom

```bash
$ ./build/spiffy
Creating rom spiff_rom.bin of size 16384 bytes
Adding files in directory files
init.lua added to spiffs (66 bytes)
$ ll *.bin
-rw-rw-r-- 1 build build 16384 Jan  1 21:00 spiff_rom.bin
```

NB: The default rom size is 16k - you can edit main.c to change this

#### burn your rom to the esp device

[Thanks for the info](https://github.com/nodemcu/nodemcu-firmware/issues/61#issuecomment-68423956)
The offset for spiffs file system:
* eagle.app.v6.flash.bin: 0x00000~len( eagle.app.v6.flash.bin )
* eagle.app.v6.irom0text.bin: 0x10000~0x10000 + len( eagle.app.v6.irom0text.bin )
* spiffs_embed.bin: (0x10000 + lengthof(iromtext) + 0x4000) & (0xFFFFC000)
* Ie: next to the irom0text.bin, but aligned to 4 * 4096 Bytes (0x4000).

For example:

```bash
uild@build:/opt/Espressif/nodemcu-firmware/bin$ ll
drwxrwxr-x  3 build build   4096 Dec 31 16:38 .
drwxrwxr-x 17 build build   4096 Jan  1 20:23 ..
-rw-rw-r--  1 build build  52064 Dec 31 16:25 eagle.app.v6.flash.bin
-rwxrwxr-x  1 build build 293568 Dec 31 16:25 eagle.app.v6.irom0text.bin
```

So to burn my image I would run:

```bash
esptool.py --port /dev/ttyUSB0 write_flash STARTADDR ../../spiffy/spiff_rom.bin
```
* Ie: STARTADDR = (0x10000 + lengthof(iromtext) + 0x4000) & (0xFFFFC000)

#### Done!




