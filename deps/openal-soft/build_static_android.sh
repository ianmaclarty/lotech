#!/bin/sh
PATH=$NDK_HOME/toolchains/llvm/prebuilt/$NDK_HOST/bin:$PATH
CC=$NDK_HOME/toolchains/llvm/prebuilt/$NDK_HOST/bin/clang
CPP=$NDK_HOME/toolchains/llvm/prebuilt/$NDK_HOST/bin/clang++
CXX=$CPP
mkdir -p build
rm -rf build/*
cd build
cmake -DLIBTYPE=STATIC -G "Unix Makefiles" -D CMAKE_TOOLCHAIN_FILE=../android_toolchain2.cmake ..
#cat config.h | sed 's/\#define AL_API .*$/\#define AL_API/' | sed 's/\#define ALC_API .*$/\#define ALC_API/' > config.h.tmp
#mv config.h.tmp config.h
#make
