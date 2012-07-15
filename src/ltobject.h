/* Copyright (C) 2012 Ian MacLarty */

LT_INIT_DECL(ltobject)

struct LTTypeDef;

struct LTObject {
    const LTTypeDef *type;

    //lua_Debug debug;
    LTObject();
    virtual ~LTObject();

    // This is called after a new object is constructed using
    // the default constructor function.
    virtual void init(lua_State *L) {};
};

LTObject *lt_expect_LTObject(lua_State *L, int arg);

int ltNumLiveObjects();
