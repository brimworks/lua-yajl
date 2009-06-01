#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <lua.h>
#include <lauxlib.h>

//////////////////////////////////////////////////////////////////////
static int js_to_string(lua_State *L) {
    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_to_value(lua_State *L) {
    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_null(void *ctx) {
    lua_State *L=(lua_State*)ctx;
    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_boolean(void *ctx, int val) {
    lua_State *L=(lua_State*)ctx;
    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_integer(void *ctx, long val) {
    lua_State *L=(lua_State*)ctx;
    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_double(void *ctx, double val) {
    lua_State *L=(lua_State*)ctx;
    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_string(void *ctx, const unsigned char *val, unsigned int len) {
    lua_State *L=(lua_State*)ctx;
    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_start_map(void *ctx) {
    lua_State *L=(lua_State*)ctx;
    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_map_key(void *ctx, const unsigned char *val, unsigned int len) {
    lua_State *L=(lua_State*)ctx;
    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_end_map(void *ctx) {
    lua_State *L=(lua_State*)ctx;
    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_start_array(void *ctx) {
    lua_State *L=(lua_State*)ctx;
    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_end_array(void *ctx) {
    lua_State *L=(lua_State*)ctx;
    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_null(void *ctx) {
    lua_State *L=(lua_State*)ctx;

    lua_pushstring(L, "value");
    lua_gettable(L, lua_upvalueindex(2));

    if ( ! lua_isnil(L, -1) ) {
        lua_pushvalue(L, lua_upvalueindex(2));
        lua_pushnil(L);
        lua_call(L, 2, 0);
    } else {
        lua_pop(L, 1);
    }

    return 1;
}

static yajl_callbacks js_parser_callbacks = {
    js_parser_null,
    js_parser_boolean,
    js_parser_integer,
    js_parser_double,
    NULL,
    js_parser_string,
    js_parser_start_map,
    js_parser_map_key,
    js_parser_end_map,
    js_parser_start_array,
    js_parser_end_array
};

//////////////////////////////////////////////////////////////////////
static int js_parser_parse(lua_State *L) {
    yajl_handle* handle = (yajl_handle*)
        lua_touserdata(L, lua_upvalueindex(1));
    if ( lua_isnil(L, 1) ) {
        js_assert_status(yajl_parse_complete(handle));
    } else {
        size_t len;
        char * buff = luaL_checklstring(L, 1, &len);
        js_assert_status(yajl_parse(handle, buff, len));
    }
}

static int js_parser_delete(lua_State *L) {
    luaL_checktype(L, 1, LUA_TUSERDATA);
    yajl_free(lua_touserdata(L, 1));
}

//////////////////////////////////////////////////////////////////////
static int js_parser(lua_State *L) {
    yajl_parser_config cfg = { 1, 1 };

    luaL_checktype(L, 1, LUA_TTABLE);

    lua_pushstring(L, "allow_comments");
    lua_gettable(L, 1);
    if ( ! lua_isnil(L, -1) ) {
        cfg.allowComments = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    lua_pushstring(L, "check_utf8");
    lua_gettable(L, 1);
    if ( ! lua_isnil(L, -1) ) {
        cfg.checkUTF8 = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    yajl_handle* handle = (yajl_handle*)lua_newuserdata(L, sizeof(yajl_handle));

    *handle = yajl_alloc(&js_parser_callbacks, &cfg, NULL, (void*)L);
    luaL_getmetatable(L, "yajl.parser.meta");
    lua_setmetatable(L, -2);

    lua_pushstring(L, "events");
    lua_gettable(L, 1);

    // Create callback function that calls yajl_parse[_complete]()
    //
    // upvalue(1) = yajl_handle*
    // upvalue(2) = events table
    lua_pushcclosure(L, &js_parser_parse, 2);

    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_generator(lua_State *L) {
    return 0;
}

//////////////////////////////////////////////////////////////////////
static void js_create_parser_mt(lua_State *L) {
    luaL_newmetatable(L, "yajl.parser.meta"); // {}

    lua_pushcfunction(L, js_parser_delete);
    lua_setfield(L, -2, "__gc");

    lua_pop(L, 1); // <empty>
}

//////////////////////////////////////////////////////////////////////
LUALIB_API int luaopen_yajl(lua_State *L) {
    js_create_parser_mt(L);

    lua_createtable(L, 0, 8);

    lua_pushcfunction(L, js_to_string);
    lua_setfield(L, -2, "to_string");

    lua_pushcfunction(L, js_to_value);
    lua_setfield(L, -2, "to_value");

    lua_pushcfunction(L, js_parser);
    lua_setfield(L, -2, "parser");

    lua_pushcfunction(L, js_generator);
    lua_setfield(L, -2, "generator");

    return 1;
}
