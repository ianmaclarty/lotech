include Make.common

TARGET_DIR=$(TARGET_PLATFORM)
PWD=$(shell pwd)
LTCFLAGS=-O3 -DNDEBUG
-include Make.params
LTCFLAGS+=$(LT_PLATFLAGS)


OBJC_FLAGS=
ifeq ($(TARGET_PLATFORM),ios)
IPHONEOS_DEPLOYMENT_TARGET=4.3
export IPHONEOS_DEPLOYMENT_TARGET
default: libs
OBJC_FLAGS=-ObjC++
endif
ifeq ($(TARGET_PLATFORM),iossim)
IPHONEOS_DEPLOYMENT_TARGET=4.3
export IPHONEOS_DEPLOYMENT_TARGET
default: libs
OBJC_FLAGS=-ObjC++ -fobjc-legacy-dispatch -fobjc-abi-version=2
endif
ifeq ($(TARGET_PLATFORM),android)
default: libs
endif
ifeq ($(TARGET_PLATFORM),osx)
MACOSX_DEPLOYMENT_TARGET=10.5
export MACOSX_DEPLOYMENT_TARGET
default: ltclient
OBJC_FLAGS=-ObjC++
endif
ifeq ($(TARGET_PLATFORM),linux)
default: ltclient
endif
ifeq ($(TARGET_PLATFORM),mingw)
default: ltclient
endif
ifeq ($(TARGET_PLATFORM),tizen-x86)
default: libs
endif

############################ iOS Target ##############################
ifeq ($(TARGET_PLATFORM),ios)

.PHONY: $(TARGET_DIR)/liblt.a
$(TARGET_DIR)/liblt.a: headers | $(TARGET_DIR)
	mkdir -p buildtmp.ios.armv6
	mkdir -p buildtmp.ios.armv7
	cd src && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCPP=$(IOS_CPP) \
		TARGET_FLAGS="$(IOS_ARMv6_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER) -mno-thumb" \
		OUT_DIR=$(PWD)/buildtmp.ios.armv6 \
		LTCFLAGS="$(LTCFLAGS) $(OBJC_FLAGS)" \
		all
	cd src && $(MAKE) \
		CROSS=$(ISDKP) \
		LTCPP=$(IOS_CPP) \
		TARGET_FLAGS="$(IOS_ARMv7_OPTS) -isysroot $(ISDK)/SDKs/$(ISDKVER) -mno-thumb" \
		OUT_DIR=$(PWD)/buildtmp.ios.armv7 \
		LTCFLAGS="$(LTCFLAGS) $(OBJC_FLAGS)" \
		all
	lipo -create buildtmp.ios.armv6/liblt.a buildtmp.ios.armv7/liblt.a -output $@
endif

############################ iPhone Simulator Target ##############################
ifeq ($(TARGET_PLATFORM),iossim)

.PHONY: $(TARGET_DIR)/liblt.a
$(TARGET_DIR)/liblt.a: headers | $(TARGET_DIR)
	mkdir -p buildtmp.iossim
	cd src && $(MAKE) \
		CROSS=$(ISDKP)/ \
		LTCPP=$(IOS_CPP) \
		TARGET_FLAGS="-arch i386 -isysroot $(ISDK)/SDKs/$(ISDKVER) -mno-thumb" \
		OUT_DIR=$(PWD)/buildtmp.iossim \
		LTCFLAGS="$(LTCFLAGS) $(OBJC_FLAGS)" \
		all
	cp buildtmp.iossim/liblt.a $(TARGET_DIR)/
endif

############################ Android Target ##############################
ifeq ($(TARGET_PLATFORM),android)
LTCFLAGS+=-mno-thumb

.PHONY: $(TARGET_DIR)/liblt.a
$(TARGET_DIR)/liblt.a: headers | $(TARGET_DIR)
	mkdir -p buildtmp.android
	cd src && $(MAKE) \
		CROSS=$(NDKP) \
		TARGET_FLAGS="$(NDKSTL) --sysroot $(NDKSYSROOT)" \
		OUT_DIR=$(PWD)/buildtmp.android \
		LTCFLAGS="$(LTCFLAGS)" \
		all
	cp buildtmp.android/liblt.a $(TARGET_DIR)/
endif

############################ Max OS X Target ##############################
ifeq ($(TARGET_PLATFORM),osx)
.PHONY: $(TARGET_DIR)/liblt.a
$(TARGET_DIR)/liblt.a: headers | $(TARGET_DIR)
	mkdir -p buildtmp.osx
	cd src && $(MAKE) \
		TARGET_FLAGS="-m64 -arch x86_64" \
		OUT_DIR=$(PWD)/buildtmp.osx \
		LTCFLAGS="$(LTCFLAGS) $(OBJC_FLAGS)" \
		all
	cp buildtmp.osx/liblt.a $(TARGET_DIR)/
endif

############################ Linux Target ##############################
ifeq ($(TARGET_PLATFORM),linux)
.PHONY: $(TARGET_DIR)/liblt.a
$(TARGET_DIR)/liblt.a: headers | $(TARGET_DIR)
	mkdir -p buildtmp.linux
	cd src && $(MAKE) \
		OUT_DIR=$(PWD)/buildtmp.linux \
		LTCFLAGS="$(LTCFLAGS)" \
		all
	cp buildtmp.linux/liblt.a $(TARGET_DIR)/
endif

############################ MinGW Target ##############################
ifeq ($(TARGET_PLATFORM),mingw)
.PHONY: $(TARGET_DIR)/liblt.a
$(TARGET_DIR)/liblt.a: headers | $(TARGET_DIR)
	mkdir -p buildtmp.mingw
	cd src && $(MAKE) \
		OUT_DIR=$(PWD)/buildtmp.mingw \
		LTCFLAGS="$(LTCFLAGS)" \
		all
	cp buildtmp.mingw/liblt.a $(TARGET_DIR)/
endif

############################ Tizen x86 target ##############################
ifeq ($(TARGET_PLATFORM),tizen-x86)
LTCFLAGS+=-I$(TIZSYSROOTX86)/usr/include/osp

.PHONY: $(TARGET_DIR)/liblt.a
$(TARGET_DIR)/liblt.a: headers | $(TARGET_DIR)
	mkdir -p buildtmp.tizen-x86
	cd src && $(MAKE) \
		CROSS=$(TDKPX86) \
		TARGET_FLAGS="$(TIZEN_TARGET_FLAGS_X86)" \
		OUT_DIR=$(PWD)/buildtmp.tizen-x86 \
		LTCFLAGS="$(LTCFLAGS)" \
		all
	cp buildtmp.tizen-x86/liblt.a $(TARGET_DIR)/
endif

##########################################################
##########################################################

.PHONY: libs
libs: $(TARGET_DIR)/liblt.a deplibs headers

ltclient: libs
	cd clients/glfw/ && make LTCFLAGS="$(LTCFLAGS) $(OBJC_FLAGS)" && cp $@ ../../


##########################################################
##########################################################

.PHONY: deplibs
deplibs: | $(TARGET_DIR)
	cd deps && $(MAKE) LTCFLAGS="$(LTCFLAGS)"
	-cp deps/*.a deps/*.lib $(TARGET_DIR)
	cp -r deps/include $(TARGET_DIR)

.PHONY: headers
headers: deplibs | $(TARGET_DIR)
	cp src/*.h $(TARGET_DIR)/include/

$(TARGET_DIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf buildtmp.$(TARGET_PLATFORM)*
	cd deps && $(MAKE) clean
	cd clients/glfw && $(MAKE) clean
	rm -rf $(TARGET_DIR)/*
	rm -f src/lua_scripts.h
	rm -f ltclient$(EXE_EXT)

.PHONY: tags
tags:
	ctags `find src -name "*.h"` `find src -name "*.cpp"` `find src -name "*.mm"` `find deps -name "*.c"` `find deps -name "*.cpp"` `find deps -name "*.h"`
