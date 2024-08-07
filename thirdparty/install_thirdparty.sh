#!/bin/bash

#builds grub on macos so we can easily use grub-mkresuce to make our isos
#ty https://gist.github.com/emkay/a1214c753e8c975d95b4

PWD=$(cd ..; pwd)
echo "found root dir \"$PWD\" "
echo SETUP THIRDPARTY TOOLS
echo not necessary on linux platforms


TC_PREFIX="$PWD/toolchain/cross/bin/smith"
TARGET=x86_64-elf
PREFIX="$PWD/thirdparty/build"

mkdir -p build


if [ ! -d "grub" ]; then
  echo "Downloading Grub Source"
  git clone --depth 1 git://git.savannah.gnu.org/grub.git
fi

echo "../configure --disable-werror TARGET_CC=$TC_PREFIX-gcc TARGET_OBJCOPY=$TC_PREFIX-objcopy TARGET_STRIP=$TC_PREFIX-strip TARGET_NM=$TC_PREFIX-nm TARGET_RANLIB=$TC_PREFIX-ranlib --target=$TARGET --prefix=$PREFIX"
exit

cd grub
export TARGET=$TARGET
export PREFIX=$PREFIX
./bootstrap
mkdir -p build-grub
cd build-grub
../configure --disable-werror TARGET_CC=$TC_PREFIX-gcc TARGET_OBJCOPY=$TC_PREFIX-objcopy \
TARGET_STRIP=$TC_PREFIX-strip TARGET_NM=$TC_PREFIX-nm TARGET_RANLIB=$TC_PREFIX-ranlib --target=$TARGET --prefix=$PREFIX
make
make install

echo Built Grub

