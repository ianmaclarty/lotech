LTDIR=../..

-include $(LTDIR)/Make.params
include ../../Make.common

.PHONY: ltclient
ltclient: ltclient.cpp
	g++ $(LTCFLAGS) -I$(LTDIR)/$(TARGET_PLATFORM)/include -L$(LTDIR)/$(TARGET_PLATFORM) ltclient.cpp \
		-o ltclient -llt -lpng -lz -llua -lbox2d -lglfw -lGLEW -lopenal -lX11 -lGL

.PHONY: clean
clean:
	rm ltclient
