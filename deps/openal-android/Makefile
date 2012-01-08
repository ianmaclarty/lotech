include ../../Make.common

all: libopenal.a

VPATH=.:OpenAL32:Alc:include
C_FILES:=$(wildcard OpenAL32/*.c) $(wildcard Alc/*.c)
O_FILES:=$(patsubst %.c,%.o,$(C_FILES))

GCC_FLAGS=-DAL_BUILD_LIBRARY -DAL_ALEXT_PROTOTYPES -I. -Iinclude -IOpenAL32/Include -O3 $(TARGET_FLAGS) -Wall
GCC=$(CROSS)gcc
AR=$(CROSS)ar rcus

$(O_FILES): %.o: %.c
	@echo CC $@
	@$(GCC) $(GCC_FLAGS) -c $< -o $@

libopenal.a: $(O_FILES)
	@echo AR $@
	@$(AR) $@ $^

.PHONY: clean
clean:
	rm -f $(O_FILES) *.a
