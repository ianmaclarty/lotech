/* Copyright (C) 2012 Ian MacLarty */
#include "lt.h"

LT_INIT_IMPL(ltffi)

static inline int absidx(lua_State *L, int index) {
    if (index < 0) {
        return lua_gettop(L) + index + 1;
    } else {
        return index;
    }
}

static void init_core_modules() {
    // These are only to ensure the static initializers in each translation unit are run.
    lt3d_init();
    ltaction_init();
    ltaudio_init();
    ltcommon_init();
    ltconfig_init();
    ltevent_init();
    ltffi_init();
    ltfilestore_init();
    ltgraphics_init();
    ltimage_init();
    ltlua_init();
    ltnet_init();
    ltobject_init();
    ltopengl_init();
    ltparticles_init();
    ltphysics_init();
    ltpickle_init();
    ltprotocol_init();
    ltrandom_init();
    ltrendertarget_init();
    ltresource_init();
    ltscene_init();
    ltstate_init();
    ltstore_init();
    lttext_init();
    lttime_init();
    lttween_init();
    ltutil_init();
    ltvector_init();
    ltmesh_init();
    ltwavefront_init();
    ltlighting_init();
}

struct LTFieldInfo {
    LTFieldKind kind;
    void *getter;
    void *setter;
    const LTTypeDef *value_type;
};

static std::vector<const LTTypeDef*>    *type_registry = NULL;
static std::vector<const LTFieldDef*>   *field_def_registry = NULL;
LTFieldInfo *field_info_registry = NULL;

LTRegisterType::LTRegisterType(const LTTypeDef *type) {
    if (type_registry == NULL) {
        type_registry = new std::vector<const LTTypeDef*>();
    }
    type_registry->push_back(type);
}

LTRegisterField::LTRegisterField(const LTFieldDef *field) {
    if (field_def_registry == NULL) {
        field_def_registry = new std::vector<const LTFieldDef*>();
    }
    field_def_registry->push_back(field);
}

static std::vector<int> type_parent;

static bool str_cmp(const char *a, const char *b) {
    return strcmp(a, b) < 0;
}

static void init_type_parent() {
    std::map<const char *, int, bool (*)(const char*, const char*)> type_by_name(&str_cmp);
    int n = type_registry->size();
    const char* name;
    const char* parent;
    type_parent.resize(n);
    for (int i = 0; i < n; i++) {
        name = (*type_registry)[i]->lua_name;
        if (type_by_name.find(name) != type_by_name.end()) {
            ltLog("Error: Duplicate entries for type %s", name);
            ltAbort();
        }
        type_by_name[name] = i;
    }
    std::map<const char *, int, bool (*)(const char*, const char*)>::iterator it;
    for (int i = 0; i < n; i++) {
        name = (*type_registry)[i]->lua_name;
        parent = (*type_registry)[i]->parent_lua_name;
        if (parent == NULL) {
            type_parent[i] = -1;
        } else {
            it = type_by_name.find(parent);
            if (it == type_by_name.end()) {
                ltLog("Error: Parent %s for type %s not registered", parent, name);
                ltAbort();
            } else {
                type_parent[i] = it->second;
            }
        }
    }
}

// -----------------

static void sort_field_def_registry();
static void init_field_info_registry();
static void init_metatables(lua_State *L);
static void init_constructors(lua_State *L);

void ltLuaInitFFI(lua_State *L) {
    init_core_modules();
    if (type_registry == NULL) {
        type_registry = new std::vector<const LTTypeDef *>();
    }
    if (field_def_registry == NULL) {
        field_def_registry = new std::vector<const LTFieldDef *>();
    }
    sort_field_def_registry();
    init_field_info_registry();
    init_type_parent();
    init_metatables(L);
    init_constructors(L);
}

static bool field_def_cmp(const LTFieldDef *def1, const LTFieldDef *def2) {
    int r = strcmp(def1->cpp_type_name, def2->cpp_type_name);
    if (r == 0) {
        return def1->line < def2->line;
    } else {
        return r < 0;
    }
}

static void sort_field_def_registry() {
    std::sort(field_def_registry->begin(), field_def_registry->end(), field_def_cmp);
}

static void init_field_info_registry() {
    int n = field_def_registry->size();
    if (field_info_registry != NULL) {
        delete[] field_info_registry;
    }
    field_info_registry = new LTFieldInfo[n];
    for (int i = 0; i < n; i++) {
        LTFieldInfo *info = &field_info_registry[i];
        const LTFieldDef *def = (*field_def_registry)[i];
        info->kind = def->kind;
        info->getter = def->getter;
        info->setter = def->setter;
        if (def->value_cpp_type_name != NULL) {
            bool found_type = false;
            for (unsigned int j = 0; j < type_registry->size(); j++) {
                const LTTypeDef *type = (*type_registry)[j];
                if (strcmp(type->cpp_name, def->value_cpp_type_name) == 0) {
                    info->value_type = type;
                    found_type = true;
                    break;
                }
            }
            if (!found_type) {
                ltLog("Unable to find value type %s for field %s on %s", def->value_cpp_type_name, def->name, def->cpp_type_name);
                ltAbort();
            }
        } else {
            info->value_type = NULL;
        }
    }
}

//--------------

// Expects: obj index = 1, field index = 2
// NOTE: May push other values onto stack below the field value.
static inline int push_field_val(lua_State *L, LTObject *obj, LTFieldInfo *field) {
    switch (field->kind) {
        case LT_FIELD_KIND_FLOAT: {
            LTFloatGetter getter = (LTFloatGetter)field->getter;
            lua_pushnumber(L, getter(obj));
            break;
        }
        case LT_FIELD_KIND_INT: {
            LTIntGetter getter = (LTIntGetter)field->getter;
            lua_pushinteger(L, getter(obj));
            break;
        }
        case LT_FIELD_KIND_BOOL: {
            LTBoolGetter getter = (LTBoolGetter)field->getter;
            lua_pushboolean(L, getter(obj));
            break;
        }
        case LT_FIELD_KIND_ENUM: {
            LTIntGetter getter = (LTIntGetter)field->getter;
            int val = getter(obj);
            lua_getmetatable(L, 1);
            lua_pushlightuserdata(L, (void*)field);
            lua_rawget(L, -2);
            lua_rawgeti(L, -1, val + 1);
            break;
        }
        case LT_FIELD_KIND_OBJECT:
            // Value will be recorded in env table, so just look it up.
            lua_getfenv(L, 1);
            lua_pushvalue(L, 2); // push field name
            lua_rawget(L, -2);
            break;
        case LT_FIELD_KIND_METHOD:
            // Methods should have been detected before this function was called.
            ltLog("push_field_val: method");
            ltAbort();
            break;
    }
    return 1;
}

static inline void set_field_val(lua_State *L, LTObject *obj, LTFieldInfo *field,
        int obj_index, int field_index, int val_index) {
    obj_index = absidx(L, obj_index);
    field_index = absidx(L, field_index);
    val_index = absidx(L, val_index);
    if (field->setter == NULL) {
        const char *field_name = lua_tostring(L, field_index);
        luaL_error(L, "Attempt to set readonly field '%s'", field_name);
    }
    switch (field->kind) {
        case LT_FIELD_KIND_FLOAT: {
            LTFloatSetter setter = (LTFloatSetter)field->setter;
            setter(obj, (LTfloat)luaL_checknumber(L, val_index));
            break;
        }
        case LT_FIELD_KIND_INT: {
            LTIntSetter setter = (LTIntSetter)field->setter;
            setter(obj, (LTint)luaL_checkinteger(L, val_index));
            break;
        }
        case LT_FIELD_KIND_BOOL: {
            LTBoolSetter setter = (LTBoolSetter)field->setter;
            setter(obj, (LTbool)lua_toboolean(L, val_index));
            break;
        }
        case LT_FIELD_KIND_ENUM: {
            LTIntSetter setter = (LTIntSetter)field->setter;
            lua_getmetatable(L, obj_index);
            lua_pushlightuserdata(L, (void*)field);
            lua_rawget(L, -2);
            lua_pushvalue(L, val_index);
            lua_rawget(L, -2);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 3); // pop nil, enum values table and metatable.
                const char *field_name = lua_tostring(L, field_index);
                luaL_error(L, "Invalid value for field '%s': '%s'", field_name, lua_tostring(L, val_index));
            } else {
                int val = lua_tointeger(L, -1);
                lua_pop(L, 3); // pop val, enum values table and metatable.
                setter(obj, val);
            }
            break;
        }
        case LT_FIELD_KIND_OBJECT: {
            LTObject *val = (LTObject*)lua_touserdata(L, val_index);
            if (val == NULL) {
                const char *field_name = lua_tostring(L, field_index);
                luaL_error(L, "Attempt to assign non-userdata value to field '%s'", field_name);
            }
            // Check val has the right type by searching for the 
            // LTTypeDef pointer in the metatable (this would have
            // been added when the metatable was created).
            if (!lua_getmetatable(L, val_index)) {
                const char *field_name = lua_tostring(L, field_index);
                luaL_error(L, "Field '%s' expects a value of type '%s' (value has no metatable)",
                    field_name, field->value_type->lua_name);
            }
            lua_pushlightuserdata(L, (void*)field->value_type);
            lua_rawget(L, -2);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 2); // pop nil and metatable.
                const char *field_name = lua_tostring(L, field_index);
                luaL_error(L, "Field '%s' expects a value of type '%s'", field_name, field->value_type->lua_name);
            }
            lua_pop(L, 2); // pop value and metatable
            // Type ok
            LTObjSetter setter = (LTObjSetter)field->setter;
            setter(obj, val);
            // Record value in obj's env table so it's not gc'd.
            lua_getfenv(L, obj_index);
            lua_pushvalue(L, field_index); // push field name
            lua_pushvalue(L, val_index); // push value
            lua_rawset(L, -3);
            lua_pop(L, 1); // pop env table.
            break;
        }
        case LT_FIELD_KIND_METHOD:
            // Cannot set a method.
            const char *field_name = lua_tostring(L, field_index);
            luaL_error(L, "Attempt to set method field '%s'", field_name);
            break;
    }
}

// upvalue 1 = the method
// upvalue 2 = the receiver
static int wrap_node_method_forward(lua_State *L) {
    int nargs = lua_gettop(L);
    lua_pushvalue(L, lua_upvalueindex(2)); // Push correct receiver
    lua_replace(L, 1); // Replace first argument with correct receiver
    lua_pushvalue(L, lua_upvalueindex(1)); // Push the method
    lua_insert(L, 1); // Insert the method at the top
    lua_call(L, nargs, LUA_MULTRET); // Call the method
    return lua_gettop(L);
}

static bool lookup_wrap_node_field(lua_State *L) {
    while (lt_is_LTWrapNode(L, 1)) {
        lua_getfenv(L, 1);
        lua_pushstring(L, "child");
        lua_rawget(L, -2);
        lua_replace(L, 1);
        lua_pop(L, 1); // pop env table
        lua_getmetatable(L, 1);
        lua_pushvalue(L, 2); // push field name
        lua_rawget(L, -2);
        if (!lua_isnil(L, -1)) {
            LTFieldInfo *field = (LTFieldInfo*)lua_touserdata(L, -1);
            if (field != NULL) {
                lua_pop(L, 2); // pop metatable and field value
                LTObject *obj = (LTObject*)lua_touserdata(L, 1);
                push_field_val(L, obj, field);
                return true;
            } else if (lua_isfunction(L, -1)) {
                // The value is a method.  We must create a new method with the correct
                // receiver.
                lua_pushvalue(L, 1);
                lua_pushcclosure(L, wrap_node_method_forward, 2);
                return true;
            } else {
                return true; // a non-method constant
            }
        } else {
            lua_pop(L, 2); // pop metatable and field value (nil)
        }
    }
    return false;
}

static bool set_wrap_node_field(lua_State *L) {
    lua_pushvalue(L, 1);
    int n = 1;
    while (lt_is_LTWrapNode(L, -1)) {
        lua_getfenv(L, -1); // push env table
        lua_pushstring(L, "child");
        lua_rawget(L, -2); // push child
        lua_getmetatable(L, -1); // push metatable
        lua_pushvalue(L, 2);
        lua_rawget(L, -2); // push field info
        LTFieldInfo *field = (LTFieldInfo*)lua_touserdata(L, -1);
        lua_pop(L, 2); // pop field info and metatable
        if (field != NULL) {
            LTObject *child = (LTObject*)lua_touserdata(L, -1);
            set_field_val(L, child, field, -1, 2, 3);
            return true;
        }
        n += 2; // so we pop env table and child later
    }
    lua_pop(L, n);
    return false;
}

static int index_func(lua_State *L) {
    LTObject *obj = (LTObject *)lua_touserdata(L, 1);
    if (obj != NULL) {
        lua_getmetatable(L, 1); // push metatable
        lua_pushvalue(L, 2); // push field name
        lua_rawget(L, -2); // lookup field in metatable
        LTFieldInfo *field = (LTFieldInfo*)lua_touserdata(L, -1);
        if (field != NULL) {
            return push_field_val(L, obj, field);
        } else {
            if (!lua_isnil(L, -1)) {
                // Could be a method or some other constant.
                return 1;
            } else {
                lua_pop(L, 1); // pop nil
                // Get value from env table.
                lua_getfenv(L, 1);
                lua_pushvalue(L, 2);
                lua_rawget(L, -2);
                if (!lua_isnil(L, -1)) {
                    return 1;
                } else {
                    lua_pop(L, 1); // pop nil
                    if (lookup_wrap_node_field(L)) {
                        return 1;
                    } else {
                        lua_pushnil(L);
                        return 1;
                    }
                }
            }
        }
    } else {
        return luaL_error(L, "index_func: obj == NULL");
    }
}

static int newindex_func(lua_State *L) {
    if (lua_type(L, 2) != LUA_TSTRING) {
        luaL_error(L, "Field not a string");
    }
    LTObject *obj = (LTObject *)lua_touserdata(L, 1);
    if (obj != NULL) {
        lua_getmetatable(L, 1); // push metatable
        lua_pushvalue(L, 2); // push field name
        lua_rawget(L, -2); // lookup field in metatable
        LTFieldInfo *field = (LTFieldInfo*)lua_touserdata(L, -1);
        lua_pop(L, 2); // pop field info and metatable
        if (field != NULL) {
            set_field_val(L, obj, field, 1, 2, 3);
        } else {
            if (!set_wrap_node_field(L)) {
                // field not in metatable, set it in the env table
                lua_getfenv(L, 1);
                lua_pushvalue(L, 2);
                lua_pushvalue(L, 3);
                lua_rawset(L, -3);
            }
        }
        return 0;
    } else {
        return luaL_error(L, "newindex_func: obj == NULL");
    }
}

static int gc_func(lua_State *L) {
    LTObject *obj = (LTObject *)lua_touserdata(L, 1);
    if (obj != NULL) {
        obj->~LTObject();
    }
    return 0;
}


static void push_metatable(lua_State *L, int type_id) {
    lua_newtable(L);
    lua_pushcfunction(L, index_func);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, newindex_func);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, gc_func);
    lua_setfield(L, -2, "__gc");

    // Set "type" field.
    lua_pushstring(L, (*type_registry)[type_id]->lua_name);
    lua_setfield(L, -2, "type");

    // Set "is" field as a table holding type names of this and ancestor
    // types.  Used for runtime type checking in Lua.
    lua_newtable(L);
    int t = type_id;
    while (t >= 0) {
        lua_pushstring(L, ((*type_registry)[t])->lua_name);
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
        t = type_parent[t];
    }
    lua_setfield(L, -2, "is");

    // Add all valid LTTypeDefs for the type to the metatable.
    // We use this to do runtime type checking in C++.
    t = type_id;
    while (t >= 0) {
        const LTTypeDef* type = ((*type_registry)[t]);
        lua_pushlightuserdata(L, (void*)type);
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
        t = type_parent[t];
    }

    std::list<const char *> cons_args; // List of field names use as arguments for default constructor.
    std::list<const char *>::iterator cons_it;

    std::list<int> enum_fields;

    // Add field infos, indexed by field name.
    t = type_id;
    while (t >= 0) {
        const LTTypeDef *type = (*type_registry)[t];
        cons_it = cons_args.begin();
        for (unsigned int i = 0; i < field_def_registry->size(); i++) {
            LTFieldInfo *field_info = &field_info_registry[i];
            const LTFieldDef *field_def = (*field_def_registry)[i];
            if (strcmp(field_def->cpp_type_name, type->cpp_name) == 0) {
                lua_getfield(L, -1, field_def->name);
                if (lua_isnil(L, -1)) {
                    lua_pop(L, 1); // pop nil
                    if (field_def->kind == LT_FIELD_KIND_METHOD) {
                        // We place methods directly in the metatable.
                        lua_pushcfunction(L, (lua_CFunction)field_def->getter);
                    } else {
                        lua_pushlightuserdata(L, (void*)field_info);
                        if (field_def->setter != NULL && field_def->include_in_constructor) {
                            cons_it = cons_args.insert(cons_it, field_def->name);
                            cons_it++;
                        }
                    }
                    lua_setfield(L, -2, field_def->name);
                    if (field_def->kind == LT_FIELD_KIND_ENUM) {
                        enum_fields.push_back(i);
                    }
                } else {
                    // Field already set (i.e. overridden by a descendent class), so ignore it.
                    lua_pop(L, 1);
                }
            }
        }
        t = type_parent[t];
    }

    // Add field names indexed by constructor argument position,
    // used for arguments passed to default constructor.
    int pos = 1;
    for (cons_it = cons_args.begin(); cons_it != cons_args.end(); cons_it++) {
        lua_pushstring(L, *cons_it);
        lua_rawseti(L, -2, pos);
        pos++;
    }

    // Add enum value tables for enum fields, indexed by the field info pointer.
    std::list<int>::iterator enum_field_it;
    for (enum_field_it = enum_fields.begin(); enum_field_it != enum_fields.end(); enum_field_it++) {
        LTFieldInfo *field_info = &field_info_registry[*enum_field_it];
        const LTFieldDef *field_def = (*field_def_registry)[*enum_field_it];
        lua_pushlightuserdata(L, (void*)field_info);
        lua_newtable(L);
        const LTEnumConstant *enum_const = field_def->enum_vals;
        while (enum_const->name != NULL) {
            lua_pushstring(L, enum_const->name);
            lua_rawseti(L, -2, enum_const->val + 1);
            lua_pushinteger(L, enum_const->val);
            lua_setfield(L, -2, enum_const->name);
            enum_const++;
        }
        lua_rawset(L, -3);
    }
}

// LTTypeDef should be at upvalue 1.
// metatable should be at upvalue 2.
static int constructor_func_default(lua_State *L) {
    int nargs = lua_gettop(L);
    const LTTypeDef *type = (const LTTypeDef*)lua_touserdata(L, lua_upvalueindex(1));
    void* ud = lua_newuserdata(L, type->size);
    memset(ud, 0, type->size);
    lua_pushvalue(L, lua_upvalueindex(2)); // push metatable
    lua_setmetatable(L, -2);
    lua_newtable(L); // env table
    lua_setfenv(L, -2);
    type->default_constructor(ud);
    LTObject *obj = (LTObject*)ud;
    //lua_getstack(L, 0, &(obj->debug));
    int i = 1;
    while (i <= nargs && !lua_istable(L, i)) {
        lua_rawgeti(L, lua_upvalueindex(2), i); // get field name for arg from metatable
        if (lua_isnil(L, -1)) {
            return luaL_error(L, "Too many arguments to function '%s'", type->lua_name);
        }
        lua_pushvalue(L, -1); // copy the field name
        lua_rawget(L, lua_upvalueindex(2)); // get field info from metatable
        LTFieldInfo *field = (LTFieldInfo*)lua_touserdata(L, -1);
        lua_pop(L, 1); // pop field def
        // -1 = field name, -2 = object
        set_field_val(L, obj, field, -2, -1, i);
        lua_pop(L, 1); // pop field name;
        i++;
    }
    // If last argument is a table then initialize fields from it.
    // (Last argument must be a table if i == nargs given condition
    // of previous while loop.)
    if (i == nargs) {
        lua_pushnil(L);
        while (lua_next(L, nargs) != 0) {
            if (lua_type(L, -2) == LUA_TSTRING) {
                lua_pushvalue(L, -2); // push field name
                lua_rawget(L, lua_upvalueindex(2)); // lookup field in metatable
                LTFieldInfo *field = (LTFieldInfo*)lua_touserdata(L, -1);
                lua_pop(L, 1); // pop field info
                if (field != NULL) {
                    set_field_val(L, obj, field, -3, -2, -1);
                }
            }
            lua_pop(L, 1); // pop value, leaving key for next call to lua_next
        }
        i++;
    }
    if (i <= nargs) {
        return luaL_error(L, "Invalid argument %d", i);
    }
    obj->init(L);
    return 1;
}

static void init_metatables(lua_State *L) {
    // The metatables are stored in the registry and
    // indexed by the corresponding LTTypeDef as a light user data.
    for (unsigned int type_id = 0; type_id < type_registry->size(); type_id++) {
        const LTTypeDef *type = (*type_registry)[type_id];
        lua_pushlightuserdata(L, (void*)type);
        push_metatable(L, type_id);
        lua_rawset(L, LUA_REGISTRYINDEX);
    }
}

static void init_constructors(lua_State *L) {
    char buf[1024];
    for (unsigned int type_id = 0; type_id < type_registry->size(); type_id++) {
        const LTTypeDef *type = (*type_registry)[type_id];
        strcpy(buf, type->lua_name);
        lua_pushvalue(L, LUA_GLOBALSINDEX);
        char *begin = buf;
        char *end = strchr(begin, '.');
        int num_modules = 0;
        // push module tables
        while (end != NULL) {
            *end = '\0';
            lua_getfield(L, -1, begin);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1); // pop nil
                lua_newtable(L); // push module table
                lua_pushvalue(L, -1); // copy so it stays on the stack
                lua_setfield(L, -3, begin);
            }
            begin = end + 1;
            end = strchr(begin, '.');
            num_modules++;
        }
        // begin is now the unqualified type name
        lua_pushlightuserdata(L, (void*)type);
        lua_pushvalue(L, -1); // copy type def light ud for looking up the metatable
        lua_rawget(L, LUA_REGISTRYINDEX);
        lua_getfield(L, -1, "new");
        if (lua_isfunction(L, -1)) { // Is there a "new" method?
            // yes, use it as the constructor.
            lua_setfield(L, -4, begin);
            lua_pop(L, 2); // pop type userdata and metatable
        } else {
            lua_pop(L, 1); // pop "new" field value
            // type def at upvalue 1, metatable at upvalue 2
            lua_pushcclosure(L, constructor_func_default, 2);
            lua_setfield(L, -2, begin);
        }
        lua_pop(L, num_modules); // pop modules
        // Put metatable somewhere it can be easily modified from lua code.
        lua_getfield(L, -1, "lt_metatables");
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1); // pop nil
            lua_newtable(L); // new lt_metatables table
            lua_pushvalue(L, -1); // copy so it's still on the stack after setfield below
            lua_setfield(L, -3, "lt_metatables");
            // Now lt_metatables is on top of the stack
        }
        lua_pushlightuserdata(L, (void*)type);
        lua_rawget(L, LUA_REGISTRYINDEX);
        lua_setfield(L, -2, type->lua_name);
        lua_pop(L, 2); // pop globals and lt_metatables
    }
}

ct_assert(sizeof(long int) == sizeof(void*));

int ltLuaAddRef(lua_State *L, int obj, int val) {
    obj = absidx(L, obj);
    val = absidx(L, val);
    lua_getfenv(L, obj);
    lua_pushvalue(L, val);
    int ref = luaL_ref(L, -2);
    lua_pop(L, 1); // pop env table
    return ref;
}

void ltLuaDelRef(lua_State *L, int obj, int ref) {
    lua_getfenv(L, obj);
    luaL_unref(L, -1, ref);
    lua_pop(L, 1); // pop env table
}

void ltLuaGetRef(lua_State *L, int obj, int ref) {
    lua_getfenv(L, obj);
    lua_rawgeti(L, -1, ref);
    lua_remove(L, -2); // remove env table, leaving referenced value.
}

int ltLuaCheckNArgs(lua_State *L, int exp_args) {
    int n = lua_gettop(L);
    if (n < exp_args) {
        return luaL_error(L, "Expecting at least %d args, got %d.", exp_args, n);
    } else {
        return n;
    }
}

void* ltLuaAllocUserData(lua_State *L, LTTypeDef *type) {
    void* ud = lua_newuserdata(L, type->size);
    memset(ud, 0, type->size);
    lua_pushlightuserdata(L, (void*)type);
    lua_rawget(L, LUA_REGISTRYINDEX); // lookup metatable
    lua_setmetatable(L, -2);
    lua_newtable(L); // env table
    lua_setfenv(L, -2);
    return ud;
}

void ltLuaGetFloatGetterAndSetter(lua_State *L, int obj_index, int field_index, LTFloatGetter *getter, LTFloatSetter *setter) {
    obj_index = absidx(L, obj_index);
    field_index = absidx(L, field_index);
    lua_getmetatable(L, obj_index);
    lua_pushvalue(L, field_index);
    lua_rawget(L, -2);
    LTFieldInfo *field = (LTFieldInfo*)lua_touserdata(L, -1);
    lua_pop(L, 2); // pop field info and metatable.
    if (field->kind == LT_FIELD_KIND_FLOAT) {
        *getter = (LTFloatGetter)field->getter;
        *setter = (LTFloatSetter)field->setter;
    } else {
        *getter = NULL;
        *setter = NULL;
    }
}

void ltLuaFindFieldOwner(lua_State *L, int obj_index, int field_index) {
    obj_index = absidx(L, obj_index);
    field_index = absidx(L, field_index);
    if (!lt_is_LTSceneNode(L, obj_index)) {
        lua_pushnil(L);
        return;
    }
    lua_pushvalue(L, obj_index);
    while (true) {
        lua_getmetatable(L, -1);
        lua_pushvalue(L, field_index);
        lua_rawget(L, -2);
        if (lua_isuserdata(L, -1)) {
            lua_pop(L, 2); // pop userdata, metatable (obj now on top of stack).
            return;
        }
        lua_pop(L, 2); // pop field val, metatable.
        if (lt_is_LTWrapNode(L, -1)) {
            lua_getfenv(L, -1); // push env table
            lua_pushstring(L, "child");
            lua_rawget(L, -2); // push child
            lua_remove(L, -2); // remove env table
            lua_remove(L, -2); // remove previous obj
        } else {
            // Not a wrap node, give up search.
            lua_pop(L, 1); // pop obj.
            lua_pushnil(L);
            return;
        }
    }
}
