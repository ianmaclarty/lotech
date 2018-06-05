all: libbox2d.a

VPATH=Box2D:Box2D/Common:Box2D/Collision:Box2D/Collision/Shapes:Box2D/Dynamics:Box2D/Dynamics/Contacts:Box2D/Dynamics/Joints:Box2D/Rope

BOX2D_CPP_FILES:=$(shell find Box2D -name "*.cpp" -exec basename \{\} \;)
BOX2D_O_FILES:=$(patsubst %.cpp,%.o,$(BOX2D_CPP_FILES))

CPP_FLAGS=-I. $(LTCFLAGS) $(TARGET_FLAGS)

$(BOX2D_O_FILES): %.o: %.cpp
	@echo CPP $@
	@$(CPP) $(CPP_FLAGS) -c $<

libbox2d.a: $(BOX2D_O_FILES)
	@echo AR $@
	@$(AR) rcus $@ $^

.PHONY: clean
clean:
	rm -f *.o *.a
