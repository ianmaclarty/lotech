include ../Make.common

LUA_DIR=$(LUA)
LUAJIT_DIR=$(LUAJIT)
BOX2D_DIR=$(BOX2D)
ZLIB_DIR=$(ZLIB)
LIBPNG_DIR=$(LIBPNG)
VORBIS_DIR=$(VORBIS)
FREETYPE_DIR=$(FREETYPE)
CURL_DIR=$(CURL)
GLEW_DIR=$(GLEW)
GLFW_DIR=$(GLFW)

TARGETS=liblua.a libbox2d.a libz.a libpng.a libft2.a libvorbis.a libcurl.a headers
ifeq ($(TARGET_PLATFORM),android)
TARGETS+=libopenal.a openal_headers
endif
ifeq ($(TARGET_PLATFORM),mingw)
TARGETS+=glfw_headers libglfw.a libOpenAL32.a openal_headers libglew32.a glew_headers
endif
ifeq ($(TARGET_PLATFORM),osx)
TARGETS+=glfw_headers libglfw.a libGLEW.a glew_headers
endif
ifeq ($(TARGET_PLATFORM),linux)
TARGETS+=glfw_headers libglfw.a libGLEW.a glew_headers openal_headers libopenal.a
endif

all: $(TARGETS)

ifeq ($(TARGET_PLATFORM),ios)
############################ iOS Targets ##############################

#liblua.a: rebuild
#	cd $(LUAJIT_DIR) && $(MAKE) \
#		HOST_CC="gcc -m32 -arch i386" \
#		CROSS=$(ISDKP) \
#		TARGET_FLAGS="$(IOS_ARMv6_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
#		TARGET=arm TARGET_SYS=iOS \
#		clean all
#	cp $(LUAJIT_DIR)/src/libluajit.a libluajit-armv6.a
#	cd $(LUAJIT_DIR) && $(MAKE) \
#		HOST_CC="gcc -m32 -arch i386" \
#		CROSS=$(ISDKP) \
#		TARGET_FLAGS="$(IOS_ARMv7_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
#		TARGET=arm TARGET_SYS=iOS \
#		clean all
#	cp $(LUAJIT_DIR)/src/libluajit.a libluajit-armv7.a
#	lipo -create libluajit-armv*.a -output liblua.a
#	rm libluajit-armv*.a

liblua.a: rebuild
	cd $(LUA_DIR) && $(MAKE) -f Makefile.custom \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="$(IOS_ARMv6_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LUA_DIR)/liblua.a liblua-armv6.a
	cd $(LUA_DIR) && $(MAKE) -f Makefile.custom \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="$(IOS_ARMv7_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LUA_DIR)/liblua.a liblua-armv7.a
	lipo -create liblua-armv*.a -output liblua.a
	rm liblua-armv*.a

libbox2d.a: rebuild
	cd $(BOX2D_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCPP=$(IOS_CPP) \
		TARGET_FLAGS="$(IOS_ARMv6_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(BOX2D_DIR)/libbox2d.a libbox2d-armv6.a
	cd $(BOX2D_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCPP=$(IOS_CPP) \
		TARGET_FLAGS="$(IOS_ARMv7_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(BOX2D_DIR)/libbox2d.a libbox2d-armv7.a
	lipo -create libbox2d-armv*.a -output libbox2d.a
	rm libbox2d-armv*.a

libz.a: rebuild
	cd $(ZLIB_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="$(IOS_ARMv6_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(ZLIB_DIR)/libz.a libz-armv6.a
	cd $(ZLIB_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="$(IOS_ARMv7_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(ZLIB_DIR)/libz.a libz-armv7.a
	lipo -create libz-armv*.a -output libz.a
	rm libz-armv*.a

libpng.a: rebuild
	cd $(LIBPNG_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="$(IOS_ARMv6_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a libpng-armv6.a
	cd $(LIBPNG_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="$(IOS_ARMv7_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a libpng-armv7.a
	lipo -create libpng-armv*.a -output libpng.a
	rm libpng-armv*.a

libvorbis.a: rebuild
	cd $(VORBIS_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="$(IOS_ARMv6_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(VORBIS_DIR)/libvorbis.a libvorbis-armv6.a
	cd $(VORBIS_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="$(IOS_ARMv7_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(VORBIS_DIR)/libvorbis.a libvorbis-armv7.a
	lipo -create libvorbis-armv*.a -output libvorbis.a
	rm libvorbis-armv*.a

libft2.a: rebuild
	cd $(FREETYPE_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="$(IOS_ARMv6_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(FREETYPE_DIR)/libft2.a libft2-armv6.a
	cd $(FREETYPE_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="$(IOS_ARMv7_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(FREETYPE_DIR)/libft2.a libft2-armv7.a
	lipo -create libft2-armv*.a -output libft2.a
	rm libft2-armv*.a

libcurl.a: rebuild
	cd $(CURL_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="$(IOS_ARMv6_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(CURL_DIR)/libcurl.a libcurl-armv6.a
	cd $(CURL_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="$(IOS_ARMv7_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(CURL_DIR)/libcurl.a libcurl-armv7.a
	lipo -create libcurl-armv*.a -output libcurl.a
	rm libcurl-armv*.a

endif

ifeq ($(TARGET_PLATFORM),iossim)
############################ iPhone Simulator Targets ##############################

liblua.a: rebuild
	cd $(LUA_DIR) && $(MAKE) -f Makefile.custom \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="-arch i386 -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LUA_DIR)/liblua.a $@

libbox2d.a: rebuild
	cd $(BOX2D_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCPP=$(IOS_CPP) \
		TARGET_FLAGS="-arch i386 -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(BOX2D_DIR)/libbox2d.a $@

libz.a: rebuild
	cd $(ZLIB_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="-arch i386 -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(ZLIB_DIR)/libz.a $@

libpng.a: rebuild
	cd $(LIBPNG_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="-arch i386 -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a $@

libvorbis.a: rebuild
	cd $(VORBIS_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="-arch i386 -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(VORBIS_DIR)/libvorbis.a $@

libft2.a: rebuild
	cd $(FREETYPE_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCC=$(IOS_CC) \
		TARGET_FLAGS="-arch i386 -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(FREETYPE_DIR)/libft2.a $@

endif

ifeq ($(TARGET_PLATFORM),android)
############################ Android Targets ##############################

liblua.a: rebuild
#	cd $(LUAJIT_DIR) && $(MAKE) \
#		HOST_CC="gcc -m32 -arch i386" \
#		CROSS=$(NDKP) \
#		TARGET_FLAGS="--sysroot $(NDKSYSROOT)" \
#		TARGET=arm TARGET_SYS=Linux \
#		clean all
#	cp $(LUAJIT_DIR)/src/libluajit.a $@
	cd $(LUA_DIR) && $(MAKE) -f Makefile.custom \
		CROSS=$(NDKP) \
		TARGET_FLAGS="--sysroot $(NDKSYSROOT) -marm" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LUA_DIR)/liblua.a $@

libbox2d.a: rebuild
	cd $(BOX2D_DIR) && $(MAKE) \
		CROSS=$(NDKP) \
		TARGET_FLAGS="$(NDKSTL) --sysroot $(NDKSYSROOT) -marm" \
		clean all
	cp $(BOX2D_DIR)/libbox2d.a .

libz.a: rebuild
	cd $(ZLIB_DIR) && $(MAKE) \
		CROSS=$(NDKP) \
		TARGET_FLAGS="--sysroot $(NDKSYSROOT) -marm" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(ZLIB_DIR)/libz.a .

libpng.a: rebuild
	cd $(LIBPNG_DIR) && $(MAKE) \
		CROSS=$(NDKP) \
		TARGET_FLAGS="--sysroot $(NDKSYSROOT) -marm" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a .

libft2.a: rebuild
	cd $(FREETYPE_DIR) && $(MAKE) \
		CROSS=$(NDKP) \
		TARGET_FLAGS="--sysroot $(NDKSYSROOT) -marm" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(FREETYPE_DIR)/libft2.a .

libvorbis.a: rebuild
	cd $(VORBIS_DIR) && $(MAKE) \
		CROSS=$(NDKP) \
		TARGET_FLAGS="--sysroot $(NDKSYSROOT) -marm" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(VORBIS_DIR)/libvorbis.a .

libcurl.a: rebuild
	cd $(CURL_DIR) && $(MAKE) \
		CROSS=$(NDKP) \
		TARGET_FLAGS="--sysroot $(NDKSYSROOT) -marm" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(CURL_DIR)/libcurl.a libcurl.a

#libopenal.a: rebuild.openal
#	cd openal-android && $(MAKE) \
#		CROSS=$(NDKP) \
#		TARGET_FLAGS="--sysroot $(NDKSYSROOT) -marm" \
#		LTCFLAGS="$(LTCFLAGS)" \
#		clean all
#	cp openal-android/libopenal.a .

libopenal.a:
	cp openal-soft/android/libopenal.a .

.PHONY: openal_headers
openal_headers:
	cp -r openal-soft/include/* include/

endif

ifeq ($(TARGET_PLATFORM),osx)
############################ Mac OS X Targets ##############################

liblua.a: rebuild
	cd $(LUA_DIR) && $(MAKE) macosx CC="gcc -m64 -arch x86_64 $(LTCFLAGS)"
	cp $(LUA_DIR)/src/liblua.a $@

libbox2d.a: rebuild
	cd $(BOX2D_DIR) && $(MAKE) \
		TARGET_FLAGS="-m64 -arch x86_64" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(BOX2D_DIR)/libbox2d.a .

libz.a: rebuild
	cd $(ZLIB_DIR) && $(MAKE) \
		TARGET_FLAGS="-m64 -arch x86_64" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(ZLIB_DIR)/libz.a .

libpng.a: rebuild
	cd $(LIBPNG_DIR) && $(MAKE) \
		TARGET_FLAGS="-m64 -arch x86_64" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a .

libvorbis.a: rebuild
	cd $(VORBIS_DIR) && $(MAKE) \
		TARGET_FLAGS="-m64 -arch x86_64" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(VORBIS_DIR)/libvorbis.a .

libft2.a: rebuild
	cd $(FREETYPE_DIR) && $(MAKE) \
		TARGET_FLAGS="-m64 -arch x86_64" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(FREETYPE_DIR)/libft2.a .

libcurl.a: rebuild
	cd $(CURL_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(CURL_DIR)/libcurl.a .

libGLEW.a: rebuild
	cd $(GLEW_DIR) && $(MAKE) \
		CC="gcc -m64 -arch x86_64" \
		clean all
	cp $(GLEW_DIR)/lib/libGLEW.a .

libglfw.a: rebuild
	cd $(GLFW_DIR) && $(MAKE) \
		CC="gcc -m64 -arch x86_64" \
		cocoa
	cp $(GLFW_DIR)/lib/cocoa/libglfw.a .

.PHONY: glfw_headers
glfw_headers:
	cp -r $(GLFW_DIR)/include/* include/

.PHONY: glew_headers
glew_headers:
	cp -r $(GLEW_DIR)/include/* include/

endif

ifeq ($(TARGET_PLATFORM),linux)
############################ Linux Targets ##############################

liblua.a: rebuild
	#cd $(LUAJIT_DIR) && $(MAKE)
	#cp $(LUAJIT_DIR)/src/libluajit.a $@
	cd $(LUA_DIR) && $(MAKE) linux CC="gcc $(LTCFLAGS)"
	cp $(LUA_DIR)/src/liblua.a $@

libbox2d.a: rebuild
	cd $(BOX2D_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(BOX2D_DIR)/libbox2d.a .

libz.a: rebuild
	cd $(ZLIB_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(ZLIB_DIR)/libz.a .

libpng.a: rebuild
	cd $(LIBPNG_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a .

libvorbis.a: rebuild
	cd $(VORBIS_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(VORBIS_DIR)/libvorbis.a .

libft2.a: rebuild
	cd $(FREETYPE_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(FREETYPE_DIR)/libft2.a .

libcurl.a: rebuild
	cd $(CURL_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(CURL_DIR)/libcurl.a .

ifdef LINUX32
libopenal.a:
	cp openal-soft/linux32/libopenal.a .
else
libopenal.a:
	cp openal-soft/linux64/libopenal.a .
endif

.PHONY: openal_headers
openal_headers:
	cp -r openal-soft/include/* include/

libGLEW.a: rebuild
	cd $(GLEW_DIR) && $(MAKE) \
		clean all
	cp $(GLEW_DIR)/lib/libGLEW.a .

libglfw.a: rebuild
	cd $(GLFW_DIR) && $(MAKE)
	cp $(GLFW_DIR)/build/src/libglfw3.a $@

.PHONY: glfw_headers
glfw_headers:
	cp -r $(GLFW_DIR)/include/* include/

.PHONY: glew_headers
glew_headers:
	cp -r $(GLEW_DIR)/include/* include/

endif


ifeq ($(TARGET_PLATFORM),mingw)
############################ MinGW Targets ##############################

liblua.a: rebuild
	cd $(LUA_DIR) && $(MAKE) mingw
	cp $(LUA_DIR)/src/liblua.a $@
	#cd $(LUAJIT_DIR) && $(MAKE) BUILDMODE=static
	#cp $(LUAJIT_DIR)/src/libluajit.a $@

libbox2d.a: rebuild
	cd $(BOX2D_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(BOX2D_DIR)/libbox2d.a .

libz.a: rebuild
	cd $(ZLIB_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(ZLIB_DIR)/libz.a .

libpng.a: rebuild
	cd $(LIBPNG_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a .

libft2.a: rebuild
	cd $(FREETYPE_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(FREETYPE_DIR)/libft2.a .

libcurl.a: rebuild
	cd $(CURL_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(CURL_DIR)/libcurl.a .

libvorbis.a: rebuild
	cd $(VORBIS_DIR) && $(MAKE) \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(VORBIS_DIR)/libvorbis.a .

libglfw.a: rebuild
	#cp $(GLFW_DIR)/mingw-prebuilt/libglfw3.a $@
	cd $(GLFW_DIR) && $(MAKE) mingw
	cp $(GLFW_DIR)/build/src/libglfw3.a $@

libglew32.a: rebuild
	cd $(GLEW_DIR) && $(MAKE) \
		clean all
	cp $(GLEW_DIR)/lib/libglew32.a .

libOpenAL32.a:
	cp openal-soft/mingw/libOpenAL32.a .

.PHONY: openal_headers
openal_headers:
	cp -r openal-soft/include/* include/

.PHONY: glfw_headers
glfw_headers:
	cp -r $(GLFW_DIR)/include/* include/

.PHONY: glew_headers
glew_headers:
	cp -r $(GLEW_DIR)/include/* include/


endif


ifeq ($(TARGET_PLATFORM),tizen-x86)
############################ Tizen ##############################

liblua.a: rebuild
	cd $(LUA_DIR) && $(MAKE) -f Makefile.custom \
		CROSS=$(TDKPX86) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_X86)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LUA_DIR)/liblua.a $@

libbox2d.a: rebuild
	cd $(BOX2D_DIR) && $(MAKE) \
		CROSS=$(TDKPX86) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_X86)" \
		clean all
	cp $(BOX2D_DIR)/libbox2d.a .

libz.a: rebuild
	cd $(ZLIB_DIR) && $(MAKE) \
		CROSS=$(TDKPX86) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_X86)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(ZLIB_DIR)/libz.a .

libpng.a: rebuild
	cd $(LIBPNG_DIR) && $(MAKE) \
		CROSS=$(TDKPX86) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_X86)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a .

libft2.a: rebuild
	cd $(FREETYPE_DIR) && $(MAKE) \
		CROSS=$(TDKPX86) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_X86)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(FREETYPE_DIR)/libft2.a .

libvorbis.a: rebuild
	cd $(VORBIS_DIR) && $(MAKE) \
		CROSS=$(TDKPX86) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_X86)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(VORBIS_DIR)/libvorbis.a .

libcurl.a: rebuild
	cd $(CURL_DIR) && $(MAKE) \
		CROSS=$(TDKPX86) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_X86)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(CURL_DIR)/libcurl.a libcurl.a

endif

ifeq ($(TARGET_PLATFORM),tizen-arm)
############################ Tizen ##############################

liblua.a: rebuild
	cd $(LUA_DIR) && $(MAKE) -f Makefile.custom \
		CROSS=$(TDKPARM) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_ARM)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LUA_DIR)/liblua.a $@

libbox2d.a: rebuild
	cd $(BOX2D_DIR) && $(MAKE) \
		CROSS=$(TDKPARM) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_ARM)" \
		clean all
	cp $(BOX2D_DIR)/libbox2d.a .

libz.a: rebuild
	cd $(ZLIB_DIR) && $(MAKE) \
		CROSS=$(TDKPARM) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_ARM)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(ZLIB_DIR)/libz.a .

libpng.a: rebuild
	cd $(LIBPNG_DIR) && $(MAKE) \
		CROSS=$(TDKPARM) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_ARM)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a .

libft2.a: rebuild
	cd $(FREETYPE_DIR) && $(MAKE) \
		CROSS=$(TDKPARM) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_ARM)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(FREETYPE_DIR)/libft2.a .

libvorbis.a: rebuild
	cd $(VORBIS_DIR) && $(MAKE) \
		CROSS=$(TDKPARM) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_ARM)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(VORBIS_DIR)/libvorbis.a .

libcurl.a: rebuild
	cd $(CURL_DIR) && $(MAKE) \
		CROSS=$(TDKPARM) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_ARM)" \
		LTCFLAGS="$(LTCFLAGS)" \
		clean all
	cp $(CURL_DIR)/libcurl.a libcurl.a

endif

##########################################################
##########################################################

.PHONY: headers
headers: rebuild
	mkdir -p include
	rm -rf include/*
	cp $(LUA_DIR)/src/lua.h include/
	cp $(LUA_DIR)/src/lauxlib.h include/
	cp $(LUA_DIR)/src/lualib.h include/
	cp $(LUA_DIR)/src/luaconf.h include/
	for h in `find $(BOX2D_DIR)/Box2D -name "*.h"`; do \
	    d=`echo $$h | sed 's/^[^\/]*\///g'`; \
	    d=`dirname $$d`; \
	    mkdir -p include/$$d; \
	    cp $$h include/$$d; \
	done
	cp $(LIBPNG_DIR)/png.h include/
	cp $(LIBPNG_DIR)/pngconf.h include/
	cp $(ZLIB_DIR)/zlib.h include/
	cp $(ZLIB_DIR)/zconf.h include/
	cp $(VORBIS_DIR)/stb_vorbis.h include/
	cp -r $(FREETYPE_DIR)/include/freetype include/
	cp -r $(CURL_DIR)/include/curl include/

.PHONY: clean
clean:
	cd $(LUAJIT_DIR) && $(MAKE) clean
	cd $(LUA_DIR) && $(MAKE) clean
	cd $(BOX2D_DIR) && $(MAKE) clean
	cd $(ZLIB_DIR) && $(MAKE) clean
	cd $(LIBPNG_DIR) && $(MAKE) clean
	cd $(FREETYPE_DIR) && $(MAKE) clean
	cd $(VORBIS_DIR) && $(MAKE) clean
	rm -rf *.a
	rm -rf *.lib
	rm -rf include/*
