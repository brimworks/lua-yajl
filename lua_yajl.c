#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <lua.h>
#include <lauxlib.h>
#include <math.h>

#define js_check_generator(L, narg) \
    (yajl_gen*)luaL_checkudata((L), (narg), "yajl.generator.meta")

static int js_generator(lua_State *L);
static int js_generator_value(lua_State *L);

//////////////////////////////////////////////////////////////////////
static int js_to_string(lua_State *L) {
    lua_pushcfunction(L, js_generator);
    // convert_me, {extra}, ?, js_gen
    if ( lua_istable(L, 2) ) {
        // Be sure printer is not defined:
        lua_pushliteral(L, "printer");
        lua_pushnil(L);
        lua_rawset(L, 2);
        lua_pushvalue(L, 2);
        // convert_me, {extra}, ?, js_gen, {extra}
    } else {
        lua_newtable(L);
        // convert_me, {extra}, ?, js_gen, {}
    }
    lua_call(L, 1, 1);
    // convert_me, {extra}, ?, gen_ud
    lua_pushcfunction(L, js_generator_value);
    // convert_me, {extra}, ?, gen_ud, js_gen_val
    lua_pushvalue(L, -2);
    // convert_me, {extra}, ?, gen_ud, js_gen_val, gen_ud
    lua_pushvalue(L, 1);
    // convert_me, {extra}, ?, gen_ud, js_gen_val, gen_ud, convert_me
    lua_call(L, 2, 0);
    // convert_me, {extra}, ?, gen_ud
    yajl_gen* gen = js_check_generator(L, -1);
    const unsigned char *buf;
    unsigned int len;
    yajl_gen_get_buf(*gen, &buf, &len);
    // Copy into results:
    lua_pushlstring(L, (char*)buf, len);
    yajl_gen_clear(*gen);
    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_to_value(lua_State *L) {
    // TODO: implement me
/*
    function to_value(string)
        local result
        local stack = {
            function(val) result = val end
        }
        local obj_key
        local events = {
            value: function(_, val)
                stack[#stack](val)
            end,
            open_array: function()
                local arr = {}
                stack[#stack](arr)
                table.insert(stack, function(val)
                    table.insert(result, val)
                end)
            end,
            open_object: function()
                local obj = {}
                stack[#stack](obj)
                table.insert(stack, function(val)
                    obj[obj_key] = val
                end)
            end,
            object_key: function(_, val)
                obj_key = val
            end,
            close: function()
                stack[#stack] = nil
            end,
        }

        yajl.parser({ events: events })(string)
        return result
    end
*/
    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_null(void *ctx) {
    lua_State *L=(lua_State*)ctx;
    lua_getfield(L, lua_upvalueindex(2), "value");
    if ( ! lua_isnil(L, -1) ) {
        lua_pushvalue(L, lua_upvalueindex(2));
        lua_pushnil(L);
        lua_pushliteral(L, "null");
        lua_call(L, 3, 0);
    } else {
        lua_pop(L, 1);
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_boolean(void *ctx, int val) {
    lua_State *L=(lua_State*)ctx;

    lua_getfield(L, lua_upvalueindex(2), "value");
    if ( ! lua_isnil(L, -1) ) {
        lua_pushvalue(L, lua_upvalueindex(2));
        lua_pushboolean(L, val);
        lua_pushliteral(L, "boolean");
        lua_call(L, 3, 0);
    } else {
        lua_pop(L, 1);
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_integer(void *ctx, long val) {
    lua_State *L=(lua_State*)ctx;

    lua_getfield(L, lua_upvalueindex(2), "value");
    if ( ! lua_isnil(L, -1) ) {
        lua_pushvalue(L, lua_upvalueindex(2));
        lua_pushinteger(L, val);
        lua_pushliteral(L, "integer");
        lua_call(L, 3, 0);
    } else {
        lua_pop(L, 1);
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_double(void *ctx, double val) {
    lua_State *L=(lua_State*)ctx;

    lua_getfield(L, lua_upvalueindex(2), "value");
    if ( ! lua_isnil(L, -1) ) {
        lua_pushvalue(L, lua_upvalueindex(2));
        lua_pushnumber(L, val);
        lua_pushliteral(L, "double");
        lua_call(L, 3, 0);
    } else {
        lua_pop(L, 1);
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_string(void *ctx, const unsigned char *val, unsigned int len) {
    lua_State *L=(lua_State*)ctx;

    lua_getfield(L, lua_upvalueindex(2), "value");
    if ( ! lua_isnil(L, -1) ) {
        lua_pushvalue(L, lua_upvalueindex(2));
        lua_pushlstring(L, (const char*)val, len);
        lua_pushliteral(L, "string");
        lua_call(L, 3, 0);
    } else {
        lua_pop(L, 1);
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_start_map(void *ctx) {
    lua_State *L=(lua_State*)ctx;

    lua_getfield(L, lua_upvalueindex(2), "open_object");
    if ( ! lua_isnil(L, -1) ) {
        lua_pushvalue(L, lua_upvalueindex(2));
        lua_call(L, 1, 0);
    } else {
        lua_pop(L, 1);
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_map_key(void *ctx, const unsigned char *val, unsigned int len) {
    lua_State *L=(lua_State*)ctx;

    // TODO: Do we want to fall-back to calling "value"?
    lua_getfield(L, lua_upvalueindex(2), "object_key");
    if ( ! lua_isnil(L, -1) ) {
        lua_pushvalue(L, lua_upvalueindex(2));
        lua_pushlstring(L, (const char*)val, len);
        lua_call(L, 2, 0);
    } else {
        lua_pop(L, 1);
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_end_map(void *ctx) {
    lua_State *L=(lua_State*)ctx;

    lua_getfield(L, lua_upvalueindex(2), "close");
    if ( ! lua_isnil(L, -1) ) {
        lua_pushvalue(L, lua_upvalueindex(2));
        lua_pushliteral(L, "object");
        lua_call(L, 2, 0);
    } else {
        lua_pop(L, 1);
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_start_array(void *ctx) {
    lua_State *L=(lua_State*)ctx;

    lua_getfield(L, lua_upvalueindex(2), "open_array");
    if ( ! lua_isnil(L, -1) ) {
        lua_pushvalue(L, lua_upvalueindex(2));
        lua_call(L, 1, 0);
    } else {
        lua_pop(L, 1);
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_end_array(void *ctx) {
    lua_State *L=(lua_State*)ctx;

    lua_getfield(L, lua_upvalueindex(2), "close");
    if ( ! lua_isnil(L, -1) ) {
        lua_pushvalue(L, lua_upvalueindex(2));
        lua_pushliteral(L, "array");
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
static void js_parser_assert(lua_State* L,
                             yajl_status status,
                             yajl_handle* handle,
                             const unsigned char* json_text,
                             unsigned int json_text_len,
                             int expect_complete,
                             const char* file,
                             int line)
{
    int verbose = 1;
    unsigned char* msg;

    switch ( status ) {
    case yajl_status_ok:
        return;
    case yajl_status_client_canceled:
        lua_pushfstring(L, "Unreachable: yajl_status_client_canceled should never be returned since all callbacks return true at %s line %d",
                        file, line);
        break;
    case yajl_status_insufficient_data:
        if ( ! expect_complete ) return;
        lua_pushfstring(L, "IncompleteInput: js_parser_parse called with nil input, but the json input was not complete at %s line %d",
                        file, line);
        break;
    case yajl_status_error:
        msg = yajl_get_error(*handle, verbose, json_text, json_text_len);
        lua_pushfstring(L, "InvalidJSONInput: %s at %s line %d", msg, file, line);
        yajl_free_error(*handle, msg);
        break;
    }
    lua_error(L);
}

//////////////////////////////////////////////////////////////////////
static int js_parser_parse(lua_State *L) {
    yajl_handle* handle = (yajl_handle*)
        lua_touserdata(L, lua_upvalueindex(1));
    if ( lua_isnil(L, 1) ) {
        int expect_complete = 1;
        js_parser_assert(L,
                         yajl_parse_complete(*handle),
                         handle,
                         NULL,
                         0,
                         expect_complete,
                         __FILE__,
                         __LINE__);
    } else {
        int expect_complete = 0;
        size_t len;
        const unsigned char* buff = (const unsigned char*) luaL_checklstring(L, 1, &len);
        if ( NULL == buff ) return 0;
        js_parser_assert(L,
                         yajl_parse(*handle, buff, len),
                         handle,
                         buff,
                         len,
                         expect_complete,
                         __FILE__,
                         __LINE__);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_parser_delete(lua_State *L) {
    luaL_checktype(L, 1, LUA_TUSERDATA);
    yajl_free(*(yajl_handle*)lua_touserdata(L, 1));
    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_parser(lua_State *L) {
    yajl_parser_config cfg = { 1, 1 };

    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "allow_comments");
    if ( ! lua_isnil(L, -1) ) {
        cfg.allowComments = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "check_utf8");
    if ( ! lua_isnil(L, -1) ) {
        cfg.checkUTF8 = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    yajl_handle* handle = (yajl_handle*)lua_newuserdata(L, sizeof(yajl_handle));

    *handle = yajl_alloc(&js_parser_callbacks, &cfg, NULL, (void*)L);
    luaL_getmetatable(L, "yajl.parser.meta");
    lua_setmetatable(L, -2);

    lua_getfield(L, 1, "events");

    // Create callback function that calls yajl_parse[_complete]()
    //
    // upvalue(1) = yajl_handle*
    // upvalue(2) = events table
    lua_pushcclosure(L, &js_parser_parse, 2);

    return 1;
}

//////////////////////////////////////////////////////////////////////
static int js_generator_delete(lua_State *L) {
    yajl_gen* handle = js_check_generator(L, 1);
    yajl_gen_free(*handle);
    return 0;
}

//////////////////////////////////////////////////////////////////////
static void js_generator_assert(lua_State *L,
                                yajl_gen_status status,
                                const char* file,
                                int line)
{
    switch ( status ) {
    case yajl_gen_status_ok:
    case yajl_gen_generation_complete:
        return;
    case yajl_gen_keys_must_be_strings:
        lua_pushfstring(L, "InvalidState: expected either a call to close() or string() since we are in the middle of an object declaration at %s line %d", file, line);
        break;
    case yajl_max_depth_exceeded:
        lua_pushfstring(L, "StackOverflow: YAJL's max generation depth was exceeded at %s line %d", file, line);
        break;
    case yajl_gen_in_error_state:
        lua_pushfstring(L, "AlreadyInError: generator method was called when the generator is already in an error state at %s line %d", file, line);
        break;
    default:
        lua_pushfstring(L, "Unreachable: yajl_gen_status (%d) not recognized at %s line %d", status, file, line);
        break;
    }
    lua_error(L);
}

//////////////////////////////////////////////////////////////////////
static int js_generator_integer(lua_State *L) {
    js_generator_assert(L,
                        yajl_gen_integer(*js_check_generator(L, 1),
                                         luaL_checkinteger(L, 2)),
                        __FILE__, __LINE__);
    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_generator_double(lua_State *L) {
    js_generator_assert(L,
                        yajl_gen_double(*js_check_generator(L, 1),
                                        luaL_checknumber(L, 2)),
                        __FILE__, __LINE__);
    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_generator_number(lua_State *L) {
    size_t len;
    const char* str = luaL_checklstring(L, 2, &len);
    js_generator_assert(L,
                        yajl_gen_number(*js_check_generator(L, 1),
                                        str, len),
                        __FILE__, __LINE__);
    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_generator_string(lua_State *L) {
    size_t len;
    const unsigned char* str = (const unsigned char*)luaL_checklstring(L, 2, &len);
    js_generator_assert(L,
                        yajl_gen_string(*js_check_generator(L, 1),
                                        str, len),
                        __FILE__, __LINE__);
    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_generator_null(lua_State *L) {
    js_generator_assert(L,
                        yajl_gen_null(*js_check_generator(L, 1)),
                        __FILE__, __LINE__);
    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_generator_boolean(lua_State *L) {
    luaL_checktype(L, 2, LUA_TBOOLEAN);
    js_generator_assert(L,
                        yajl_gen_bool(*js_check_generator(L, 1),
                                      lua_toboolean(L, 2)),
                        __FILE__, __LINE__);
    return 0;
}

#define JS_OPEN_OBJECT 1
#define JS_OPEN_ARRAY  2

//////////////////////////////////////////////////////////////////////
static int js_generator_open_object(lua_State *L) {
    js_generator_assert(L,
                        yajl_gen_map_open(*js_check_generator(L, 1)),
                        __FILE__, __LINE__);
    // Why doesn't yajl_gen keep track of this!?
    lua_getfenv(L, 1);
    lua_getfield(L, -1, "stack");
    lua_pushinteger(L, JS_OPEN_OBJECT);
    lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_generator_open_array(lua_State *L) {
    js_generator_assert(L,
                        yajl_gen_array_open(*js_check_generator(L, 1)),
                        __FILE__, __LINE__);
    // Why doesn't yajl_gen keep track of this!?
    lua_getfenv(L, 1);
    lua_getfield(L, -1, "stack");
    lua_pushinteger(L, JS_OPEN_ARRAY);
    lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_generator_close(lua_State *L) {
    // Why doesn't yajl_gen keep track of this!?
    lua_getfenv(L, 1);
    lua_getfield(L, -1, "stack");
    lua_rawgeti(L, -1, lua_objlen(L, -1));
    if ( lua_isnil(L, -1) ) {
        lua_pushfstring(L, "StackUnderflow: Attempt to call close() when no array or object has been opened at %s line %d", __FILE__, __LINE__);
        lua_error(L);
    }
    lua_Integer type = lua_tointeger(L, -1);
    switch ( type ) {
    case JS_OPEN_OBJECT:
        js_generator_assert(L,
                            yajl_gen_map_close(*js_check_generator(L, 1)),
                            __FILE__, __LINE__);
        break;
    case JS_OPEN_ARRAY:
        js_generator_assert(L,
                            yajl_gen_array_close(*js_check_generator(L, 1)),
                            __FILE__, __LINE__);
        break;
    default:
        lua_pushfstring(L, "Unreachable: internal 'stack' contained invalid integer (%d) at %s line %d", type, __FILE__, __LINE__);
        lua_error(L);
    }
    // delete the top of the "stack":
    lua_pop(L, 1);
    lua_pushnil(L);
    lua_rawseti(L, -2, lua_objlen(L, -2));

    return 0;
}

//////////////////////////////////////////////////////////////////////
static int js_generator_value(lua_State *L) {
    int type = lua_type(L, 2);

    switch ( type ) {
    case LUA_TNIL:
        return js_generator_null(L);
    case LUA_TNUMBER:
        return js_generator_number(L);
    case LUA_TBOOLEAN:
        return js_generator_boolean(L);
    case LUA_TSTRING:
        return js_generator_string(L);
    case LUA_TTABLE:
    case LUA_TFUNCTION:
    case LUA_TUSERDATA:
    case LUA_TTHREAD:
    case LUA_TLIGHTUSERDATA:
        if ( luaL_getmetafield(L, 2, "__gen_json") ) {
            if  ( lua_isfunction(L, -1) ) {
                lua_settop(L, 3); // gen, obj, func
                lua_insert(L, 1); // func, gen, obj
                lua_insert(L, 2); // func, obj, gen
                lua_call(L, 2, 0);
                return 0;
            }
            lua_pop(L, 1);
        }
        // Simply ignore it, perhaps we should warn?
        if ( type != LUA_TTABLE ) return 0;

        int max      = 0;
        int is_array = 1;

        // First iterate over the table to see if it is an array:
        lua_pushnil(L);
        while ( lua_next(L, 2) != 0 ) {
            if ( lua_isnumber(L, -2) ) {
                double num = lua_tonumber(L, -2);
                if ( num == floor(num) ) {
                    if ( num > max ) max = num;
                } else {
                    lua_pop(L, 2);
                    is_array = 0;
                    break;
                }
            } else {
                lua_pop(L, 2);
                is_array = 0;
                break;
            }
            lua_pop(L, 1);
        }

        if ( is_array ) {
            js_generator_open_array(L);
            for ( int i=1; i <= max; i++ ) {
                lua_pushinteger(L, i);
                lua_gettable(L, 2);

                // RECURSIVE CALL:
                // gen, obj, ?, val, func, gen, val
                lua_pushcfunction(L, js_generator_value);
                lua_pushvalue(L, 1);
                lua_pushvalue(L, -3);
                lua_call(L, 2, 0);

                lua_pop(L, 1);
            }
        } else {
            js_generator_open_object(L);

            lua_pushnil(L);
            while ( lua_next(L, 2) != 0 ) {
                // gen, obj, ?, key, val, func, gen, key
                lua_pushcfunction(L, js_generator_string);
                lua_pushvalue(L, 1);
                lua_pushvalue(L, -4);
                lua_call(L, 2, 0);


                // RECURSIVE CALL:
                // gen, obj, ?, key, val, func, gen, val
                lua_pushcfunction(L, js_generator_value);
                lua_pushvalue(L, 1);
                lua_pushvalue(L, -3);
                lua_call(L, 2, 0);

                lua_pop(L, 1);
            }
        }
        js_generator_close(L);
        return 0;
    case LUA_TNONE:
        lua_pushfstring(L, "MissingArgument: second parameter to js_generator_value() must be defined at %s line %d", type, __FILE__, __LINE__);
    default:
        lua_pushfstring(L, "Unreachable: js_generator_value passed lua type (%d) not recognized at %s line %d", type, __FILE__, __LINE__);
    }
    // Shouldn't get here:
    lua_error(L);
    return 0;
}

typedef struct {
    lua_State* L;
    int        printer_ref;
} js_printer_ctx;

//////////////////////////////////////////////////////////////////////
static void js_printer(void* void_ctx, const char* str, unsigned int len) {
    js_printer_ctx* ctx = (js_printer_ctx*)void_ctx;
    lua_State* L = ctx->L;

    // refs
    lua_getfield(L, LUA_REGISTRYINDEX, "yajl.refs");
    // refs, printer
    lua_rawgeti(L, -1, ctx->printer_ref);
    if ( lua_isfunction(L, -1) ) {
        lua_pushlstring(L, str, len);
        // Not sure if yajl can handle longjmp's if this errors...
        lua_call(L, 1, 0);
        lua_pop(L, 1);
    } else {
        lua_pop(L, 2);
    }
}

//////////////////////////////////////////////////////////////////////
static int js_generator(lua_State *L) {
    yajl_gen_config cfg = { 0, NULL };
    yajl_print_t   print = NULL;
    void *          ctx  = NULL;

    luaL_checktype(L, 1, LUA_TTABLE);

    // {args}, ?, tbl
    lua_newtable(L);

    // Validate and save in fenv so it isn't gc'ed:
    lua_getfield(L, 1, "printer");
    if ( ! lua_isnil(L, -1) ) {
        luaL_checktype(L, -1, LUA_TFUNCTION);

        lua_pushvalue(L, -1);

        // {args}, ?, tbl, printer, printer
        lua_setfield(L, -2, "printer");

        js_printer_ctx* print_ctx = (js_printer_ctx*)
            lua_newuserdata(L, sizeof(js_printer_ctx));
        // {args}, ?, tbl, printer, printer_ctx

        lua_setfield(L, -3, "printer_ctx");
        // {args}, ?, tbl, printer

        lua_getfield(L, LUA_REGISTRYINDEX, "yajl.refs");
        // {args}, ?, tbl, printer, refs
        lua_insert(L, -2);
        // {args}, ?, tbl, refs, printer
        print_ctx->printer_ref = luaL_ref(L, -2);
        print_ctx->L = L;
        print = &js_printer;
        ctx   = print_ctx;
    }
    lua_pop(L, 1);
    // {args}, ?, tbl

    // Get the indent and save so it isn't gc'ed:
    lua_getfield(L, 1, "indent");
    if ( ! lua_isnil(L, -1) ) {
        cfg.beautify = 1;
        cfg.indentString = lua_tostring(L, -1);
        lua_setfield(L, -2, "indent");
    } else {
        lua_pop(L, 1);
    }
    // {args}, ?, tbl

    // Sucks that yajl's generator doesn't keep track of this for me
    // (this is a stack of strings "array" and "object" so I can keep
    // track of what to "close"):
    lua_newtable(L);
    lua_setfield(L, -2, "stack");

    // {args}, ?, tbl
    yajl_gen* handle = (yajl_gen*)lua_newuserdata(L, sizeof(yajl_gen));
    *handle = yajl_gen_alloc2(print, &cfg, NULL, ctx);

    // {args}, ?, tbl, ud, meta
    luaL_getmetatable(L, "yajl.generator.meta");
    lua_setmetatable(L, -2);
    // {args}, ?, tbl, ud

    lua_insert(L, -2);
    // {args}, ?, ud, tbl
    lua_setfenv(L, -2);

    return 1;
}

//////////////////////////////////////////////////////////////////////
static void js_create_parser_mt(lua_State *L) {
    luaL_newmetatable(L, "yajl.parser.meta"); // {}

    lua_pushcfunction(L, js_parser_delete);
    lua_setfield(L, -2, "__gc");

    lua_pop(L, 1); // <empty>
}
//////////////////////////////////////////////////////////////////////
static void js_create_generator_mt(lua_State *L) {
    luaL_newmetatable(L, "yajl.generator.meta"); // {}

    lua_pushcfunction(L, js_generator_delete);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, js_generator_value);
    lua_setfield(L, -2, "value");

    lua_pushcfunction(L, js_generator_integer);
    lua_setfield(L, -2, "integer");

    lua_pushcfunction(L, js_generator_double);
    lua_setfield(L, -2, "double");

    lua_pushcfunction(L, js_generator_number);
    lua_setfield(L, -2, "number");

    lua_pushcfunction(L, js_generator_string);
    lua_setfield(L, -2, "string");

    lua_pushcfunction(L, js_generator_null);
    lua_setfield(L, -2, "null");

    lua_pushcfunction(L, js_generator_boolean);
    lua_setfield(L, -2, "boolean");

    lua_pushcfunction(L, js_generator_open_object);
    lua_setfield(L, -2, "open_object");

    lua_pushcfunction(L, js_generator_open_array);
    lua_setfield(L, -2, "open_array");

    lua_pushcfunction(L, js_generator_close);
    lua_setfield(L, -2, "close");

    lua_pop(L, 1); // <empty>
}

//////////////////////////////////////////////////////////////////////
LUALIB_API int luaopen_yajl(lua_State *L) {
    js_create_parser_mt(L);
    js_create_generator_mt(L);

    // Create the yajl.refs weak table:
    lua_createtable(L, 0, 2);
    lua_pushliteral(L, "v"); // tbl, "v"
    lua_setfield(L, -2, "__mode");
    lua_pushvalue(L, -1);    // tbl, tbl
    lua_setmetatable(L, -2); // tbl
    lua_setfield(L, LUA_REGISTRYINDEX, "yajl.refs");

    lua_createtable(L, 0, 4);

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
