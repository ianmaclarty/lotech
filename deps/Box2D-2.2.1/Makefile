include ../../Common.mk

all: libbox2d$(ALIB_EXT)

VPATH=Box2D:Box2D/Common:Box2D/Collision:Box2D/Collision/Shapes:Box2D/Dynamics:Box2D/Dynamics/Contacts:Box2D/Dynamics/Joints:Box2D/Rope

CPP_FILES:=$(shell find Box2D -name "*.cpp" -exec basename \{\} \;)
O_FILES:=$(patsubst %.cpp,%$(OBJ_EXT),$(CPP_FILES))

CPPFLAGS = $(COMMON_CFLAGS) -I.

$(O_FILES): %$(OBJ_EXT): %.cpp
	$(CPP) $(CPPFLAGS) $(NOLINK_OPT) $< $(OBJ_OUT_OPT)$@

libbox2d$(ALIB_EXT): $(O_FILES)
	$(AR) $(AR_OPTS) $(AR_OUT_OPT)$@ $^

.PHONY: clean
clean:
	rm -f *.o *.a *.lib *.obj *.o-arm* *.o-sim*
