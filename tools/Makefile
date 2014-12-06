include ../Make.common

LTDIR=..

ifeq ($(TARGET_PLATFORM),osx)
GPPOPTS=-ObjC++ -g -DLTOSX -I$(LTDIR)/osx/include -L$(LTDIR)/osx -llt -lpng -lz -llua -lbox2d -lGLEW -lglfw -framework OpenGL -framework OpenAL -framework Cocoa -framework IOKit
else
GPPOPTS=-O3 -DLTLINUX -I$(LTDIR)/linux/include -L$(LTDIR)/linux -llt -lvorbis -lcurl -lpng -lz -llua -lbox2d -lGLEW -lglfw -lopenal -lGL -pthread -ldl
endif

PROGS=randtest devserver pngbb

all: $(PROGS)

$(PROGS): %: %.cpp
	g++ -DLTDEVMODE $< $(GPPOPTS) -o $@ 

.PHONY: clean
clean:
	rm -f $(PROGS)
