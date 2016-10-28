@echo off
title Installing additional packages for MinGW
cls
echo.
echo Installing additional packages for MinGW
echo.
net session >nul 2>&1
if not %ERRORLEVEL% EQU 0 (
      echo Error: To install packages need admin rights.
      echo.
      echo Run the script as an administrator.
      echo.
      echo To run as administrator, click on the file install.cmd the right mouse
      echo button and select 'Run as administrator'
      echo.
      echo Press Enter to exit.
      pause >nul
      goto :eof
)
echo Press Ctrl-C to cancel, Enter to continue
pause >nul
cd /d %~dp0
echo.

if exist "C:\MinGW" (
  if exist "C:\MinGW\var\cache\mingw-get" (
    if not exist "mingw-cache-packages.zip" (
      utils\ESP8266\wget.exe https://dl.programs74.ru/get.php?file=mingw-cache-packages -O mingw-cache-packages.zip --no-check-certificate
    )
    if exist "mingw-cache-packages.zip" (
      copy /Y mingw-cache-packages.zip "C:\MinGW\var\cache\mingw-get"
      copy /Y utils\ESP8266\unzip.exe "C:\MinGW\var\cache\mingw-get"
      C:
      cd C:\MinGW\var\cache\mingw-get
      if exist "C:\MinGW\var\cache\mingw-get\mingw-cache-packages.zip" (
        unzip.exe -o mingw-cache-packages.zip
        del /Q /F mingw-cache-packages.zip
        del /Q /F unzip.exe
      )
    )
  )
  C:
  cd C:\MinGW\bin
  if exist mingw-get.exe (
    echo Installing mingw packages...
    mingw-get install mingw32-base
    mingw-get install mingw32-mgwport
    mingw-get install mingw32-pdcurses
    mingw-get install mingw32-make
    mingw-get install mingw32-autoconf
    mingw-get install mingw32-automake
    mingw-get install mingw32-gdb
    mingw-get install gcc
    mingw-get install gcc-c++
    mingw-get install libz
    mingw-get install bzip2
    mingw-get install gettext
    mingw-get install pthreads-w32
    mingw-get install msys-base
    mingw-get install msys-coreutils
    mingw-get install msys-coreutils-ext
    mingw-get install msys-gcc-bin
    mingw-get install msys-wget-bin
    mingw-get install msys-m4
    mingw-get install msys-bison-bin
    mingw-get install msys-flex-bin
    mingw-get install msys-gawk
    mingw-get install msys-sed
    mingw-get install msys-patch
    mingw-get install msys-autoconf
    mingw-get install msys-automake
    mingw-get install msys-mktemp
    mingw-get install msys-libtool
    mingw-get install msys-help2man
    mingw-get install msys-gettext
    mingw-get install msys-perl
    echo.
    echo Installing additional packages for MinGW complete.
    echo.
    echo Press Enter to exit the installation wizard.
    echo.
    pause >nul
  ) else (
    echo.
    echo ERROR! mingw-get.exe not found.
    echo.
    echo Press Enter to exit.
    echo.
    pause >nul
    goto :eof
  )
) else (
  echo.
  echo ERROR! MinGW not found.
  echo.
  echo Press Enter to exit.
  echo.
)
