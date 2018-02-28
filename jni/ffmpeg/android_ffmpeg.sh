#!/bin/bash

while getopts ":s:n:p:u:i:h" arg 
do	case $arg in
	s)
		WORKING_DIR="$OPTARG"
		;;
	n)
		NDK="$OPTARG"
		;;
	p)	
		SDK="$OPTARG"
		;;
	u)
		CPU="$OPTARG"
		;;
	i)
		INSTALL_PATH="$OPTARG"
		;;
	esac
done
shift $((OPTIND-1))

if [ "WORKING_DIR" = "" ]; then
	echo "source dir not set, use current path = $PWD "
	WORKING_DIR=`pwd`
fi

if [ "$NDK" = "" ]; then
	echo "ndk path not set, use default path = /home/zlseu_edu/Android/android-ndk-r14b"
	NDK=/home/zlseu_edu/Android/android-ndk-r14b
fi

if [ "$SDK" = "" ]; then
	echo "android platform not set, use default android-24"
	SDK=android-24
fi

if [ "$CPU" = "" ]; then
	echo "cpu arch not set, use default armeabi"
	CPU=armeabi
fi

if [ "$INSTALL_PATH" = "" ]; then
	echo "INSTALL_PATH variable not set, use default path = $PWD/android"
	INSTALL_PATH=./android
fi

if [ "$CPU" != "arm64-v8a" ]; then
	TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64
	PLATFORM=$NDK/platforms/$SDK/arch-arm
	CCP=arm-linux-androideabi-
	CC=arm-linux-androideabi-gcc
	NM=arm-linux-androideabi-nm
	ARM_ARCH=arm
else
	TOOLCHAIN=$NDK/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64
	PLATFORM=$NDK/platforms/$SDK/arch-arm64
	CCP=aarch64-linux-android-
	CC=aarch64-linux-android-gcc
	NM=aarch64-linux-android-nm
	ARM_ARCH=arm64
fi

PREFIX=$INSTALL_PATH/$CPU

function build_arm
{
    ADDITIONAL_CONFIGURE_FLAG=

    ./configure \
        --prefix=$PREFIX \
        --target-os=linux \
        --cross-prefix=$TOOLCHAIN/bin/$CCP \
        --arch=$ARM_ARCH \
        --sysroot=$PLATFORM \
        --extra-cflags="-I$PLATFORM/usr/include" \
        --cc=$TOOLCHAIN/bin/$CC \
        --nm=$TOOLCHAIN/bin/$NM\
        --enable-static \
        --enable-runtime-cpudetect \
        --enable-gpl \
        --enable-small \
        --enable-cross-compile \
        --disable-debug \
        --disable-shared \
        --disable-doc \
        --disable-asm \
        --disable-ffmpeg \
        --disable-ffplay \
        --disable-ffprobe \
        --disable-ffserver \
        --disable-postproc \
        --disable-avdevice \
        --disable-symver \
        --disable-stripping \
    $ADDITIONAL_CONFIGURE_FLAG
    sed -i '' 's/HAVE_LRINT 0/HAVE_LRINT 1/g' config.h
    sed -i '' 's/HAVE_LRINTF 0/HAVE_LRINTF 1/g' config.h
    sed -i '' 's/HAVE_ROUND 0/HAVE_ROUND 1/g' config.h
    sed -i '' 's/HAVE_ROUNDF 0/HAVE_ROUNDF 1/g' config.h
    sed -i '' 's/HAVE_TRUNC 0/HAVE_TRUNC 1/g' config.h
    sed -i '' 's/HAVE_TRUNCF 0/HAVE_TRUNCF 1/g' config.h
    sed -i '' 's/HAVE_CBRT 0/HAVE_CBRT 1/g' config.h
    sed -i '' 's/HAVE_RINT 0/HAVE_RINT 1/g' config.h
    make clean
    make -j4
    make install
}

build_arm
