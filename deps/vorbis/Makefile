include ../../Make.common

all: libvorbis.a

C_FILES:=$(wildcard *.c)
O_FILES:=$(patsubst %.c,%.o,$(C_FILES))

ifndef LTCC
LTCC=gcc
endif

GCC_FLAGS=-I. $(LTCFLAGS) $(TARGET_FLAGS)
GCC=$(CROSS)$(LTCC)
ifndef LTAR
AR=$(CROSS)ar rcus
else
AR=$(LTAR)
endif

$(O_FILES): %.o: %.c
	@echo CC $@
	@$(GCC) $(GCC_FLAGS) -c $<

libvorbis.a: $(O_FILES)
	@echo AR $@
	@$(AR) $@ $^

.PHONY: clean
clean:
	rm -f *.o *.a
