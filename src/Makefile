include ../Make.common

all: $(OUT_DIR)/liblt.a

CPP_FILES:=$(wildcard *.cpp)
CPP_O_FILES:=$(patsubst %.cpp,$(OUT_DIR)/%.o,$(CPP_FILES))
MM_FILES:=$(wildcard *.mm)
MM_O_FILES:=$(patsubst %.mm,$(OUT_DIR)/%.o,$(MM_FILES))

O_FILES=$(CPP_O_FILES)
INCLUDE_FLAGS=-I. -I../deps/$(LUA)/src -I../deps/$(BOX2D) -I../deps/$(ZLIB) -I../deps/$(LIBPNG)
GPP_FLAGS= $(INCLUDE_FLAGS) -O3 -DLTDEPTHBUF -DLTDEVMODE $(TARGET_FLAGS) -Wall
GPP=$(CROSS)g++
AR=$(CROSS)ar rcus

ifeq ($(TARGET_PLATFORM),ios)
GPP_FLAGS+=-DLTIOS
O_FILES+=$(MM_O_FILES)
endif
ifeq ($(TARGET_PLATFORM),osx)
GPP_FLAGS+=-DLTOSX
O_FILES+=$(MM_O_FILES)
endif


$(CPP_O_FILES): $(OUT_DIR)/%.o: %.cpp
	@echo CPP   $@
	@$(GPP) $(GPP_FLAGS) -c $< -o $@

$(MM_O_FILES): $(OUT_DIR)/%.o: %.mm
	@echo CPP   $@
	@$(GPP) -ObjC++ $(GPP_FLAGS) -c $< -o $@

$(OUT_DIR)/liblt.a: $(O_FILES)
	@echo AR $@
	@$(AR) $@ $^

.PHONY: clean
clean:
	rm -f $(O_FILES) $(OUT_DIR)/*.a
