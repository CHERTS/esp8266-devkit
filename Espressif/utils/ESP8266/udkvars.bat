@echo Setting environment for using Unofficial Development Kit for Espressif ESP8266 tools.

@set mingw_dir=C:\MinGW
@set devkit_dir=C:\Espressif
@set PATH=%devkit_dir%\xtensa-lx106-elf\bin;%mingw_dir%\bin;%mingw_dir%\msys\1.0\bin;%PATH%
@cd %devkit_dir%\examples\ESP8266\
