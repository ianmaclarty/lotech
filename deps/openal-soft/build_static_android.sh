#!/bin/sh
PATH=/Users/ian/muheyo/android-ndk-r7/toolchains/arm-linux-androideabi-4.4.3/prebuilt/darwin-x86/bin:$PATH
mkdir -p build
rm -rf build/*
cd build
cmake -DLIBTYPE=STATIC -G "Unix Makefiles" -D CMAKE_TOOLCHAIN_FILE=../android_toolchain2.cmake ..
#cat config.h | sed 's/\#define AL_API .*$/\#define AL_API/' | sed 's/\#define ALC_API .*$/\#define ALC_API/' > config.h.tmp
#mv config.h.tmp config.h
#make
