del libmgcc.a
md libgcc
cd libgcc
@rem C:\Espressif\xtensa-lx106-elf\bin\xtensa-lx106-elf-ar x C:\Espressif\xtensa-lx106-elf\lib\gcc\xtensa-lx106-elf\4.8.2\libgcc.a
C:\Espressif\xtensa-lx106-elf\bin\xtensa-lx106-elf-ar x C:\Espressif\xtensa-lx106-elf\lib\gcc\xtensa-lx106-elf\5.1.0\libgcc.a
@rem delete:
@rem _fixunsdfsi.o _umoddi3.o _umodsi3.o _extendsfdf2.o _fixdfsi.o _divsi3.o _divdf3.o _divdi3.o _fixunssfsi.o
@rem _floatsidf.o _floatsisf.o _floatunsidf.o _floatunsisf.o _muldf3.o _muldi3.o _mulsf3.o _truncdfsf2.o
@rem _udivdi3.o _udivsi3.o _umulsidi3.o _addsubdf3.o _addsubsf3.o
for /f "delims=" %%f in (../rom_files_list.txt) do del "%%f.o"
C:\Espressif\xtensa-lx106-elf\bin\xtensa-lx106-elf-ar ru ..\libmgcc.a *.o
cd ..
rd /q /s libgcc