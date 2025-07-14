#include "luajit_compat.h"

/* For LuaJIT readonly table compatibility, we need to track visited tables */
#define LUAJIT_READONLY_REGISTRY_KEY "__LUAJIT_READONLY_TABLES__"

/* Stub implementations for readonly table functions that don't exist in LuaJIT
 */
void lua_enablereadonlytable(lua_State *L, int index, int enabled) {
  /* LuaJIT doesn't support readonly tables, but we simulate the tracking
   * to prevent infinite recursion in luaSetTableProtectionRecursively */
  if (!enabled)
    return;

  lua_pushvalue(L, index); /* push table to top */

  /* Get or create the readonly tables registry */
  lua_pushstring(L, LUAJIT_READONLY_REGISTRY_KEY);
  lua_rawget(L, LUA_REGISTRYINDEX);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);   /* pop nil */
    lua_newtable(L); /* create new table */
    lua_pushstring(L, LUAJIT_READONLY_REGISTRY_KEY);
    lua_pushvalue(L, -2);             /* duplicate table */
    lua_rawset(L, LUA_REGISTRYINDEX); /* store in registry */
  }

  /* Mark this table as readonly by storing table -> true */
  lua_pushvalue(L, -2);  /* push table again as key */
  lua_pushboolean(L, 1); /* push true as value */
  lua_rawset(L, -3);     /* set in readonly registry */

  lua_pop(L, 2); /* pop registry and table */
}

int lua_isreadonlytable(lua_State *L, int index) {
  /* Check if table is marked as readonly in our registry */
  lua_pushvalue(L, index); /* push table to top */

  lua_pushstring(L, LUAJIT_READONLY_REGISTRY_KEY);
  lua_rawget(L, LUA_REGISTRYINDEX);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 2); /* pop nil and table */
    return 0;
  }

  lua_pushvalue(L, -2); /* push table as key */
  lua_rawget(L, -2);    /* get from readonly registry */
  int is_readonly = lua_toboolean(L, -1);

  lua_pop(L, 3); /* pop result, registry, and table */
  return is_readonly;
}
