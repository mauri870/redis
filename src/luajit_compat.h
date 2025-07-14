#ifndef LUAJIT_COMPAT_H
#define LUAJIT_COMPAT_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#ifndef LUA_JITLIBNAME
#define LUA_JITLIBNAME "jit"
#endif
#ifndef LUA_FFILIBNAME
#define LUA_FFILIBNAME "ffi"
#endif

/* LuaJIT compatibility layer for missing functions */
#ifndef lua_unref
#define lua_unref(L, ref) luaL_unref(L, LUA_REGISTRYINDEX, (ref))
#endif

/* Function declarations for LuaJIT libraries */
LUALIB_API int luaopen_jit(lua_State *L);
LUALIB_API int luaopen_ffi(lua_State *L);
LUALIB_API int luaopen_bit(lua_State *L);

/* External library declarations that need to be linked from lua/ directory */
LUALIB_API int luaopen_cjson(lua_State *L);
LUALIB_API int luaopen_struct(lua_State *L);
LUALIB_API int luaopen_cmsgpack(lua_State *L);

/* Readonly table compatibility functions */
void lua_enablereadonlytable(lua_State *L, int index, int enabled);
int lua_isreadonlytable(lua_State *L, int index);

#endif /* LUAJIT_COMPAT_H */
