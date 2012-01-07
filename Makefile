include Make.common

TARGET_DIR=lib-$(TARGET_PLATFORM)
PWD=$(shell pwd)

all: $(TARGET_DIR)/liblt.a deplibs headers

ifeq ($(TARGET_PLATFORM),ios)
############################ iOS Target ##############################
.PHONY: $(TARGET_DIR)/liblt.a
$(TARGET_DIR)/liblt.a: | $(TARGET_DIR)
	mkdir -p buildtmp.ios.armv6
	mkdir -p buildtmp.ios.armv7
	cd src && $(MAKE) \
		CROSS=$(ISDKP)/ \
		TARGET_FLAGS="-arch armv6 -isysroot $(ISDK)/SDKs/$(ISDKVER) -mno-thumb" \
		OUT_DIR=$(PWD)/buildtmp.ios.armv6 \
		all
	cd src && $(MAKE) \
		CROSS=$(ISDKP) \
		TARGET_FLAGS="-arch armv7 -isysroot $(ISDK)/SDKs/$(ISDKVER) -mno-thumb" \
		OUT_DIR=$(PWD)/buildtmp.ios.armv7 \
		all
	lipo -create buildtmp.ios.armv6/liblt.a buildtmp.ios.armv7/liblt.a -output $@
endif

ifeq ($(TARGET_PLATFORM),osx)
############################ Max OS X Target ##############################
.PHONY: $(TARGET_DIR)/liblt.a
$(TARGET_DIR)/liblt.a: | $(TARGET_DIR)
	mkdir -p buildtmp.osx
	cd src && $(MAKE) \
		TARGET_FLAGS="-m64 -arch x86_64 -DLTWINHEIGHT=640 -DLTWINWIDTH=960 -DLTENVELOPE" \
		OUT_DIR=$(PWD)/buildtmp.osx \
		all
	cp buildtmp.osx/liblt.a $(TARGET_DIR)/
endif

.PHONY: deplibs
deplibs: | $(TARGET_DIR)
	cd deps && $(MAKE)
	cp deps/*.a $(TARGET_DIR)
	cp -r deps/include $(TARGET_DIR)

.PHONY: headers
headers: | $(TARGET_DIR)
	cp src/*.h $(TARGET_DIR)/include

$(TARGET_DIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf buildtmp.$(TARGET_PLATFORM)*
	cd deps && $(MAKE) clean
	rm -rf $(TARGET_DIR)/*
