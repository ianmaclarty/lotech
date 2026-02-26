#!/bin/sh
set -xe
export NDK_HOME=~/AppData/Local/Android/sdk/ndk/29.0.14206865
export NDK_HOST=windows-x86_64
echo TARGET=android_arm64.release > settings
make
cp builds/android_arm64/lua51/release/bin/liblotech.so ../bosonx/android3/bosonx/app/src/main/jniLibs/arm64-v8a/
