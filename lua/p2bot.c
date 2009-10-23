#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int lusleep(lua_State *L);

int lusleep(lua_State *L)
{
    int microseconds;
    microseconds = lua_tonumber(L, 1);
    usleep(microseconds);
    return 1;
}

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch(opt) {
        case 'd':
            if (daemon(1, 0)) {
                fprintf(stderr, "p2bot: daemon() failed\n");
                return 1;
            }
            break;
        }
    }

    lua_State *L;
    L = lua_open();
    luaL_openlibs(L);

    lua_register(L, "usleep", lusleep);

    if (luaL_dofile(L, "p2bot.lua")) {
        fprintf(stderr, "p2bot.lua: load error\n");
        return 1;
    }

    lua_close(L);
    return 0;
}
