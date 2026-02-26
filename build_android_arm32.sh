#!/bin/sh
set -xe
export NDK_HOME=~/AppData/Local/Android/sdk/ndk/29.0.14206865
export NDK_HOST=windows-x86_64
echo TARGET=android_arm32.release > settings
make
cp builds/android_arm32/lua51/release/bin/liblotech.so ../bosonx/android3/bosonx/app/src/main/jniLibs/armeabi-v7a/
