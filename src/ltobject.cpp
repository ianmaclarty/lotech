/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltobject)

static int num_objs = 0;
#ifdef LTMEMTRACK
static std::set<LTObject*> registry;
#endif

LTObject::LTObject() {
#ifdef LTMEMTRACK
    num_objs++;
    assert(registry.insert(this).second == true);
#endif
}

LTObject::~LTObject() {
#ifdef LTMEMTRACK
    num_objs--;
    assert(registry.erase(this) == 1);
#endif
}

LT_REGISTER_TYPE(LTObject, "lt.Object", NULL)

int ltNumLiveObjects() {
    return num_objs;
}

void ltResetNumLiveObjects() {
    num_objs = 0;
}
