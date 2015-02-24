@echo off

title Build program...
@set curdir=%cd%
@set pythondir="C:\Python27"
@set prgname="gen_appbin_old"

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
        echo Installing py2exe...
        pip.exe install py2exe
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
:end
pause
