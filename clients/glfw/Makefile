LTDIR=../..

-include $(LTDIR)/Make.params
include ../../Make.common

default: ltclient

EXTRA_PREREQS=

ifeq ($(TARGET_PLATFORM),osx)
LIBFLAGS= -llt -lcurl -lpng -lvorbis -lz -llua -lbox2d -lGLEW -lglfw -framework OpenGL \
		-framework OpenAL -framework Cocoa -framework IOKit \
		-pagezero_size 10000 -image_base 100000000
endif

ifeq ($(TARGET_PLATFORM),linux)
ifdef LINUX32
LIBFLAGS=-static-libstdc++ -static-libgcc \
	$(LTDIR)/linux/liblt.a $(LTDIR)/linux/libpng.a $(LTDIR)/linux/libz.a \
	$(LTDIR)/linux/liblua.a $(LTDIR)/linux/libvorbis.a \
	$(LTDIR)/linux/libbox2d.a $(LTDIR)/linux/libglfw.a \
	$(LTDIR)/linux/libGLEW.a $(LTDIR)/linux/libopenal.a \
	$(LTDIR)/linux/libcurl.a \
	-lX11 -lGL -ldl -pthread
else
EXTRA_PREREQS=wrap_memcpy.o
LIBFLAGS=-static-libstdc++ -static-libgcc \
	$(LTDIR)/linux/liblt.a $(LTDIR)/linux/libpng.a $(LTDIR)/linux/libz.a \
	$(LTDIR)/linux/liblua.a $(LTDIR)/linux/libvorbis.a \
	$(LTDIR)/linux/libbox2d.a $(LTDIR)/linux/libglfw.a \
	$(LTDIR)/linux/libGLEW.a $(LTDIR)/linux/libopenal.a \
	$(LTDIR)/linux/libcurl.a \
	 -lX11 -lXrandr -lXi -lXxf86vm -lGL wrap_memcpy.o -Wl,--wrap=memcpy
wrap_memcpy.o:
	gcc -c wrap_memcpy.c
endif
endif

ifeq ($(TARGET_PLATFORM),mingw)
LIBFLAGS= -static -static-libstdc++ -static-libgcc -llt \
		-lcurl -lws2_32 -llua -lpng -lvorbis -lz -lbox2d \
		 -lOpenAL32 -lwinmm -lole32 -ldsound \
		-lglew32 -lglfw -lopengl32 -lgdi32
endif

.PHONY: ltclient
ltclient: ltclient.cpp $(EXTRA_PREREQS)
	g++ $(LTCFLAGS) -I$(LTDIR)/$(TARGET_PLATFORM)/include -L$(LTDIR)/$(TARGET_PLATFORM) ltclient.cpp \
		-o ltclient $(LIBFLAGS)

.PHONY: clean
clean:
	rm -f ltclient$(EXE_EXT)
	rm -f *.o
