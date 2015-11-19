@echo off

title ESP8266 - Build firmware

@set devkit_dir=C:\Espressif
@set mingw_dir=C:\MinGW

PATH=%devkit_dir%\xtensa-lx106-elf\bin;%mingw_dir%\bin;%mingw_dir%\msys\1.0\bin;%PATH%

cls

if not exist %mingw_dir%\bin\mingw32-make.exe (
	echo ERROR! MinGW not found.
	echo 1. Download MinGW at http://sourceforge.net/projects/mingw/files/Installer/
	echo 2. Install MinGW to only on disk C
	echo Press Enter to exit.
	pause >nul
	goto :eof
)

if not exist %devkit_dir%\xtensa-lx106-elf\bin\xtensa-lx106-elf-gcc.exe (
	echo ERROR! Unofficial Development Kit for Espressif ESP8266 not found.
	echo 1. Download DevKit at http://programs74.ru/udkew.html
	echo 2. Install DevKit to only on disk C
	echo Press Enter to exit.
	pause >nul
	goto :eof
)

echo Please follow below steps (1-7) to generate specific bin(s) and burn firmware:
echo STEP 1: Choose boot version (0=boot_v1.1, 1=boot_v1.2+, 2=none)
set input=default
set /p input=Enter (0/1/2, default 2):

if %input% equ 0 (
    set boot=old
) else (
if %input% equ 1 (
    set boot=new
) else (
    set boot=none
)
)

echo Boot mode: %boot%
echo.

echo STEP 2: Choose bin generate (0=eagle.flash.bin+eagle.irom0text.bin, 1=user1.bin, 2=user2.bin)
set input=default
set /p input=Enter (0/1/2, default 0):

if %input% equ 1 (
    if %boot% equ none (
        set app=0
        echo Choose no boot before
        echo Generate bin: eagle.flash.bin + eagle.irom0text.bin
    ) else (
        set app=1
        echo Generate bin: user1.bin
    )
) else (
if %input% equ 2 (
    if %boot% equ none (
        set app=0
        echo Choose no boot before
        echo Generate bin: eagle.flash.bin + eagle.irom0text.bin
    ) else (
        set app=2
        echo Generate bin: user2.bin
    )
) else (
    if %boot% neq none (
        set boot=none
        echo Ignore boot
    )
    set app=0
    echo Generate bin: eagle.flash.bin + eagle.irom0text.bin
))

echo.

echo STEP 3: Choose SPI speed (0=20MHz, 1=26.7MHz, 2=40MHz, 3=80MHz)
set input=default
set /p input=Enter (0/1/2/3, default 2):

if %input% equ 0 (
    set spi_speed=20
) else (
if %input% equ 1 (
    set spi_speed=26.7
) else (
if %input% equ 3 (
    set spi_speed=80
) else (
    set spi_speed=40
)))

echo SPI speed: %spi_speed% MHz
echo.

echo STEP 4: Choose SPI mode (0=QIO, 1=QOUT, 2=DIO, 3=DOUT)
set input=default
set /p input= Enter (0/1/2/3, default 0):

if %input% equ 1 (
    set spi_mode=QOUT
) else (
if %input% equ 2 (
    set spi_mode=DIO
) else (
if %input% equ 3 (
    set spi_mode=DOUT
8) else (
    set spi_mode=QIO
)))

echo SPI mode: %spi_mode%
echo.

echo STEP 5: Choose flash size and map
echo     0=512KB  (256KB+256KB)
echo     2=1024KB (512KB+512KB)
echo     3=2048KB (512KB+512KB)
echo     4=4096KB (512KB+512KB)
echo     5=2048KB (1024KB+1024KB)
echo     6=4096KB (1024KB+1024KB)
set input=default
set /p input=Enter (0/1/2/3/4/5/6, default 0):

if %input% equ 2 (
  set spi_size_map=2
  echo spi size: 1024KB
  echo spi ota map: 512KB + 512KB
) else (
  if %input% equ 3 (
    set spi_size_map=3
    echo spi size: 2048KB
    echo spi ota map: 512KB + 512KB
  ) else (
    if %input% equ 4 (
      set spi_size_map=4
      echo spi size: 4096KB
      echo spi ota map: 512KB + 512KB
    ) else (
      if %input% equ 5 (
        set spi_size_map=5
        echo spi size: 2048KB
        echo spi ota map: 1024KB + 1024KB
      ) else (
        if %input% equ 6 (
          set spi_size_map=6
          echo spi size: 4096KB
          echo spi ota map: 1024KB + 1024KB
        ) else (
          set spi_size_map=0
          echo spi size: 512KB
          echo spi ota map: 256KB + 256KB
        )
      )
    )
  )
)

echo.
echo STEP 6: Burn firmware to flash after build?
echo     0=No burn
echo     1=Yes, burn firmware
set input=default
set /p input=Enter (0/1, default 0):

if %input% equ 1 (
  set burn=yes
) else (
  set burn=no
)

echo Burn firmware: %burn%
echo.

echo STEP 7: Select COM-port
echo     1=COM1
echo     2=COM2
echo     3=COM3
echo     4=COM4
echo     5=COM5
set input=default
set /p input=Enter (1/2/3/4/5, default 3):

if %input% equ 1 (
   set port=1
) else (
  if %input% equ 2 (
    set port=2
  ) else (
    if %input% equ 3 (
      set port=3
    ) else (
      if %input% equ 4 (
        set port=4
      ) else (
        if %input% equ 5 (
          set port=5
        ) else (
          set port=3
        )
      )
    )
  )
)
echo COM-port selected: %port%

echo.
echo Start build...
echo.

make clean
make BOOT=%boot% APP=%app% SPI_SPEED=%spi_speed% SPI_MODE=%spi_mode% SPI_SIZE_MAP=%spi_size_map%

if %burn% equ yes (
  echo.
  echo Start burn firmware...
  echo.
  make ESPPORT=COM%port% flash
  rem %devkit_dir%\utils\esptool-ck.exe -cp COM%port% -cd ck -cb 256000 -ca 0x00000 -cf firmware/eagle.flash.bin
  rem %devkit_dir%\utils\esptool-ck.exe -cp COM%port% -cd ck -cb 256000 -ca 0x40000 -cf firmware/eagle.irom0text.bin 
)

echo All done, hit Enter to exit.
pause >nul
