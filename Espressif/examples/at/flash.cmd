@echo off

title ESP8266 - Burn firmware

@set devkit_dir=C:\Espressif
@set mingw_dir=C:\MinGW

PATH=%devkit_dir%\xtensa-lx106-elf\bin;%mingw_dir%\bin;%mingw_dir%\msys\1.0\bin;%PATH%

make flashinit ESPPORT=COM3
make flashboot ESPPORT=COM3
make BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=0 ESPPORT=COM3 flash
make BOOT=new APP=2 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=0 ESPPORT=COM3 flash
