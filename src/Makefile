include ../Make.common

all: $(OUT_DIR)/liblt.a

CPP_FILES:=$(wildcard *.cpp)
CPP_O_FILES:=$(patsubst %.cpp,$(OUT_DIR)/%.o,$(CPP_FILES))
MM_FILES:=$(wildcard *.mm)
MM_O_FILES:=$(patsubst %.mm,$(OUT_DIR)/%.o,$(MM_FILES))

O_FILES=$(CPP_O_FILES)
INCLUDE_FLAGS=-I. -I../$(TARGET_PLATFORM)/include
GPP_FLAGS= $(INCLUDE_FLAGS) -O1 -DLTDEPTHBUF $(TARGET_FLAGS) -Wall -g
GPP=$(CROSS)g++
AR=$(CROSS)ar rcus

ifeq ($(TARGET_PLATFORM),ios)
GPP_FLAGS+=-DLTIOS -DLTGLES1 -DLTDEVMODE
O_FILES+=$(MM_O_FILES)
endif
ifeq ($(TARGET_PLATFORM),osx)
GPP_FLAGS+=-DLTOSX -DLTDEVMODE -DLTENVELOPE
O_FILES+=$(MM_O_FILES)
endif
ifeq ($(TARGET_PLATFORM),android)
GPP_FLAGS+=-DLTANDROID -DLTGLES1
endif
ifeq ($(TARGET_PLATFORM),linux)
GPP_FLAGS+=-DLTLINUX -DLTENVELOPE -DLTDEVMODE
endif
ifeq ($(TARGET_PLATFORM),mingw)
GPP_FLAGS+=-DLTMINGW -DLTENVELOPE
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

.PHONY: clean
clean:
	rm -f $(O_FILES) $(OUT_DIR)/*.a
