#!/bin/bash
# Author: Fabien Poussin
# Last edit: 20/11/2014
#
# Modified: Mikhail Grigorev
# Last edit: 08/04/2015
#
# You will need the following mingw32/64 or equivalent linux packages to build it:
# msys gcc msys-coreutils msys-wget msys-autoconf msys-automake msys-mktemp
#
# Use mingw-get to install these.
# run this script from msys's or any unix console.

JOBS=-j1

TARGET=xtensa-lx106-elf

BUILDROOT=$PWD
XTTC=$PWD/$TARGET
XTBP=$PWD/build
XTDLP=$PWD/dl

MINGW_PATH=c:/mingw
PATH=$XTTC/bin:$PATH

GMP="gmp-6.0.0a"
MPFR="mpfr-3.1.2"
MPC="mpc-1.0.2"

DOWNLOAD=1
REPOCHECK=1
RECONF=1
REBUILD=1
REINSTALL=1
BASELIBS=1

while true ; do
    case "$1" in
        --nodownloads) DOWNLOAD=0 ; echo "Not downloading anything" ; shift ;;
        --norepocheck) REPOCHECK=0 ; echo "Never check for updates in the repository gcc-xtensa, esp-newlib and esp-binutils" ; shift ;;
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

  echo "Downloading..."

  echo "GMP"
  wget -c http://ftp.gnu.org/gnu/gmp/$GMP.tar.bz2  -P $XTDLP
  echo "MPFR"
  wget -c http://ftp.gnu.org/gnu/mpfr/$MPFR.tar.bz2  -P $XTDLP
  echo "MPC"
  wget -c http://ftp.gnu.org/gnu/mpc/$MPC.tar.gz  -P $XTDLP

  echo "Extracting..."

  tar xf $XTDLP/$GMP.tar.bz2 -C $XTDLP/
  tar xf $XTDLP/$MPFR.tar.bz2 -C $XTDLP/
  tar xf $XTDLP/$MPC.tar.gz -C $XTDLP/

  echo "Extract path fixes..."

  # Fixes in case archive name != folder name
  find $XTDLP -maxdepth 1 -type d -name gmp-* | xargs -i mv -v {} $XTDLP/$GMP
  find $XTDLP -maxdepth 1 -type d -name mpfr-* | xargs -i mv -v {} $XTDLP/$MPFR
  find $XTDLP -maxdepth 1 -type d -name mpc-* | xargs -i mv -v {} $XTDLP/$MPC

fi

if [ $REPOCHECK -gt 0 ]; then

  echo "Cloning/pulling repos..."
  
  # Makeinfo will fail if it encounters CRLF endings.
  git config --global core.autocrlf false

  echo "GCC v5.1"
  if cd $XTDLP/gcc-xtensa; then git pull; else git clone -b lx106-5.1 https://github.com/jcmvbkbc/gcc-xtensa.git $XTDLP/gcc-xtensa; fi
  echo "Newlib"
  if cd $XTDLP/esp-newlib; then git pull; else git clone -b xtensa https://github.com/jcmvbkbc/newlib-xtensa.git $XTDLP/esp-newlib; fi
  echo "Binutils"
  if cd $XTDLP/esp-binutils; then git pull; else git clone https://github.com/fpoussin/esp-binutils.git $XTDLP/esp-binutils; fi

fi

mkdir -p $XTDLP/$GMP/build $XTDLP/$MPC/build $XTDLP/$MPFR/build 
mkdir -p $XTDLP/gcc-xtensa/{build-1,build-2} 
mkdir -p $XTDLP/esp-newlib/build $XTDLP/esp-binutils/build

set -e

cd $XTDLP/$GMP/build
if [ $BASELIBS -gt 0 -o ! -f .built ]; then
  echo "Buidling GMP"
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
  echo "Buidling MPFR"
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
  echo "Buidling MPC"
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

echo "Buidling Binutils"
cd $XTDLP/esp-binutils/build
if [ $RECONF -gt 0 -o ! -f .configured ]; then
  rm -f .configured
  ../configure --prefix=$XTTC --target=$TARGET --enable-werror=no  --enable-multilib --disable-nls --disable-shared --disable-threads --with-gcc --with-gnu-as --with-gnu-ld
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

echo "Building first stage GCC v5.1"
cd $XTDLP/gcc-xtensa/build-1
if [ $RECONF -gt 0 -o ! -f .configured ]; then
  rm -f .configured
  ../configure --prefix=$XTTC --target=$TARGET --enable-multilib --enable-languages=c --with-newlib --disable-nls --disable-shared --disable-threads --with-gnu-as --with-gnu-ld --with-gmp=$XTBP/gmp --with-mpfr=$XTBP/mpfr --with-mpc=$XTBP/mpc  --disable-libssp --without-headers --disable-__cxa_atexit
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

echo "Buidling Newlib"
cd $XTDLP/esp-newlib/build
if [ $RECONF -gt 0 -o ! -f .configured ]; then
  rm -f .configured
  ../configure  --prefix=$XTTC --target=$TARGET --enable-multilib --with-gnu-as --with-gnu-ld --disable-nls --disable-newlib-io-c99-formats --disable-newlib-io-long-long --disable-newlib-io-float --disable-newlib-io-long-double --disable-newlib-supplied-syscalls --enable-target-optspace
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

echo "Building final GCC v5.1"
cd $XTDLP/gcc-xtensa/build-2
if [ $RECONF -gt 0 -o ! -f .configured ]; then
  rm -f .configured
  ../configure --prefix=$XTTC --target=$TARGET --enable-multilib --disable-nls --disable-shared --disable-threads --with-gnu-as --with-gnu-ld --with-gmp=$XTBP/gmp --with-mpfr=$XTBP/mpfr --with-mpc=$XTBP/mpc --enable-languages=c,c++ --with-newlib --disable-libssp --disable-__cxa_atexit
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
  cp $BUILDROOT/specs-gcc-5.1 xtensa-lx106-elf/lib/gcc/xtensa-lx106-elf/5.1.0/specs
fi

echo "Done!"
echo "Compiler is located at $XTTC"