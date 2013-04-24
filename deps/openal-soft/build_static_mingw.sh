#!/bin/sh
rm -rf build/*
export DXSDK_DIR=C:/Program\ Files\ \(x86\)/Microsoft\ DirectX\ 9.0\ SDK\ \(Summer\ 2004\)
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=$LOCALDESTDIR -DLIBTYPE=STATIC -G "MSYS Makefiles" ..
cat config.h | sed 's/\#define AL_API .*$/\#define AL_API/' | sed 's/\#define ALC_API .*$/\#define ALC_API/' > config.h.tmp
mv config.h.tmp config.h
make
