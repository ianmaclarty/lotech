/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltffi)

#define LT_REGISTER_TYPE(cpp_type, lua_name, lua_parent_name) \
    static void LT_CONCAT(lt_constructor_, __LINE__)(void *buf) { \
        new (buf) cpp_type(); \
    } \
    static LTTypeDef LT_CONCAT(lt_type_def_, __LINE__) = \
        {#cpp_type, lua_name, lua_parent_name, sizeof(cpp_type), \
        LT_CONCAT(&lt_constructor_, __LINE__)}; \
    cpp_type *lt_expect_##cpp_type(lua_State *L, int index) { \
        if (lua_getmetatable(L, index)) { \
            lua_pushlightuserdata(L, (void*)LT_CONCAT(&lt_type_def_, __LINE__)); \
            lua_rawget(L, -2); \
            if (!lua_isnil(L, -1)) { \
                lua_pop(L, 2); \
                cpp_type *val = (cpp_type*)lua_touserdata(L, index); \
                if (val != NULL) { \
                    return val; \
                } \
            } else { \
                lua_pop(L, 2); \
            } \
        } \
        luaL_error(L, "Expecting a value of type %s at position %d", lua_name, index); \
        return NULL; \
    } \
    bool lt_is_##cpp_type(lua_State *L, int index) { \
        if (lua_getmetatable(L, index)) { \
            lua_pushlightuserdata(L, (void*)LT_CONCAT(&lt_type_def_, __LINE__)); \
            lua_rawget(L, -2); \
            if (!lua_isnil(L, -1)) { \
                lua_pop(L, 2); \
                return true; \
            } else { \
                lua_pop(L, 2); \
            } \
        } \
        return false; \
    } \
    void *lt_alloc_##cpp_type(lua_State *L) { \
        return ltLuaAllocUserData(L, LT_CONCAT(&lt_type_def_, __LINE__)); \
    } \
    static LTRegisterType LT_CONCAT(lt_register_type_, __LINE__)(LT_CONCAT(&lt_type_def_, __LINE__));

#define LT_REGISTER_FIELD_BOOL(cpp_type, field_name) \
    static LTbool LT_CONCAT(lt_field_getter_, __LINE__)(LTObject *obj) { \
        return ((cpp_type*)obj)->field_name; \
    } \
    static void LT_CONCAT(lt_field_setter_, __LINE__)(LTObject *obj, LTbool val) { \
        ((cpp_type*)obj)->field_name = val; \
    } \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_BOOL, NULL, \
            (void*)LT_CONCAT(&lt_field_getter_, __LINE__), (void*)LT_CONCAT(&lt_field_setter_, __LINE__), \
            NULL, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_PROPERTY_BOOL(cpp_type, field_name, getter, setter) \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_BOOL, NULL, \
            (void*)getter, (void*)setter, NULL, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_PROPERTY_BOOL_NOCONS(cpp_type, field_name, getter, setter) \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_BOOL, NULL, \
            (void*)getter, (void*)setter, NULL, __LINE__, false}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_FIELD_INT(cpp_type, field_name) \
    static LTint LT_CONCAT(lt_field_getter_, __LINE__)(LTObject *obj) { \
        return ((cpp_type*)obj)->field_name; \
    } \
    static void LT_CONCAT(lt_field_setter_, __LINE__)(LTObject *obj, LTint val) { \
        ((cpp_type*)obj)->field_name = val; \
    } \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_INT, NULL, \
            (void*)LT_CONCAT(&lt_field_getter_, __LINE__), (void*)LT_CONCAT(&lt_field_setter_, __LINE__), \
            NULL, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_FIELD_INT_AS(cpp_type, field_name, lua_name) \
    static LTint LT_CONCAT(lt_field_getter_, __LINE__)(LTObject *obj) { \
        return ((cpp_type*)obj)->field_name; \
    } \
    static void LT_CONCAT(lt_field_setter_, __LINE__)(LTObject *obj, LTint val) { \
        ((cpp_type*)obj)->field_name = val; \
    } \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, lua_name, LT_FIELD_KIND_INT, NULL, \
            (void*)LT_CONCAT(&lt_field_getter_, __LINE__), (void*)LT_CONCAT(&lt_field_setter_, __LINE__), \
            NULL, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_PROPERTY_INT(cpp_type, field_name, getter, setter) \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_INT, NULL, \
            (void*)getter, (void*)setter, NULL, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_FIELD_ENUM(cpp_type, field_name, enum_type, enum_vals) \
    static LTint LT_CONCAT(lt_field_getter_, __LINE__)(LTObject *obj) { \
        return ((cpp_type*)obj)->field_name; \
    } \
    static void LT_CONCAT(lt_field_setter_, __LINE__)(LTObject *obj, LTint val) { \
        ((cpp_type*)obj)->field_name = (enum_type)val; \
    } \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_ENUM, NULL, \
            (void*)LT_CONCAT(&lt_field_getter_, __LINE__), (void*)LT_CONCAT(&lt_field_setter_, __LINE__), \
            enum_vals, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_FIELD_FLOAT(cpp_type, field_name) \
    static LTfloat LT_CONCAT(lt_field_getter_, __LINE__)(LTObject *obj) { \
        return ((cpp_type*)obj)->field_name; \
    } \
    static void LT_CONCAT(lt_field_setter_, __LINE__)(LTObject *obj, LTfloat val) { \
        ((cpp_type*)obj)->field_name = val; \
    } \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_FLOAT, NULL, \
            (void*)LT_CONCAT(&lt_field_getter_, __LINE__), (void*)LT_CONCAT(&lt_field_setter_, __LINE__), \
            NULL, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_FIELD_FLOAT_AS(cpp_type, field_name, lua_name) \
    static LTfloat LT_CONCAT(lt_field_getter_, __LINE__)(LTObject *obj) { \
        return ((cpp_type*)obj)->field_name; \
    } \
    static void LT_CONCAT(lt_field_setter_, __LINE__)(LTObject *obj, LTfloat val) { \
        ((cpp_type*)obj)->field_name = val; \
    } \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, lua_name, LT_FIELD_KIND_FLOAT, NULL, \
            (void*)LT_CONCAT(&lt_field_getter_, __LINE__), (void*)LT_CONCAT(&lt_field_setter_, __LINE__), \
            NULL, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_PROPERTY_FLOAT(cpp_type, field_name, getter, setter) \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_FLOAT, NULL, \
            (void*)getter, (void*)setter, NULL, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_PROPERTY_FLOAT_NOCONS(cpp_type, field_name, getter, setter) \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_FLOAT, NULL, \
            (void*)getter, (void*)setter, NULL, __LINE__, false}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_PROPERTY_STRING(cpp_type, field_name, getter, setter) \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_STRING, NULL, \
            (void*)getter, (void*)setter, NULL, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_PROPERTY_STRING_NOCONS(cpp_type, field_name, getter, setter) \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_STRING, NULL, \
            (void*)getter, (void*)setter, NULL, __LINE__, false}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_FIELD_OBJ(cpp_type, field_name, value_type) \
    static LTObject* LT_CONCAT(lt_field_getter_, __LINE__)(LTObject *obj) { \
        return ((cpp_type*)obj)->field_name; \
    } \
    static void LT_CONCAT(lt_field_setter_, __LINE__)(LTObject *obj, LTObject *val) { \
        ((cpp_type*)obj)->field_name = (value_type*)val; \
    } \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_OBJECT, #value_type, \
            (void*)LT_CONCAT(&lt_field_getter_, __LINE__), (void*)LT_CONCAT(&lt_field_setter_, __LINE__), \
            NULL, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_PROPERTY_OBJ(cpp_type, field_name, value_type, getter, setter) \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_OBJECT, #value_type, \
            (void*)getter, (void*)setter, NULL, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

#define LT_REGISTER_METHOD(cpp_type, field_name, function) \
    static lua_CFunction LT_CONCAT(lt_lua_func_typecheck_, __LINE__) = &function; \
    static LTFieldDef LT_CONCAT(lt_field_def_, __LINE__) = \
        {#cpp_type, #field_name, LT_FIELD_KIND_METHOD, NULL, \
            (void*)LT_CONCAT(lt_lua_func_typecheck_, __LINE__), NULL, NULL, __LINE__, true}; \
    static LTRegisterField LT_CONCAT(lt_register_field_, __LINE__)(LT_CONCAT(&lt_field_def_, __LINE__));

enum LTFieldKind {
    LT_FIELD_KIND_FLOAT,
    LT_FIELD_KIND_INT,
    LT_FIELD_KIND_BOOL,
    LT_FIELD_KIND_ENUM,
    LT_FIELD_KIND_STRING,
    LT_FIELD_KIND_OBJECT,
    LT_FIELD_KIND_METHOD,
};

struct LTTypeDef {
    const char *cpp_name;
    const char *lua_name; // Lua name, including module table, e.g. "lt.Translate"
    const char *parent_lua_name; // Lua name of parent class.
    const int size; // bytes
    void (*default_constructor)(void*);
};

struct LTEnumConstant {
    const char *name;
    int val;
};

struct LTFieldDef {
    const char *cpp_type_name;
    const char *name; // Lua name
    LTFieldKind kind;
    const char *value_cpp_type_name; // Only relevant when kind == LT_FIELD_KIND_OBJECT
    void* getter; // also used for methods
    void* setter; 
    const LTEnumConstant *enum_vals; // NULL terminated array
    int line;
    bool include_in_constructor;
};

struct LTRegisterType {
    LTRegisterType(const LTTypeDef *type);
};

struct LTRegisterField {
    LTRegisterField(const LTFieldDef *field);
};

typedef LTfloat     (*LTFloatGetter)(LTObject*);
typedef void        (*LTFloatSetter)(LTObject*, LTfloat);
typedef LTint       (*LTIntGetter)(LTObject*);
typedef void        (*LTIntSetter)(LTObject*, LTint);
typedef LTbool      (*LTBoolGetter)(LTObject*);
typedef void        (*LTBoolSetter)(LTObject*, LTbool);
typedef LTstring    (*LTStringGetter)(LTObject*);
typedef void        (*LTStringSetter)(LTObject*, LTstring);
typedef LTObject*   (*LTObjGetter)(LTObject*);
typedef void        (*LTObjSetter)(LTObject*, LTObject*);

void ltLuaInitFFI(lua_State *L);
int ltLuaAddRef(lua_State *L, int obj, int val);
void ltLuaAddNamedRef(lua_State *L, int obj, int val, const char* name);
void ltLuaGetNamedRef(lua_State *L, int obj, const char* name);
void ltLuaDelRef(lua_State *L, int obj, int ref);
void ltLuaGetRef(lua_State *L, int obj, int ref);
int ltLuaCheckNArgs(lua_State *L, int n);
void* ltLuaAllocUserData(lua_State *L, LTTypeDef *type);
void ltLuaGetFloatGetterAndSetter(lua_State *L, int obj_index, int field_index, LTFloatGetter *getter, LTFloatSetter *setter);
void ltLuaGetIntGetterAndSetter(lua_State *L, int obj_index, int field_index, LTIntGetter *getter, LTIntSetter *setter);
void ltLuaFindFieldOwner(lua_State *L, int obj_index, int field_index);
