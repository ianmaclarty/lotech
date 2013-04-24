LTDIR=../..

-include $(LTDIR)/Make.params
include ../../Make.common

ifeq ($(TARGET_PLATFORM),osx)
LIBFLAGS= -llt -lpng -lz -llua -lbox2d -lGLEW -lglfw -framework OpenGL \
		-framework OpenAL -framework Cocoa \
		-pagezero_size 10000 -image_base 100000000
endif
ifeq ($(TARGET_PLATFORM),linux)
LIBFLAGS= -llt -lpng -lz -llua -lbox2d -lglfw -lGLEW -lopenal -lX11 -lGL
endif
ifeq ($(TARGET_PLATFORM),mingw)
LIBFLAGS= -static -static-libstdc++ -static-libgcc -llt -llua -lpng -lz -lbox2d \
		 -lOpenAL32 -lwinmm -lole32 \
		-lglew32s -lglfw -lopengl32 \
		"/c/Program Files (x86)/Microsoft DirectX 9.0 SDK (Summer 2004)/Lib/dsound.lib" 
endif

.PHONY: ltclient
ltclient: ltclient.cpp
	g++ $(LTCFLAGS) -I$(LTDIR)/$(TARGET_PLATFORM)/include -L$(LTDIR)/$(TARGET_PLATFORM) ltclient.cpp \
		-o ltclient $(LIBFLAGS)

.PHONY: clean
clean:
	rm ltclient
