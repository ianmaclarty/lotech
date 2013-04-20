include ../Make.common

all: $(OUT_DIR)/liblt.a

CPP_FILES:=$(wildcard *.cpp)
CPP_O_FILES:=$(patsubst %.cpp,$(OUT_DIR)/%.o,$(CPP_FILES))
MM_FILES:=$(wildcard *.mm)
MM_O_FILES:=$(patsubst %.mm,$(OUT_DIR)/%.o,$(MM_FILES))

ifndef LTCPP
LTCPP=g++
endif

O_FILES=$(CPP_O_FILES)
INCLUDE_FLAGS=-I. -I../$(TARGET_PLATFORM)/include
GPP_FLAGS= $(INCLUDE_FLAGS) $(LTCFLAGS) -DLTDEPTHBUF $(TARGET_FLAGS) -Wall -pthread
GPP=$(CROSS)$(LTCPP)
AR=$(CROSS)ar rcus

ifeq ($(TARGET_PLATFORM),ios)
GPP_FLAGS+=-DLTIOS -DLTGLES1 -ObjC++
O_FILES+=$(MM_O_FILES)
endif
ifeq ($(TARGET_PLATFORM),osx)
GPP_FLAGS+=-DLTOSX -ObjC++
O_FILES+=$(MM_O_FILES)
endif
ifeq ($(TARGET_PLATFORM),android)
GPP_FLAGS+=-DLTANDROID -DLTGLES1
endif
ifeq ($(TARGET_PLATFORM),linux)
GPP_FLAGS+=-DLTLINUX
endif
ifeq ($(TARGET_PLATFORM),mingw)
GPP_FLAGS+=-DLTMINGW
endif

$(CPP_O_FILES): $(OUT_DIR)/%.o: %.cpp
	@echo CPP   `basename $@`
	@$(GPP) $(GPP_FLAGS) -c $< -o $@

$(MM_O_FILES): $(OUT_DIR)/%.o: %.mm
	@echo MM   `basename $@`
	@$(GPP) -ObjC++ $(GPP_FLAGS) -c $< -o $@

$(OUT_DIR)/liblt.a: $(O_FILES)
	@echo AR $@
	@$(AR) $@ $^

$(OUT_DIR)/ltlua.o: lua_scripts.h

lua_scripts.h: lua/*.lua
	./gen_lua_scripts_h.sh

.PHONY: clean
clean:
	rm -f $(O_FILES) $(OUT_DIR)/*.a
