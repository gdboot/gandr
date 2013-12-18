#!/bin/sh

# The download a file function.
download()
{
    local url=$1

    # Download the file.
    wget  --progress=dot -c -P $PREFIX $url 2>&1 | grep "%" | sed -u -e "s,\.,,g" -e "s,\,,,g" | awk '{printf("\b\b\b\b%4s", $2)}'
    echo " [DONE]"
}

test ! -n "$PREFIX" && PREFIX=$(readlink -f ./tools)

# The TARGET.
# Currently allowed values: i386-elf
test ! -n "$TARGET" && { echo "Error: target not set."; exit 1; }

# Make the build directory.
mkdir -p $PREFIX
WORKING_DIR=$(pwd)

# Get the tools.

# Binutils.
echo -n "  [WGET]  $PREFIX/binutils-2.23.2.tar.bz2,     "
download "http://ftp.gnu.org/gnu/binutils/binutils-2.23.2.tar.bz2"

# GCC.
echo -n "  [WGET]  $PREFIX/gcc-4.8.2.tar.bz2,     "
download "http://ftp.gnu.org/gnu/gcc/gcc-4.8.2/gcc-4.8.2.tar.bz2"

# Untar them.

# Binutils.
echo "  [UNTAR] $PREFIX/binutils-2.23.2.tar.bz2"
tar -xf $PREFIX/binutils-2.23.2.tar.bz2 -C $PREFIX >/dev/null

# GCC.
echo "  [UNTAR] $PREFIX/gcc-4.8.2.tar.bz2"
tar -xf $PREFIX/gcc-4.8.2.tar.bz2 -C $PREFIX >/dev/null

# Build the tools.

# Binutils.
mkdir -p $PREFIX/build-binutils

# Configure.
echo "  [BINUT] Configuring"
cd $PREFIX/build-binutils && ../binutils-2.23.2/configure --target=$TARGET --prefix=$PREFIX --disable-nls
cd $WORKING_DIR

# Compile.
echo "  [BINUT] Compiling"
make -C $PREFIX/build-binutils all

# Install.
echo "  [BINUT] Installing"
make -C $PREFIX/build-binutils install

# Clean.
echo "  [BINUT] Cleaning"
rm -rf $PREFIX/build-binutils $PREFIX/binutils-2.23.2

# GCC.
mkdir -p $PREFIX/build-gcc

# Configure.
echo "  [GCC]   Configuring"
export PATH=$PATH:$PREFIX/bin

cd $PREFIX/build-gcc && ../gcc-4.8.2/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c --without-headers --with-gnu-as --with-gnu-ld
cd $WORKING_DIR

export LD_FOR_TARGET=$PREFIX/bin/$TARGET-ld
export OBJDUMP_FOR_TARGET=$PREFIX/bin/$TARGET-objdump
export NM_FOR_TARGET=$PREFIX/bin/$TARGET-nm
export RANLIB_FOR_TARGET=$PREFIX/bin/$TARGET-ranlib
export READELF_FOR_TARGET=$PREFIX/bin/$TARGET-readelf
export STRIP_FOR_TARGET=$PREFIX/bin/$TARGET-strip
export AS_FOR_TARGET=$PREFIX/bin/$TARGET-as

# Compile.
echo "  [GCC]   Compiling"
make -C $PREFIX/build-gcc all-gcc

# Install.
echo "  [GCC]   Installing"
make -C $PREFIX/build-gcc install-gcc

# Compile libgcc.
echo "  [LIBGCC] Compiling"
make -C $PREFIX/build-gcc all-target-libgcc CFLAGS_FOR_TARGET="-g -O2"

# Install
echo "  [LIBGCC] Installing"
make -C $PREFIX/build-gcc install-target-libgcc

# Clean.
echo "  [GCC]   Cleaning"
rm -rf $PREFIX/build-gcc $PREFIX/gcc-4.8.2
