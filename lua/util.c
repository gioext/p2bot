#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

static int lsleep(lua_State* L)
{
    int second = lua_tonumber(L, 1);
    sleep(second);
    return 1;
}

static int lusleep(lua_State* L)
{
    int microsecond = lua_tonumber(L, 1);
    usleep(microsecond);
    return 1;
}

static const struct luaL_reg utillib[] = {
    {"sleep", lsleep},
    {"usleep", lusleep},
    {NULL, NULL},
};

int luaopen_util(lua_State* L)
{
    luaL_openlib(L, "util", utillib, 0);
    return 1;
}
