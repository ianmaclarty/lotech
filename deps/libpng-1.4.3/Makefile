include ../../Make.common

all: libpng.a

C_FILES:=$(wildcard *.c)
O_FILES:=$(patsubst %.c,%.o,$(C_FILES))

GCC_FLAGS=-I. -I../$(ZLIB) -O3 $(TARGET_FLAGS) -Wall
GCC=$(CROSS)gcc
AR=$(CROSS)ar rcus

$(O_FILES): %.o: %.c
	@echo CC $@
	@$(GCC) $(GCC_FLAGS) -c $<

libpng.a: $(O_FILES)
	@echo AR $@
	@$(AR) $@ $^

.PHONY: clean
clean:
	rm -f *.o *.a
