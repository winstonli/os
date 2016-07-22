#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
JOBS=$((`nproc`-1)) # use number of processors minus one

if [ "$JOBS" -eq 0 ]
then
  JOBS=1
fi

mkdir -p "$DIR/opt/cross"
mkdir -p "$DIR/opt/src"
cd "$DIR/opt/src"

sudo apt install libgmp3-dev libmpfr-dev libisl-dev libcloog-isl-dev libmpc-dev texinfo -y
wget -nc ftp://ftp.gnu.org/gnu/binutils/binutils-2.26.tar.gz
wget -nc ftp://ftp.gnu.org/gnu/gcc/gcc-6.1.0/gcc-6.1.0.tar.gz

tar -xvzf binutils-2.26.tar.gz
tar -xvzf gcc-6.1.0.tar.gz

export PREFIX="$DIR/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

mkdir build-binutils
cd build-binutils
../binutils-2.26/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j $JOBS
make install

cd ..
mkdir build-gcc
cd build-gcc
../gcc-6.1.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j $JOBS
make all-target-libgcc -j $JOBS
make install-gcc
make install-target-libgcc
