all: libbox2d.a

VPATH=Box2D:Box2D/Common:Box2D/Collision:Box2D/Collision/Shapes:Box2D/Dynamics:Box2D/Dynamics/Contacts:Box2D/Dynamics/Joints:Box2D/Rope

BOX2D_CPP_FILES:=$(shell find Box2D -name "*.cpp" -exec basename \{\} \;)
BOX2D_O_FILES:=$(patsubst %.cpp,%.o,$(BOX2D_CPP_FILES))

ifndef LTCPP
LTCPP=g++
endif

GPP_FLAGS=-I. $(LTCFLAGS) $(TARGET_FLAGS)
GPP=$(CROSS)$(LTCPP)
ifndef LTAR
AR=$(CROSS)ar rcus
else
AR=$(LTAR)
endif

$(BOX2D_O_FILES): %.o: %.cpp
	@echo CPP $@
	@$(GPP) $(GPP_FLAGS) -c $<

libbox2d.a: $(BOX2D_O_FILES)
	@echo AR $@
	@$(AR) $@ $^

.PHONY: clean
clean:
	rm -f *.o *.a
