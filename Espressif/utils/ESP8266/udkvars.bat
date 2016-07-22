@echo Setting environment for using Unofficial Development Kit for Espressif ESP8266 tools.

@set mingw_dir=C:\MinGW
@set devkit_dir=C:\Espressif
@set python_dir=C:\Python27
@set PATH=%python_dir%;%devkit_dir%\xtensa-lx106-elf\bin;%devkit_dir%\utils\ESP8266;%mingw_dir%\bin;%mingw_dir%\msys\1.0\bin;%PATH%
@cd %devkit_dir%\examples\ESP8266\
