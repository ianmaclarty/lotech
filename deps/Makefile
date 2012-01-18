include ../Make.common

LUA_DIR=$(LUA)
LUAJIT_DIR=$(LUAJIT)
BOX2D_DIR=$(BOX2D)
ZLIB_DIR=$(ZLIB)
LIBPNG_DIR=$(LIBPNG)

TARGETS=liblua.a libbox2d.a libz.a libpng.a headers
ifeq ($(TARGET_PLATFORM),android)
TARGETS+=libopenal.a openal_headers
endif

all: $(TARGETS)

ifeq ($(TARGET_PLATFORM),ios)
############################ iOS Targets ##############################

liblua.a: rebuild
	cd $(LUAJIT_DIR) && $(MAKE) \
		HOST_CC="gcc -m32 -arch i386" \
		CROSS=$(ISDKP) \
		TARGET_FLAGS="-arch armv6 -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		TARGET=arm TARGET_SYS=iOS \
		clean all
	cp $(LUAJIT_DIR)/src/libluajit.a libluajit-armv6.a
	cd $(LUAJIT_DIR) && $(MAKE) \
		HOST_CC="gcc -m32 -arch i386" \
		CROSS=$(ISDKP) \
		TARGET_FLAGS="-arch armv7 -isysroot $(ISDK)/SDKs/$(ISDKVER)" \
		TARGET=arm TARGET_SYS=iOS \
		clean all
	cp $(LUAJIT_DIR)/src/libluajit.a libluajit-armv7.a
	lipo -create libluajit-armv*.a -output liblua.a
	rm libluajit-armv*.a

libbox2d.a: rebuild
	cd $(BOX2D_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		TARGET_FLAGS="-arch armv6 -isysroot $(ISDK)/SDKs/$(ISDKVER) -mno-thumb" \
		clean all
	cp $(BOX2D_DIR)/libbox2d.a libbox2d-armv6.a
	cd $(BOX2D_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		TARGET_FLAGS="-arch armv7 -isysroot $(ISDK)/SDKs/$(ISDKVER) -mno-thumb" \
		clean all
	cp $(BOX2D_DIR)/libbox2d.a libbox2d-armv7.a
	lipo -create libbox2d-armv*.a -output libbox2d.a
	rm libbox2d-armv*.a

libz.a: rebuild
	cd $(ZLIB_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		TARGET_FLAGS="-arch armv6 -isysroot $(ISDK)/SDKs/$(ISDKVER) -mno-thumb" \
		clean all
	cp $(ZLIB_DIR)/libz.a libz-armv6.a
	cd $(ZLIB_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		TARGET_FLAGS="-arch armv7 -isysroot $(ISDK)/SDKs/$(ISDKVER) -mno-thumb" \
		clean all
	cp $(ZLIB_DIR)/libz.a libz-armv7.a
	lipo -create libz-armv*.a -output libz.a
	rm libz-armv*.a

libpng.a: rebuild
	cd $(LIBPNG_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		TARGET_FLAGS="-arch armv6 -isysroot $(ISDK)/SDKs/$(ISDKVER) -mno-thumb" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a libpng-armv6.a
	cd $(LIBPNG_DIR) && $(MAKE) \
		CROSS=$(ISDKP)/ \
		TARGET_FLAGS="-arch armv7 -isysroot $(ISDK)/SDKs/$(ISDKVER) -mno-thumb" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a libpng-armv7.a
	lipo -create libpng-armv*.a -output libpng.a
	rm libpng-armv*.a
endif

ifeq ($(TARGET_PLATFORM),android)
############################ Android Targets ##############################

liblua.a: rebuild
	cd $(LUAJIT_DIR) && $(MAKE) \
		HOST_CC="gcc -m32 -arch i386" \
		CROSS=$(NDKP) \
		TARGET_FLAGS="--sysroot $(NDKSYSROOT)" \
		TARGET=arm TARGET_SYS=Linux \
		clean all
	cp $(LUAJIT_DIR)/src/libluajit.a $@

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
		clean all
	cp $(ZLIB_DIR)/libz.a .

libpng.a: rebuild
	cd $(LIBPNG_DIR) && $(MAKE) \
		CROSS=$(NDKP) \
		TARGET_FLAGS="--sysroot $(NDKSYSROOT) -marm" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a .

libopenal.a: rebuild.openal
	cd openal-android && $(MAKE) \
		CROSS=$(NDKP) \
		TARGET_FLAGS="--sysroot $(NDKSYSROOT) -marm" \
		clean all
	cp openal-android/libopenal.a .

.PHONY: openal_headers
openal_headers:
	cp -r openal-android/include/AL include/OpenAL

endif

ifeq ($(TARGET_PLATFORM),osx)
############################ Mac OS X Targets ##############################

liblua.a: rebuild
	cd $(LUAJIT_DIR) && $(MAKE) \
		CC="gcc -m64 -arch x86_64" \
		clean all
	cp $(LUAJIT_DIR)/src/libluajit.a $@

libbox2d.a: rebuild
	cd $(BOX2D_DIR) && $(MAKE) \
		TARGET_FLAGS="-m64 -arch x86_64" \
		clean all
	cp $(BOX2D_DIR)/libbox2d.a .

libz.a: rebuild
	cd $(ZLIB_DIR) && $(MAKE) \
		TARGET_FLAGS="-m64 -arch x86_64" \
		clean all
	cp $(ZLIB_DIR)/libz.a .

libpng.a: rebuild
	cd $(LIBPNG_DIR) && $(MAKE) \
		TARGET_FLAGS="-m64 -arch x86_64" \
		clean all
	cp $(LIBPNG_DIR)/libpng.a .
endif

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

.PHONY: clean
clean:
	cd $(LUAJIT_DIR) && $(MAKE) clean
	cd $(LUA_DIR) && $(MAKE) clean
	cd $(BOX2D_DIR) && $(MAKE) clean
	cd $(ZLIB_DIR) && $(MAKE) clean
	cd $(LIBPNG_DIR) && $(MAKE) clean
	rm -rf *.a
	rm -rf include/*
