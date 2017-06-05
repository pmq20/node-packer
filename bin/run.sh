#!/bin/sh -e

#Define our target device
#export TARGET_ARCH="-march=x64"
# export TARGET_TUNE="-mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp -mthumb-interwork -mno-thumb"

#Define the cross compilators on your system
export AR="x86_64-unknown-linux-gnu-ar"
export CC="x86_64-unknown-linux-gnu-gcc"
export CXX="x86_64-unknown-linux-gnu-g++"
export LINK="x86_64-unknown-linux-gnu-g++"
export CPP="x86_64-unknown-linux-gnu-gcc -E"
export LD="x86_64-unknown-linux-gnu-ld"
export AS="x86_64-unknown-linux-gnu-as"
export CCLD="ax86_64-unknown-linux-gnu-gcc ${TARGET_ARCH}"
export NM="x86_64-unknown-linux-gnu-nm"
export STRIP="x86_64-unknown-linux-gnu-strip"
export OBJCOPY="x86_64-unknown-linux-gnu-objcopy"
export RANLIB="x86_64-unknown-linux-gnu-ranlib"
export F77="x86_64-unknown-linux-gnu-g77 ${TARGET_ARCH}"
unset LIBC

#Define flags
#export CXXFLAGS="-march=armv7-a"
export LDFLAGS="-L${CSTOOLS_LIB} -Wl,-rpath-link,${CSTOOLS_LIB} -Wl,-O1 -Wl,--hash-style=gnu"
export CFLAGS="-isystem${CSTOOLS_INC} -fexpensive-optimizations -frename-registers -fomit-frame-pointer -O2 -ggdb3"
export CPPFLAGS="-isystem${CSTOOLS_INC}"
# export CCFLAGS="-march=armv7-a"

#Tools
export CSTOOLS=/Volumes/crosstools/x86_64-unknown-linux-gnu
export CSTOOLS_INC=${CSTOOLS}/include
export CSTOOLS_LIB=${CSTOOLS}/lib
#export ARM_TARGET_LIB=$CSTOOLS_LIB
# export GYP_DEFINES="armv7=1"

#Define other things, those are not 'must' to have defined but we added
export SHELL="/bin/bash"
export TERM="screen"
export LANG="en_US.UTF-8"
export MAKE="make"

#Export the path for your system
#export HOME="/home/gioyik" #Change this one with the name of your user directory
export PATH=${CSTOOLS}/bin:/usr/arm-linux-gnueabi/bin/:$PATH

./nodec --clean-tmpdir --dest-os=linux --dest-arch=x64 ../tests/index.js