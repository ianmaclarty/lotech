all: libgecode.a

DIRS:=$(shell find gecode -type d)
EMPTY=
DIRS_SEP=$(subst $(EMPTY) $(EMPTY),:,$(DIRS))
INCLUDE_DIRS=-I. 
#-I$(subst $(EMPTY) $(EMPTY), -I,$(DIRS))

VPATH=$(DIRS_SEP)

GECODE_CPP_FILES:=$(shell find gecode -name "*.cpp")
GECODE_O_FILES:=$(patsubst %.cpp,%.o,$(GECODE_CPP_FILES))

GPP_FLAGS=$(INCLUDE_DIRS) $(LTCFLAGS) $(TARGET_FLAGS) -Os -DNDEBUG
GPP=$(CROSS)g++
AR=$(CROSS)ar rcus

$(GECODE_O_FILES): %.o: %.cpp
	@echo CPP $@
	@$(GPP) $(GPP_FLAGS) -c -o $@ $<

libgecode.a: $(GECODE_O_FILES)
	@echo AR $@
	@$(AR) $@ $^

.PHONY: clean
clean:
	find . -name "*.o" -exec rm {} \;
	find . -name "*.a" -exec rm {} \;
