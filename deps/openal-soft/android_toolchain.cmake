SET(CMAKE_SYSTEM_NAME Linux)  # Tell CMake we're cross-compiling
#include(CMakeForceCompiler)
# Prefix detection only works with compiler id "GNU"
# CMake will look for prefixed g++, cpp, ld, etc. automatically
#CMAKE_FORCE_C_COMPILER(arm-linux-androideabi-gcc GNU)
set(CMAKE_C_COMPILER   "arm-linux-androideabi-gcc"     CACHE PATH "gcc" FORCE)
set(CMAKE_CXX_COMPILER "arm-linux-androideabi-g++"     CACHE PATH "g++" FORCE)
set(CMAKE_AR           "arm-linux-androideabi-ar"      CACHE PATH "archive" FORCE)
set(CMAKE_LINKER       "arm-linux-androideabi-ld"      CACHE PATH "linker" FORCE)
set(CMAKE_NM           "arm-linux-androideabi-nm"      CACHE PATH "nm" FORCE)
set(CMAKE_OBJCOPY      "arm-linux-androideabi-objcopy" CACHE PATH "objcopy" FORCE)
set(CMAKE_OBJDUMP      "arm-linux-androideabi-objdump" CACHE PATH "objdump" FORCE)
set(CMAKE_STRIP        "arm-linux-androideabi-strip"   CACHE PATH "strip" FORCE)
set(CMAKE_RANLIB       "arm-linux-androideabi-ranlib"  CACHE PATH "ranlib" FORCE)

SET(CMAKE_FIND_ROOT_PATH "/Users/ian/muheyo/android-ndk-r7/platforms/android-9/arch-arm")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
