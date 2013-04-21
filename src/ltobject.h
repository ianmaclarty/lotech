/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltobject)

struct LTTypeDef;

struct LTObject {
    LTObject();
    virtual ~LTObject();

    // This is called after a new object is constructed using
    // the default constructor function.
    virtual void init(lua_State *L) {};
};

LTObject *lt_expect_LTObject(lua_State *L, int arg);

int ltNumLiveObjects();
void ltResetNumLiveObjects();
