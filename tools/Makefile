include ../Make.common

LTDIR=..

.PHONY: randtest
ifeq ($(TARGET_PLATFORM),osx)
randtest: randtest.cpp
	g++ -ObjC++ -g -DLTOSX -I$(LTDIR)/osx/include -L$(LTDIR)/osx randtest.cpp \
		-o randtest -llt -lpng -lz -llua -lbox2d -lGLEW -lglfw -framework OpenGL -framework OpenAL -framework Cocoa
else
randtest: randtest.cpp
	g++ -g -DLTLINUX -I$(LTDIR)/linux/include -L$(LTDIR)/linux randtest.cpp \
		-o randtest -llt -lpng -lz -llua -lbox2d -lGLEW -lglfw -lopenal
endif

.PHONY: clean
clean:
	rm ltclient
