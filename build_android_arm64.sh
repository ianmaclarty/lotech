#!/bin/sh
set -xe
export NDK_HOME=~/AppData/Local/Android/sdk/ndk/29.0.14206865
export NDK_HOST=windows-x86_64
export NDK_TARGET=aarch64-linux-android30
echo android > target
rm -rf buildtmp.android
# find deps -name "*.o" -exec rm {} \;
# find deps -name "*.a" -exec rm {} \;
make