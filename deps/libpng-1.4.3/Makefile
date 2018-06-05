include ../../Make.common

all: libpng.a

C_FILES:=$(wildcard *.c)
O_FILES:=$(patsubst %.c,%.o,$(C_FILES))

CC_FLAGS=-I. -I../$(ZLIB) $(LTCFLAGS) $(TARGET_FLAGS)

$(O_FILES): %.o: %.c
	@echo CC $@
	@$(CC) $(CC_FLAGS) -c $<

libpng.a: $(O_FILES)
	@echo AR $@
	@$(AR) rcus $@ $^

.PHONY: clean
clean:
	rm -f *.o *.a
