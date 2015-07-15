@echo off

@set comport=COM3
@set curdir=%cd%
@set elf_out=app.out
@set out_dir=build
@set irom0text_addr="0x40000"
@set devkit_dir=C:\Espressif
@set mingw_dir=C:\MinGW

rem SPI_SPEED = 40, 26, 20, 80
@set SPI_SPEED="40"

rem SPI_MODE: qio, qout, dio, dout
@set SPI_MODE="qio"

rem SPI_SIZE: 512K, 256K, 1M, 2M, 4M
@set SPI_SIZE="512K"

PATH=%devkit_dir%\xtensa-lx106-elf\bin;%mingw_dir%\bin;%mingw_dir%\msys\1.0\bin;%PATH%

if exist %mingw_dir%\bin\mingw32-make.exe (
  echo Clean project directory...
  make -f Makefile clean
  echo Build project...
  make -f Makefile
) else (
    echo ERROR! MinGW not found
)

if exist %out_dir% (
  echo Build firmware...
  cd %out_dir%
  if exist %elf_out% (
    %devkit_dir%\utils\esptool-ck.exe -v -eo %elf_out% -bo eagle.flash.bin.%SPI_MODE%.%SPI_SPEED%.%SPI_SIZE% -bm %SPI_MODE% -bz %SPI_SIZE% -bf %SPI_SPEED% -bs .text -bs .data -bs .rodata -bc -ec -eo %elf_out% -es .irom0.text eagle.irom0text.bin.%SPI_MODE%.%SPI_SPEED%.%SPI_SIZE% -ec
    if exist %devkit_dir%\utils\gen_flashbin.exe (
      %devkit_dir%\utils\gen_flashbin.exe eagle.flash.bin.%SPI_MODE%.%SPI_SPEED%.%SPI_SIZE% eagle.irom0text.bin.%SPI_MODE%.%SPI_SPEED%.%SPI_SIZE% %irom0text_addr%
    ) else (
      echo ERROR! %devkit_dir%\utils\gen_flashbin.exe is not found
    )
    if exist eagle.app.flash.bin (
      move eagle.app.flash.bin %curdir%\flash.%SPI_MODE%.%SPI_SPEED%.%SPI_SIZE%.bin
      cd %curdir%
      if exist flash.%SPI_MODE%.%SPI_SPEED%.%SPI_SIZE%.bin (
        echo Burn firmware...
        %devkit_dir%\utils\esptool-ck.exe -cp %comport% -cd ck -cb 256000 -ca 0x00000 -cf flash.%SPI_MODE%.%SPI_SPEED%.%SPI_SIZE%.bin
        echo Clean project directory...
        make -f Makefile clean
      )
    ) else (
      echo ERROR! File eagle.app.flash.bin is not found
    )
  ) else (
      echo ERROR! File %elf_out% is not found
  )
) else (
    echo ERROR! %out_dir% directory not found
)
