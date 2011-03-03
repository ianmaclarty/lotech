default: harness

VPATH=../src/lt:../src/lua:../src/zlib:../src/lpng:../src/Box2D:../src/Box2D/Common:../src/Box2D/Collision:../src/Box2D/Collision/Shapes:../src/Box2D/Dynamics:../src/Box2D/Dynamics/Contacts:../src/Box2D/Dynamics/Joints:../tests:../tools

GPP=g++
GCC=gcc

UNAME=$(shell uname)

COMMONDEFS=-DLTDEVMODE -DLTDEPTHBUF

ifeq "$(UNAME)" "Linux"
DEBUG_OPTS=-g
LD_OPTS=$(DEBUG_OPTS) -lglut -lGL -lm
DEFINES=-DLTLINUX -DLUA_USE_LINUX -DLTGLUT $(COMMONDEFS)
else
DEBUG_OPTS=-gdwarf-2
LD_OPTS=$(DEBUG_OPTS) -framework GLUT -framework OpenGL -framework Cocoa -lm
DEFINES=-DLUA_USE_MACOSX -DLTGLUT $(COMMONDEFS)
endif

GCC_OPTS=$(DEFINES) $(DEBUG_OPTS) -I../src/lt -I../src/zlib -I../src/lpng -I../src -I../src/lua -Wall -O0

LT_CPP_FILES:=$(shell find ../src/lt/ -name "*.cpp" -exec basename \{\} \;)
LT_H_FILES:=$(shell find ../src/lt/ -name "*.h" -exec basename \{\} \;)
LT_O_FILES:=$(patsubst %.cpp,%.o,$(LT_CPP_FILES))

BOX2D_CPP_FILES:=$(shell find ../src/Box2D -name "*.cpp" -exec basename \{\} \;)
BOX2D_H_FILES:=$(shell find ../src/Box2D -name "*.h" -exec basename \{\} \;)
BOX2D_O_FILES:=$(patsubst %.cpp,%.o,$(BOX2D_CPP_FILES))

ZLIB_C_FILES:=$(shell find ../src/zlib -name "*.c" -exec basename \{\} \;)
ZLIB_H_FILES:=$(shell find ../src/zlib -name "*.h" -exec basename \{\} \;)
ZLIB_O_FILES:=$(patsubst %.c,%.o,$(ZLIB_C_FILES))

LPNG_C_FILES:=$(shell find ../src/lpng -name "*.c" -exec basename \{\} \;)
LPNG_H_FILES:=$(shell find ../src/lpng -name "*.h" -exec basename \{\} \;)
LPNG_O_FILES:=$(patsubst %.c,%.o,$(LPNG_C_FILES))

LUA_C_FILES:=$(shell find ../src/lua -name "*.c" -exec basename \{\} \;)
LUA_H_FILES:=$(shell find ../src/lua -name "*.h" -exec basename \{\} \;)
LUA_O_FILES:=$(patsubst %.c,%.o,$(LUA_C_FILES))

TEST_CPP_FILES:=$(shell find ../tests/ -name "*.cpp" -exec basename \{\} \;)
TEST_O_FILES:=$(patsubst %.cpp,%.o,$(TEST_CPP_FILES))
TEST_EXES:=$(patsubst %.cpp,%,$(TEST_CPP_FILES))

TOOL_CPP_FILES:=$(shell find ../tools/ -name "*.cpp" -exec basename \{\} \;)
TOOL_O_FILES:=$(patsubst %.cpp,%.o,$(TOOL_CPP_FILES))
TOOL_EXES:=$(patsubst %.cpp,%,$(TOOL_CPP_FILES))

CPP_O_FILES:=$(LT_O_FILES) $(BOX2D_O_FILES) 
C_O_FILES:=$(LPNG_O_FILES) $(ZLIB_O_FILES) $(LUA_O_FILES)
O_FILES:=$(CPP_O_FILES) $(C_O_FILES) 
C_H_FILES:=$(ZLIB_H_FILES) $(LPNG_H_FILES) $(LUA_H_FILES)

$(TEST_EXES): %: %.o $(O_FILES)
	$(GPP) $^ $(LD_OPTS) -o $@

$(TOOL_EXES): %: %.o $(O_FILES)
	$(GPP) $^ $(LD_OPTS) -o $@

$(LT_O_FILES) $(TEST_O_FILES) $(TOOL_O_FILES): %.o: %.cpp $(LT_H_FILES)
	$(GPP) $(GCC_OPTS) -c $<

$(BOX2D_O_FILES): %.o: %.cpp $(BOX2D_H_FILES)
	$(GPP) $(GCC_OPTS) -c $<

$(C_O_FILES): %.o: %.c $(C_H_FILES)
	$(GCC) $(GCC_OPTS) -c $<

%.dSYM: %
	dsymutil $^

.PHONY: tags
tags:
	ctags `find .. -name "*.h"` `find .. -name "*.cpp"`

clean:
	rm -f *.o
	rm -f $(TEST_EXES)
	rm -f $(TOOL_EXES)
	rm -f cocoa2
	rm -rf *.dSYM

cocoa2: cocoa2.o
	gcc $(LD_OPTS) -o cocoa2 cocoa2.o

cocoa2.o: ../tests/cocoa2.m
	gcc -c ../tests/cocoa2.m
