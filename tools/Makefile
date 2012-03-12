LTDIR=..

.PHONY: randtest
randtest: randtest.cpp
	g++ -ObjC++ -g -DLTOSX -I$(LTDIR)/osx/include -L$(LTDIR)/osx randtest.cpp \
		-o randtest -llt -lpng -lz -llua -lbox2d -lGLEW -lglfw -framework OpenGL -framework OpenAL -framework Cocoa

.PHONY: clean
clean:
	rm ltclient
