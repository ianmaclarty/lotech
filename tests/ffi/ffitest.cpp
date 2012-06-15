#include "lt.h"

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Error: expecting exactly one argument\n");
        exit(1);
    }
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    ltLuaInitFFI(L);
    luaL_loadfile(L, argv[1]);
    if (lua_pcall(L, 0, 0, 0) != 0) {
        const char *errmsg = lua_tostring(L, -1);
        printf("Error: %s\n", errmsg);
    }
    lua_close(L);
    return 0;
}

enum TestEnum {
    EnumVal1,
    EnumVal2,
    EnumVal3,
};

struct TestStruct1 : LTObject {
    LTint int_field;
    LTfloat float_field;
    TestEnum enum_field;
    virtual void init(lua_State *L) { printf("init:int_field = %d\n", int_field); }
    TestStruct1() {
        int_field = 99;
        float_field = 2.2;
    }
    virtual ~TestStruct1() {};
};

LT_REGISTER_TYPE(TestStruct1, "test.TestStruct1", NULL)
LT_REGISTER_FIELD_INT(TestStruct1, int_field)
LT_REGISTER_FIELD_FLOAT(TestStruct1, float_field)
static LTEnumConstant TestStruct1_enums[] = {
    {"val1", EnumVal1},
    {"val2", EnumVal2},
    {"val3", EnumVal3},
    {NULL, 0},
};
LT_REGISTER_FIELD_ENUM(TestStruct1, enum_field, TestEnum, TestStruct1_enums);

struct TestStruct2 : TestStruct1 {
    LTbool bool_field;
    TestStruct1 *obj_field;
    std::list<TestStruct1 *> obj_list;
    TestStruct2() {
        bool_field = true;
        obj_field = NULL;
    }
    virtual ~TestStruct2() {};
};

static int add_one(lua_State *L);
static int insert_obj(lua_State *L);
static int remove_obj(lua_State *L);
static int count_obj(lua_State *L);

LT_REGISTER_TYPE(TestStruct2, "test.TestStruct2", "test.TestStruct1")
LT_REGISTER_FIELD_BOOL( TestStruct2, bool_field)
LT_REGISTER_FIELD_OBJ(  TestStruct2, obj_field,     TestStruct1)
LT_REGISTER_METHOD(     TestStruct2, add_one,       add_one)
LT_REGISTER_METHOD(     TestStruct2, insert_obj,    insert_obj)
LT_REGISTER_METHOD(     TestStruct2, remove_obj,    remove_obj)
LT_REGISTER_METHOD(     TestStruct2, count_obj,     count_obj)

static int add_one(lua_State *L) {
    TestStruct2 *s = lt_expect_TestStruct2(L, 1);
    s->int_field++;
    return 0;
}

static int insert_obj(lua_State *L) {
    TestStruct2 *s = lt_expect_TestStruct2(L, 1);
    TestStruct1 *o = lt_expect_TestStruct1(L, 2);
    s->obj_list.push_back(o);
    ltLuaAddRef(L, 1, 2);
    return 0;
}

static int remove_obj(lua_State *L) {
    TestStruct2 *s = lt_expect_TestStruct2(L, 1);
    TestStruct1 *o = lt_expect_TestStruct1(L, 2);
    std::list<TestStruct1 *>::iterator it;
    for (it = s->obj_list.begin(); it != s->obj_list.end(); it++) {
        if (*it == o) {
            s->obj_list.erase(it);
            ltLuaDelRef(L, 1, 2);
            break;
        }
    }
    return 0;
}

static int count_obj(lua_State *L) {
    TestStruct2 *s = lt_expect_TestStruct2(L, 1);
    lua_pushinteger(L, s->obj_list.size());
    return 1;
}

struct TestStruct3 : TestStruct2 {
    virtual void init(lua_State *L) { ltAbort(); }
    TestStruct3() { ltAbort(); }
    TestStruct3(LTint i) {
        int_field = i;
        float_field = 888.0;
        obj_field = NULL;
        bool_field = true;
    }
};

static int new_TestStruct3(lua_State *L);

LT_REGISTER_TYPE(TestStruct3, "test.TestStruct3", "test.TestStruct2")
LT_REGISTER_METHOD(TestStruct3, new,       new_TestStruct3)

static int new_TestStruct3(lua_State *L) {
    ltLuaCheckNArgs(L, 1);
    new (lt_alloc_TestStruct3(L)) TestStruct3(luaL_checkinteger(L, 1));
    return 1;
}
