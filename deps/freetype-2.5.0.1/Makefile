all: libft2.a

C_FILES:=	\
	src/autofit.c  \
	src/ftbbox.c   \
	src/ftdebug.c  \
	src/ftglyph.c  \
	src/ftinit.c  \
	src/ftstroke.c  \
	src/gxvalid.c  \
	src/raster.c  \
	src/smooth.c  \
	src/ftbase.c  \
	src/ftbitmap.c  \
	src/ftgasp.c   \
	src/ftgxval.c  \
	src/ftsystem.c  \
	src/psnames.c  \
	src/sfnt.c    \
	src/truetype.c

O_FILES:=$(patsubst %.c,%.o,$(C_FILES))

CC_FLAGS=-Iinclude -I. $(LTCFLAGS) $(TARGET_FLAGS) -DFT2_BUILD_LIBRARY

$(O_FILES): %.o: %.c
	@echo CC $@
	@$(CC) $(CC_FLAGS) -c $< -o $@

libft2.a: $(O_FILES)
	@echo AR $@
	@$(AR) rcus $@ $^

.PHONY: clean
clean:
	rm -f src/*.o *.a
