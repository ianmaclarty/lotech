include ../../Make.common

LTDIR=../..

ifeq ($(TARGET_PLATFORM),osx)
GPPOPTS=-ObjC++ -g -DLTOSX -I$(LTDIR)/osx/include -L$(LTDIR)/osx -llt -lpng -lz -llua -lbox2d -lGLEW -lglfw -framework OpenGL -framework OpenAL -framework Cocoa
else
GPPOPTS=-O3 -DLTLINUX -I$(LTDIR)/linux/include -L$(LTDIR)/linux -llt -lpng -lz -llua -lbox2d -lGLEW -lglfw -lopenal -lGL
endif

all: run

.PHONY: ffitest
ffitest:
	@g++ -DLTDEVMODE ffitest.cpp $(GPPOPTS) -o $@

.PHONY: clean
clean:
	-@rm -f ffitest
	-@rm *.out
	-@rm *.res

.PHONY: run
run: ffitest
	@for inp in `ls *.lua`; do \
	    test=`basename $$inp .lua`; \
	    ./ffitest $$inp > $$test.out 2>&1 ; \
	    diff -u $$test.exp $$test.out > $$test.res ; \
	    if [ "!" -e $$test.out -o -s $$test.res ]; then \
		echo $$test "FAIL ****"; \
	    else \
		echo $$test pass; \
	    fi; \
	done
