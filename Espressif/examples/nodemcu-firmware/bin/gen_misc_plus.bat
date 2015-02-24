@echo off

@set comport=COM2
@set curdir=%cd%
@set eagle_app_v6_out="eagle.app.v6.out"
@set eagle_app_v6_out_dir="..\app\.output\eagle\debug\image"
@set irom0text_addr="0x10000"
@set xtensa_tool_root="C:\Espressif\xtensa-lx106-elf\bin"
@set devkit_utils="C:\Espressif\utils"

if exist %eagle_app_v6_out_dir% (
  cd %eagle_app_v6_out_dir%
  if exist eagle.app.flash.bin (
    del /F /Q eagle.app.flash.bin
  )
  if exist eagle.app.v6.data.bin (
    del /F /Q eagle.app.v6.data.bin
  )
  if exist eagle.app.v6.flash.bin (
    del /F /Q eagle.app.v6.flash.bin
  )
  if exist eagle.app.v6.irom0text.bin (
    del /F /Q eagle.app.v6.irom0text.bin
  )
  if exist eagle.app.v6.rodata.bin (
    del /F /Q eagle.app.v6.rodata.bin
  )
  if exist eagle.app.v6.text.bin (
    del /F /Q eagle.app.v6.text.bin
  )
  if exist eagle.app.sym (
    del /F /Q eagle.app.sym
  )

  if exist %eagle_app_v6_out% (
    %xtensa_tool_root%\xtensa-lx106-elf-objcopy --only-section .text -O binary %eagle_app_v6_out% eagle.app.v6.text.bin
    %xtensa_tool_root%\xtensa-lx106-elf-objcopy --only-section .data -O binary %eagle_app_v6_out% eagle.app.v6.data.bin
    %xtensa_tool_root%\xtensa-lx106-elf-objcopy --only-section .rodata -O binary %eagle_app_v6_out% eagle.app.v6.rodata.bin
    %xtensa_tool_root%\xtensa-lx106-elf-objcopy --only-section .irom0.text -O binary %eagle_app_v6_out% eagle.app.v6.irom0text.bin
    if exist %devkit_utils%\gen_appbin_old.exe (
      %devkit_utils%\gen_appbin_old.exe %eagle_app_v6_out% v6
    ) else (
      echo ERROR! %devkit_utils%\gen_appbin_old.exe is not found
    )
    if exist %devkit_utils%\gen_flashbin.exe (
      %devkit_utils%\gen_flashbin.exe eagle.app.v6.flash.bin eagle.app.v6.irom0text.bin %irom0text_addr%
    ) else (
      echo ERROR! %devkit_utils%\gen_flashbin.exe is not found
    )
    if exist eagle.app.flash.bin (
      del /F /Q eagle.app.v6.data.bin
      del /F /Q eagle.app.v6.flash.bin
      del /F /Q eagle.app.v6.irom0text.bin
      del /F /Q eagle.app.v6.rodata.bin
      del /F /Q eagle.app.v6.text.bin
      del /F /Q eagle.app.sym
      move eagle.app.flash.bin %curdir%
      cd %curdir%
      if exist eagle.app.flash.bin (
        %devkit_utils%\esptool-ck.exe -cp %comport% -cd ck -cb 256000 -ca 0x00000 -cf eagle.app.flash.bin -v
        %devkit_utils%\esptool-ck.exe -cp %comport% -cd ck -cb 256000 -ca 0x7e000 -cf blank.bin -v
      )
    ) else (
      echo ERROR! File eagle.app.flash.bin is not found
    )
  ) else (
      echo ERROR! File %eagle_app_v6_out% is not found
  )
) else (
    echo ERROR! Directory %eagle_app_v6_out_dir% is not found
)
