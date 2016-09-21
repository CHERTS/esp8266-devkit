@echo off

title Build program...
@set curdir=%cd%
@set pythondir=C:\Python27
@set prgname=gen_appbin

setlocal enableextensions enabledelayedexpansion

if exist %pythondir% (
  if not exist "%pythondir%\Scripts\pip.exe" (
    copy /Y get-pip.py %pythondir%
  )
  copy /Y %prgname%.py %pythondir%
  copy /Y %prgname%-build.py %pythondir%
  C:
  cd %pythondir%
  if exist "python.exe" (
    if not exist "%pythondir%\Scripts\pip.exe" (
      echo Installing Pip...
      python get-pip.py
    )
    if exist "%pythondir%\Scripts\pip.exe" (
      cd Scripts\
      if not exist "%pythondir%\Lib\site-packages\serial\__init__.py" (
        echo Installing pyserial...
        pip install pyserial
      )
      if not exist "%pythondir%\Lib\site-packages\argparse.py" (
        echo Installing argparse...
        pip install argparse
      )
      if not exist "%pythondir%\Lib\site-packages\py2exe\__init__.py" (
        rem echo Check Windows architecture...
	rem echo %PROCESSOR_ARCHITECTURE%|FINDSTR AMD64>NUL && SET ARCH=AMD64 || SET ARCH=x86
	if exist %TEMP%\py_arch.txt (
	  del /Q /F %TEMP%\py_arch.txt
	)
	echo Check Python architecture...
	%pythondir%\python.exe -c "import platform; print platform.architecture()">%TEMP%\py_arch.txt
	call :check
	if exist %TEMP%\py_arch.txt (
	  del /Q /F %TEMP%\py_arch.txt
	)
	echo Found Python 2.7 !ARCH!
        echo Downloading py2exe...
	if "!ARCH!"=="x86" (
		"%curdir%\wget.exe" "http://kent.dl.sourceforge.net/project/py2exe/py2exe/0.6.9/py2exe-0.6.9.win32-py2.7.exe" -O "%curdir%\py2exe-0.6.9.win32-py2.7.exe"
	) else (
		"%curdir%\wget.exe" "http://kent.dl.sourceforge.net/project/py2exe/py2exe/0.6.9/py2exe-0.6.9.win64-py2.7.amd64.exe" -O "%curdir%\py2exe-0.6.9.win64-py2.7.amd64.exe"
	)
	if exist "%curdir%\py2exe-0.6.9.win32-py2.7.exe" (
	        echo Installing py2exe...
		"%curdir%\py2exe-0.6.9.win32-py2.7.exe"
		del /Q /F "%curdir%\py2exe-0.6.9.win32-py2.7.exe"
	)
	if exist "%curdir%\py2exe-0.6.9.win64-py2.7.amd64.exe" (
	        echo Installing py2exe...
		"%curdir%\py2exe-0.6.9.win64-py2.7.amd64.exe"
		del /Q /F "%curdir%\py2exe-0.6.9.win64-py2.7.amd64.exe"
	)
      )
      cd ..
    ) else (
      echo ERROR! %pythondir%\Scripts\pip.exe is not found
    )
    echo Build %prgname%.exe...
    python.exe %prgname%-build.py py2exe
    if exist "%pythondir%\dist\%prgname%.exe" (
      copy /Y dist\%prgname%.exe %curdir%
      copy /Y dist\python27.dll %curdir%
      copy /Y dist\_ctypes.pyd %curdir%
      copy /Y dist\library.zip %curdir%
      echo Done
    ) else (
      echo ERROR! %pythondir%\dist\%prgname%.exe is not found
    )
    if exist "%pythondir%\get-pip.py" (
      del /Q get-pip.py
    )
    del /Q %prgname%.py
    del /Q %prgname%-build.py
    rd /S /Q build\
    rd /S /Q dist\
  ) else (
    echo ERROR! python.exe is not found
    goto end
  )
) else (
  echo ERROR! Python 2.7 is not found
)
endlocal
goto end

:check
for /F "delims=" %%x in (%TEMP%\py_arch.txt) do set py_arch=%%x
echo %py_arch%|FINDSTR 64bit>NUL && SET ARCH=AMD64 || SET ARCH=x86
exit /b

:end
pause
