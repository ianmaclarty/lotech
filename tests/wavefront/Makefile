include ../../Make.common

LTDIR=../..

ifeq ($(TARGET_PLATFORM),osx)
GPPOPTS=-ObjC++ -g -DLTOSX -I$(LTDIR)/osx/include -L$(LTDIR)/osx -llt -lpng -lz -llua -lbox2d -lGLEW -lglfw -framework OpenGL -framework OpenAL -framework Cocoa
else
GPPOPTS=-O3 -DLTLINUX -I$(LTDIR)/linux/include -L$(LTDIR)/linux -llt -lpng -lz -llua -lbox2d -lGLEW -lglfw -lopenal -lGL
endif

all: run

.PHONY: test
test:
	@g++ -DLTDEVMODE test.cpp $(GPPOPTS) -o $@

.PHONY: clean
clean:
	-@rm -f test
	-@rm *.out
	-@rm *.res

.PHONY: run1
run1: test
	@for inp in `ls *.obj`; do \
	    test=`basename $$inp .obj`; \
	    ./test $$inp; \
	done

.PHONY: run
run: test
	@for inp in `ls *.obj`; do \
	    test=`basename $$inp .obj`; \
	    ./test $$inp > $$test.out 2>&1 ; \
	    diff -u $$test.exp $$test.out > $$test.res ; \
	    if [ "!" -e $$test.out -o -s $$test.res ]; then \
		echo $$test "FAIL ****"; \
	    else \
		echo $$test pass; \
	    fi; \
	done
