LTDIR=../..

-include $(LTDIR)/Make.params
include ../../Make.common

ifeq ($(TARGET_PLATFORM),osx)
LIBFLAGS= -llt -lpng -lz -llua -lbox2d -lGLEW -lglfw -framework OpenGL \
		-framework OpenAL -framework Cocoa -lpthread \
		-pagezero_size 10000 -image_base 100000000
endif
ifeq ($(TARGET_PLATFORM),linux)
LIBFLAGS= -llt -lpng -lz -llua -lbox2d -lglfw -lGLEW -lopenal -lX11 -lGL
endif

.PHONY: ltclient
ltclient: ltclient.cpp
	g++ $(LTCFLAGS) -I$(LTDIR)/$(TARGET_PLATFORM)/include -L$(LTDIR)/$(TARGET_PLATFORM) ltclient.cpp \
		-o ltclient $(LIBFLAGS)

.PHONY: clean
clean:
	rm ltclient
