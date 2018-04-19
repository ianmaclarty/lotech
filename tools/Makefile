include ../Make.common

LTDIR=..

ifeq ($(TARGET_PLATFORM),osx)
MACOSX_DEPLOYMENT_TARGET=10.5
export MACOSX_DEPLOYMENT_TARGET
GPPOPTS=-DLTOSX -ObjC++ -I../osx/include -L../osx -llt -lcurl -lpng -lvorbis -lz -llua -lbox2d -lGLEW -lglfw -framework OpenGL -framework OpenAL -framework Cocoa -framework IOKit -pagezero_size 10000 -image_base 100000000
CPP=g++
else
GPPOPTS=-O3 -DLTLINUX -I$(LTDIR)/linux/include -L$(LTDIR)/linux -llt -lvorbis -lcurl -lpng -lz -llua -lbox2d -lGLEW -lglfw -lopenal -lGL -pthread -ldl
CPP=g++
endif

PROGS=randtest devserver pngbb

all: $(PROGS)

$(PROGS): %: %.cpp
	$(CPP) -DLTDEVMODE $< $(GPPOPTS) -o $@ 

.PHONY: clean
clean:
	rm -f $(PROGS)
