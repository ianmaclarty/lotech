include ../Make.common

all: $(OUT_DIR)/liblt.a

CPP_FILES:=$(wildcard *.cpp)
CPP_O_FILES:=$(patsubst %.cpp,$(OUT_DIR)/%.o,$(CPP_FILES))
MM_FILES:=$(wildcard *.mm)
MM_O_FILES:=$(patsubst %.mm,$(OUT_DIR)/%.o,$(MM_FILES))

O_FILES=$(CPP_O_FILES)

ifeq ($(TARGET_PLATFORM),ios)
O_FILES+=$(MM_O_FILES)
endif
ifeq ($(TARGET_PLATFORM),iossim)
O_FILES+=$(MM_O_FILES)
endif
ifeq ($(TARGET_PLATFORM),osx)
O_FILES+=$(MM_O_FILES)
endif

ifndef LTCPP
LTCPP=g++
endif

INCLUDE_FLAGS=-I. -I../$(TARGET_PLATFORM)/include
GPP_FLAGS= $(INCLUDE_FLAGS) $(LTCFLAGS) -DLTDEPTHBUF $(TARGET_FLAGS) -Wall
GPP=$(CROSS)$(LTCPP)
AR=$(CROSS)ar rcus

$(CPP_O_FILES): $(OUT_DIR)/%.o: %.cpp lt*.h
	@echo CPP   `basename $@`
	@$(GPP) $(GPP_FLAGS) -c $< -o $@

$(MM_O_FILES): $(OUT_DIR)/%.o: %.mm *.h
	@echo MM   `basename $@`
	@$(GPP) -std=c++0x -ObjC++ $(GPP_FLAGS) -c $< -o $@

$(OUT_DIR)/liblt.a: $(O_FILES)
	@echo AR $@
	@$(AR) $@ $^

$(OUT_DIR)/ltlua.o: lua_scripts.h

lua_scripts.h: lua/*.lua
	./gen_lua_scripts_h.sh

.PHONY: clean
clean:
	rm -f $(O_FILES) $(OUT_DIR)/*.a
