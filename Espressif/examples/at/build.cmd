@echo off

title ESP8266 - Build firmware

@set devkit_dir=C:\Espressif
@set mingw_dir=C:\MinGW

PATH=%devkit_dir%\xtensa-lx106-elf\bin;%mingw_dir%\bin;%mingw_dir%\msys\1.0\bin;%PATH%

echo ESP8266 - Clean project
make clean
echo ESP8266 - Build project, step 1: user1.512.new.0.bin
make BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=0
if exist "firmware\upgrade\user1.512.new.0.bin" (
  copy /y firmware\upgrade\user1.512.new.0.bin firmware\
)

echo ESP8266 - Clean project
make clean
echo ESP8266 - Build project, step 2: user2.512.new.0.bin
make BOOT=new APP=2 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=0
if exist "firmware\upgrade\user2.512.new.0.bin" (
  copy /y firmware\upgrade\user2.512.new.0.bin firmware\
)
