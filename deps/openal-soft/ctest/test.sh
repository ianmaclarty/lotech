#!/bin/sh
#/Users/ian/Library/Android/sdk/ndk-bundle/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang -isystem /Users/ian/Library/Android/sdk/ndk-bundle/sources/cxx-stl/gnu-libstdc++/4.9/include -isystem /Users/ian/Library/Android/sdk/ndk-bundle/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include --sysroot=/Users/ian/Library/Android/sdk/ndk-bundle/platforms/android-16/arch-arm -fPIC -DANDROID -Wno-psabi -fsigned-char -mthumb -march=armv7-a -mfloat-abi=softfp -o CMakeFiles/cmTC_55e1c.dir/testCCompiler.c.o -c ./test.c

#/Users/ian/Library/Android/sdk/ndk-bundle/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang++ -DAM_ANDROID -DAM_RELEASE -DAM_LUA52 -DLUA_COMPAT_ALL -DLUA_USE_POSIX -Wall -Werror -pthread -fno-strict-aliasing -Ibuilds/android/lua52/release/include -Ithird_party/glm-0.9.7.1 -ffast-math --sysroot /Users/ian/Library/Android/sdk/ndk-bundle/platforms/android-16/arch-arm -gcc-toolchain /Users/ian/Library/Android/sdk/ndk-bundle/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64 -fpic -target armv7-none-linux-androideabi -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -fno-exceptions -fno-rtti -I/Users/ian/Library/Android/sdk/ndk-bundle/sources/android/native_app_glue -I/Users/ian/Library/Android/sdk/ndk-bundle/sources/cxx-stl/gnu-libstdc++/4.9/include -I/Users/ian/Library/Android/sdk/ndk-bundle/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include -I/Users/ian/Library/Android/sdk/ndk-bundle/sources/cxx-stl/gnu-libstdc++/4.9/include/backward -I/Users/ian/Library/Android/sdk/ndk-bundle/platforms/android-16/arch-armusr/include -DANDROID -O3 -DNDEBUG   -c src/am_action.cpp -obuilds/android/lua52/release/obj/am_action.o

/Users/ian/Library/Android/sdk/ndk-bundle/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang -Wall -Werror -pthread -fno-strict-aliasing -Ibuilds/android/lua52/release/include -Ithird_party/glm-0.9.7.1 -ffast-math --sysroot /Users/ian/Library/Android/sdk/ndk-bundle/platforms/android-16/arch-arm -gcc-toolchain /Users/ian/Library/Android/sdk/ndk-bundle/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64 -fpic -target armv7-none-linux-androideabi -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -fno-exceptions -fno-rtti -I/Users/ian/Library/Android/sdk/ndk-bundle/sources/android/native_app_glue -I/Users/ian/Library/Android/sdk/ndk-bundle/sources/cxx-stl/gnu-libstdc++/4.9/include -I/Users/ian/Library/Android/sdk/ndk-bundle/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include -I/Users/ian/Library/Android/sdk/ndk-bundle/sources/cxx-stl/gnu-libstdc++/4.9/include/backward -I/Users/ian/Library/Android/sdk/ndk-bundle/platforms/android-16/arch-armusr/include -DANDROID -O3 -DNDEBUG   -c test.c -o test.o