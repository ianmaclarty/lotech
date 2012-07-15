#include "lt.h"

LT_INIT_IMPL(ltobject)

static int num_objs = 0;

LTObject::LTObject() {
    num_objs++;
    //memset(&debug, 0, sizeof(lua_Debug));
    type = NULL;
}

LTObject::~LTObject() {
    num_objs--;
}

LT_REGISTER_TYPE(LTObject, "lt.Object", NULL)

int ltNumLiveObjects() {
    return num_objs;
}
