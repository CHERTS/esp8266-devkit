# ESPWebFramework
A C++ Web Framework for ESP8266

To compile requires:
- Windows x86/x64;
- <a href="http://programs74.ru/udkew-en.html">Unofficial Development Kit for Espressif ESP8266</a>;
- <a href="http://git-scm.com/download/win">Git for Windows</a>;
- <a href="http://sourceforge.net/projects/mingw/files/Installer/">MinGW</a>;
- <a href="http://www.oracle.com/technetwork/java/javase/downloads/index.html">Java Runtime x86/x64</a>;
- <a href="http://www.eclipse.org/downloads/packages/eclipse-ide-cc-developers/lunasr2">Eclipse Luna x86/x64</a>;

Installing additional modules for MinGW:

1. <a href="http://programs74.ru/get.php?file=EspressifESP8266DevKitAddon">Download</a> the my scripts to automate the installation of additional modules for MinGW.
2. Extract archive and run install-mingw-package.bat. He will installed the basic modules for MinGW, installation should proceed without error.

Build:

1. Create directory C:\Espressif\myproject\
2. Clone repo: git clone https://github.com/CHERTS/ESPWebFramework.git C:\Espressif\myproject\ESPWebFramework
3. Run Eclipse, select File -> Import -> General -> Existing Project into Workspace, in the line Select root directory, select the directory C:\Espressif\myproject\ and import ESPWebFramework project.
4. Open project ESPWebFramework, select the Make Target project and run the target all, flashinit, flash and flashweb the assembly, while in the console window should display the progress of the build and flash.
