all: libz.a

C_FILES:=$(wildcard *.c)
O_FILES:=$(patsubst %.c,%.o,$(C_FILES))

CC_FLAGS=-I. $(LTCFLAGS) $(TARGET_FLAGS)

$(O_FILES): %.o: %.c
	@echo CC $@
	@$(CC) $(CC_FLAGS) -c $<

libz.a: $(O_FILES)
	@echo AR $@
	@$(AR) rcus $@ $^

.PHONY: clean
clean:
	rm -f *.o *.a
