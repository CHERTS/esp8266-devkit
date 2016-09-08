#!/bin/bash
# Author: Mikhail Grigorev
# Last edit: 03/09/2016
#
# You will need the following mingw32/64 or equivalent linux packages to build it:
# msys gcc msys-coreutils msys-wget msys-autoconf msys-automake msys-mktemp
#
# Use mingw-get to install these.
# run this script from msys's or any unix console.

JOBS=-j1

TARGET=xtensa-lx106-elf

XTTC=$PWD/$TARGET
XTBP=$PWD/build
XTDLP=$PWD/dl
XTPTH=$PWD/patches

MINGW_PATH=c:/mingw
PATH=$XTTC/bin:$PATH

GMP="gmp-6.0.0a"
MPFR="mpfr-3.1.2"
MPC="mpc-1.0.2"
BINUTILS="binutils-2.25.1"
BINUTILS_VER="2.25.1"
NEWLIB="newlib-2.0.0"
NEWLIB_VER="2.0.0"
GCC="gcc-5.2.0"
GCC_VER="5.2.0"

DOWNLOAD=1
EXTRACT=1
BASELIBS=0
REPATCH=0
RECONF=0
REBUILD=0
REINSTALL=0

while true ; do
    case "$1" in
        --nodownloads) DOWNLOAD=0 ; echo "Not downloading anything" ; shift ;;
        --noextract) EXTRACT=0 ; echo "Never extract anything" ; shift ;;
        --nopatch) REPATCH=0 ; echo "Never patching anything" ; shift ;;
        --noreconf) RECONF=0 ; echo "Not reconfiguring anything" ; shift ;;
        --norebuild) REBUILD=0 ; echo "Not rebuilding anything" ; shift ;;
        --noreinstall) REINSTALL=0 ; echo "Not reinstalling anything" ; shift ;;
        --nobaselibs) BASELIBS=0 ; echo "Not building/installing support libs" ; shift ;;
        *) shift ; break ;;
    esac
done

# check if mingw is mounted, mount if needed
df /mingw
if [ $? -gt 0 ]; then
  mount $MINGW_PATH /mingw
  if [ $? -gt 0 ]; then
    echo "Failed to mount mingw using"
    echo $MINGW_PATH
    exit 1
  fi
  PATH=/mingw/bin:$PATH
fi

#find $XTDLP/*/build -type d | xargs rm -rf
mkdir -p $XTTC $XTDLP $XTBP

if [ $DOWNLOAD -gt 0 ]; then
  if [ ! -f $XTDLP/$GMP.tar.bz2 ]; then
    echo "==> Downloading GMP..."
    wget -c http://ftp.gnu.org/gnu/gmp/$GMP.tar.bz2 -P $XTDLP
  fi
  if [ ! -f $XTDLP/$MPFR.tar.bz2 ]; then
    echo "==> Downloading MPFR..."
    wget -c http://ftp.gnu.org/gnu/mpfr/$MPFR.tar.bz2 -P $XTDLP
  fi
  if [ ! -f $XTDLP/$MPC.tar.gz ]; then
    echo "==> Downloading MPC..."
    wget -c http://ftp.gnu.org/gnu/mpc/$MPC.tar.gz -P $XTDLP
  fi
  if [ ! -f $XTDLP/$BINUTILS.tar.bz2 ]; then
    echo "==> Downloading Binutils..."
    wget -c http://ftp.gnu.org/gnu/binutils/$BINUTILS.tar.bz2 -P $XTDLP
  fi
  if [ ! -f $XTDLP/$NEWLIB.tar.gz ]; then
    echo "==> Downloading Newlib..."
    wget -c ftp://sourceware.org/pub/newlib/$NEWLIB.tar.gz -P $XTDLP
  fi
  if [ ! -f $XTDLP/$GCC.tar.bz2 ]; then
    echo "==> Downloading GCC..."
    wget -c http://ftp.gnu.org/gnu/gcc/$GCC/$GCC.tar.bz2 -P $XTDLP
  fi
fi

if [ $EXTRACT -gt 0 ]; then
  if [ ! -d $XTDLP/$GMP ]; then
    echo "==> Extracting GMP..."
    tar xf $XTDLP/$GMP.tar.bz2 -C $XTDLP/
    # Fixes in case archive name != folder name
    find $XTDLP -maxdepth 1 -type d -name gmp-* | xargs -i mv -v {} $XTDLP/$GMP
  fi
  if [ ! -d $XTDLP/$MPFR ]; then
    echo "==> Extracting MPFR..."
    tar xf $XTDLP/$MPFR.tar.bz2 -C $XTDLP/
    # Fixes in case archive name != folder name
    #find $XTDLP -maxdepth 1 -type d -name mpfr-* | xargs -i mv -v {} $XTDLP/$MPFR
  fi
  if [ ! -d $XTDLP/$MPC ]; then
    echo "==> Extracting MPC..."
    tar xf $XTDLP/$MPC.tar.gz -C $XTDLP/
    # Fixes in case archive name != folder name
    #find $XTDLP -maxdepth 1 -type d -name mpc-* | xargs -i mv -v {} $XTDLP/$MPC
  fi
  if [ ! -d $XTDLP/$BINUTILS ]; then
    echo "==> Extracting Binutils..."
    tar xf $XTDLP/$BINUTILS.tar.bz2 -C $XTDLP/
  fi
  if [ ! -d $XTDLP/$NEWLIB ]; then
    echo "==> Extracting Newlib..."
    tar xf $XTDLP/$NEWLIB.tar.gz -C $XTDLP/
  fi
  if [ ! -d $XTDLP/$GCC ]; then
    echo "==> Extracting GCC..."
    tar xf $XTDLP/$GCC.tar.bz2 -C $XTDLP/
  fi
fi

mkdir -p $XTDLP/$GMP/build $XTDLP/$MPC/build $XTDLP/$MPFR/build 
mkdir -p $XTDLP/$BINUTILS/build $XTDLP/$NEWLIB/build
mkdir -p $XTDLP/$GCC/{build-1,build-2} 

set -e

if [ -f $XTPTH/overlays/xtensa_lx106.tar ]; then
  if [ ! -d $XTDLP/overlays ]; then
    mkdir $XTDLP/overlays
  fi  
  if [ ! -f $XTDLP/overlays/.extract ]; then
    rm -f $XTDLP/overlays/.extract
    echo "==> Extracting overlays xtensa_lx106.tar..."
    tar xf $XTPTH/overlays/xtensa_lx106.tar -C $XTDLP/overlays
    touch $XTDLP/overlays/.extract
  fi
fi

cd $XTDLP/$GMP/build
if [ $BASELIBS -gt 0 -o ! -f .built ]; then
  echo "==> Buidling GMP..."
  if [ $RECONF -gt 0 -o ! -f .configured ]; then
    rm -f .configured
    ../configure --prefix=$XTBP/gmp --disable-shared --enable-static
    touch .configured
  fi
  if [ $REBUILD -gt 0 -o ! -f .built ]; then
    rm -f .built
    nice make $JOBS
    touch .built
  fi
  if [ $REINSTALL -gt 0 -o ! -f .installed ]; then
    rm -f .installed
    make install
    touch .installed
  fi
fi

cd $XTDLP/$MPFR/build
if [ $BASELIBS -gt 0 -o ! -f .built ]; then
  echo "==> Buidling MPFR..."
  if [ $RECONF -gt 0 -o ! -f .configured ]; then
    rm -rf .configured
    ../configure --prefix=$XTBP/mpfr --with-gmp=$XTBP/gmp --disable-shared --enable-static
    touch .configured
  fi
  if [ $REBUILD -gt 0 -o ! -f .built ]; then
    rm -f .built
    nice make $JOBS
    touch .built
  fi
  if [ $REINSTALL -gt 0 -o ! -f .installed ]; then
    rm -f .installed
    make install
    touch .installed
  fi
fi

cd $XTDLP/$MPC/build
if [ $BASELIBS -gt 0 -o ! -f .built ]; then
  echo "==> Buidling MPC..."
  if [ $RECONF -gt 0 -o ! -f .configured ]; then
    rm -f .configured
    ../configure --prefix=$XTBP/mpc --with-mpfr=$XTBP/mpfr --with-gmp=$XTBP/gmp --disable-shared --enable-static
    touch .configured
  fi
  if [ $REBUILD -gt 0 -o ! -f .built ]; then
    rm -f .built
    nice make $JOBS
    touch .built
  fi
  if [ $REINSTALL -gt 0 -o ! -f .installed ]; then
    rm -f .installed
    make install
    touch .installed
  fi
fi

cd $XTDLP/../
if [ -d "$XTDLP/../patches/binutils/$BINUTILS_VER" ]; then
  if [ $REPATCH -gt 0 -o ! -f $XTDLP/$BINUTILS/build/.patched ]; then
    echo "==> Patching Binutils..."
    patch_list=`ls -L1 $XTDLP/../patches/binutils/$BINUTILS_VER`
    cd $XTDLP/$BINUTILS
    rm -f build/.patched
    for i in ${patch_list[@]}; do
      patch -p1 < $XTDLP/../patches/binutils/$BINUTILS_VER/$i
    done
    touch build/.patched
  fi
fi
if [ -f $XTDLP/overlays/.extract ]; then
  echo "==> Overlays Binutils..."
  cp -R $XTDLP/overlays/binutils/* $XTDLP/$BINUTILS/
fi
echo "==> Buidling Binutils..."
cd $XTDLP/$BINUTILS/build
if [ $RECONF -gt 0 -o ! -f .configured ]; then
  rm -f .configured
  ../configure --prefix=$XTTC --target=$TARGET --enable-werror=no --disable-multilib --disable-nls --disable-shared --disable-threads --with-gcc --with-gnu-as --with-gnu-ld
  touch .configured
fi
if [ $REBUILD -gt 0 -o ! -f .built ]; then
  rm -f .built
  nice make $JOBS
  touch .built
fi
if [ $REINSTALL -gt 0 -o ! -f .installed ]; then
  rm -f .installed
  make install
  touch .installed
fi

cd $XTDLP/../
if [ -d "$XTDLP/../patches/gcc/$GCC_VER" ]; then
  if [ $REPATCH -gt 0 -o ! -f $XTDLP/$GCC/build-1/.patched ]; then
    echo "==> Patching GCC..."
    patch_list=`ls -L1 $XTDLP/../patches/gcc/$GCC_VER`
    cd $XTDLP/$GCC
    rm -f build-1/.patched
    for i in ${patch_list[@]}; do
      patch -p1 < $XTDLP/../patches/gcc/$GCC_VER/$i
    done
    touch build-1/.patched
  fi
fi
if [ -f $XTDLP/overlays/.extract ]; then
  echo "==> Overlays GCC..."
  cp -R $XTDLP/overlays/gcc/* $XTDLP/$GCC/
fi
echo "==> Building first stage GCC..."
cd $XTDLP/$GCC/build-1
if [ $RECONF -gt 0 -o ! -f .configured ]; then
  rm -f .configured
  ../configure CFLAGS_FOR_TARGET="-mlongcalls" --prefix=$XTTC --target=$TARGET --disable-multilib --enable-languages=c --with-newlib --disable-nls --disable-shared --disable-threads --with-gnu-as --with-gnu-ld --with-gmp=$XTBP/gmp --with-mpfr=$XTBP/mpfr --with-mpc=$XTBP/mpc  --disable-libssp --without-headers --disable-__cxa_atexit --enable-cxx-flags="-fno-exceptions -fno-rtti" --enable-target-optspace --without-libiconv
  touch .configured
fi
if [ $REBUILD -gt 0 -o ! -f .built ]; then
  rm -f .built
  nice make $JOBS all-gcc
  touch .built
fi
if [ $REINSTALL -gt 0 -o ! -f .installed ]; then
  rm -f .installed
  make install-gcc
  touch .installed
fi

cd $XTDLP/../
if [ -d "$XTDLP/../patches/newlib/$NEWLIB_VER" ]; then
  if [ $REPATCH -gt 0 -o ! -f $XTDLP/$NEWLIB/build/.patched ]; then
    echo "==> Patching Newlib..."
    patch_list=`ls -L1 $XTDLP/../patches/newlib/$NEWLIB_VER`
    cd $XTDLP/$NEWLIB
    rm -f build/.patched
    for i in ${patch_list[@]}; do
      patch -p1 < $XTDLP/../patches/newlib/$NEWLIB_VER/$i
    done
    touch build/.patched
  fi
fi
if [ -f $XTDLP/overlays/.extract ]; then
  echo "==> Overlays Newlib..."
  cp -R $XTDLP/overlays/newlib/* $XTDLP/$NEWLIB/
fi
echo "==> Buidling Newlib..."
cd $XTDLP/$NEWLIB/build
if [ $RECONF -gt 0 -o ! -f .configured ]; then
  rm -f .configured
  ../configure CFLAGS_FOR_TARGET="-DMALLOC_PROVIDED" --prefix=$XTTC --target=$TARGET --disable-multilib --with-gnu-as --with-gnu-ld --disable-nls --disable-newlib-io-c99-formats --disable-newlib-io-long-long --disable-newlib-io-float --disable-newlib-io-long-double --disable-newlib-supplied-syscalls --enable-target-optspace --enable-newlib-mb --enable-newlib-hw-fp
  touch .configured
fi
if [ $REBUILD -gt 0 -o ! -f .built ]; then
  rm -f .built
  nice make $JOBS
  touch .built
fi
if [ $REINSTALL -gt 0 -o ! -f .installed ]; then
  rm -rf .installed
  make install
  touch .installed
fi

echo "==> Building final GCC..."
cd $XTDLP/$GCC/build-2
if [ $RECONF -gt 0 -o ! -f .configured ]; then
  rm -f .configured
  ../configure CFLAGS_FOR_TARGET="-mlongcalls" --prefix=$XTTC --target=$TARGET --disable-multilib --disable-nls --disable-shared --disable-threads --with-gnu-as --with-gnu-ld --with-gmp=$XTBP/gmp --with-mpfr=$XTBP/mpfr --with-mpc=$XTBP/mpc --enable-languages=c,c++ --with-newlib --disable-libssp --disable-__cxa_atexit --enable-cxx-flags="-fno-exceptions -fno-rtti" --enable-target-optspace --without-libiconv
  touch .configured
fi
if [ $REBUILD -gt 0 -o ! -f .built ]; then
  rm -f .built
  nice make $JOBS
  touch .built
fi
if [ $REINSTALL -gt 0 -o ! -f .installed ]; then
  rm -rf .installed
  make install
  touch .installed
fi

if [ -d "$XTTC/$TARGET/bin/" ]; then
  if [ -f "$XTTC/bin/$TARGET-g++.exe" ]; then
    cp "$XTTC/bin/$TARGET-g++.exe" "$XTTC/$TARGET/bin/g++.exe"
  fi
  if [ -f "$XTTC/bin/$TARGET-gcc.exe" ]; then
    cp "$XTTC/bin/$TARGET-gcc.exe" "$XTTC/$TARGET/bin/gcc.exe"
  fi
  if [ -f "$XTTC/bin/$TARGET-c++.exe" ]; then
    cp "$XTTC/bin/$TARGET-c++.exe" "$XTTC/$TARGET/bin/c++.exe"
  fi
fi

if [ -d "$MINGW_PATH/bin/" ]; then
  if [ -d "$XTTC/bin/" ]; then
    if [ -f "$MINGW_PATH/bin/libgcc_s_dw2-1.dll" ]; then
      cp "$MINGW_PATH/bin/libgcc_s_dw2-1.dll" "$XTTC/bin/"
    fi
    if [ -f "$MINGW_PATH/bin/zlib1.dll" ]; then
      cp "$MINGW_PATH/bin/zlib1.dll" "$XTTC/bin/"
    fi
  fi
  if [ -d "$XTTC/$TARGET/bin/" ]; then
    if [ -f "$MINGW_PATH/bin/libgcc_s_dw2-1.dll" ]; then
      cp "$MINGW_PATH/bin/libgcc_s_dw2-1.dll" "$XTTC/$TARGET/bin/"
    fi
    if [ -f "$MINGW_PATH/bin/zlib1.dll" ]; then
      cp "$MINGW_PATH/bin/zlib1.dll" "$XTTC/$TARGET/bin/"
    fi
  fi
fi

echo "==> Cloning/pulling lx106-hal repos..."
# Makeinfo will fail if it encounters CRLF endings.
git config --global core.autocrlf false
if cd $XTDLP/lx106-hal; then git pull; else git clone https://github.com/tommie/lx106-hal.git $XTDLP/lx106-hal; fi
if [ ! -d $XTDLP/lx106-hal/build ]; then
  mkdir $XTDLP/lx106-hal/build
fi
cd $XTDLP/../
if [ -d "$XTDLP/../patches/lx106-hal" ]; then
  if [ $REPATCH -gt 0 -o ! -f $XTDLP/lx106-hal/build/.patched ]; then
    echo "==> Patching lx106-hal..."
    patch_list=`ls -L1 $XTDLP/../patches/lx106-hal`
    cd $XTDLP/lx106-hal
    rm -f build/.patched
    for i in ${patch_list[@]}; do
      patch -p1 < $XTDLP/../patches/lx106-hal/$i
    done
    touch build/.patched
  fi
fi
echo "==> Build lx106-hal..."
cd $XTDLP/lx106-hal
if [ $RECONF -gt 0 -o ! -f build/.configured ]; then
  rm -f build/.configured
  autoreconf -i
  cd build
  ../configure --host=$TARGET
  touch .configured
fi
cd $XTDLP/lx106-hal/build
if [ $REBUILD -gt 0 -o ! -f .built ]; then
  rm -f .built
  nice make $JOBS
  touch .built
fi
cd $XTDLP/lx106-hal/build
if [ $REINSTALL -gt 0 -o ! -f .installed ]; then
  rm -rf .installed
  cp src/libhal.a "$XTTC/$TARGET/lib/"
  touch .installed
fi

cd $XTDLP
if [ -f $XTPTH/overlays/xtensa_include.zip ]; then
  if [ ! -d $XTTC/$TARGET/include ]; then
    mkdir -p $XTTC/$TARGET/include
  fi  
  if [ ! -f $XTDLP/overlays/.extract_xtensa_include ]; then
    rm -f $XTDLP/overlays/.extract_xtensa_include
    echo "==> Extracting overlays xtensa_include.zip..."
    unzip -o $XTPTH/overlays/xtensa_include.zip -d $XTTC/$TARGET/include
    touch $XTDLP/overlays/.extract_xtensa_include
  fi
fi

echo "==> Done!"
echo "==> Compiler is located at $XTTC"
