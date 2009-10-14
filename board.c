#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include "p2bot.h"

url_t *strtourl(const char *str);
void free_url(url_t *url);

int main(int argc, char *argv[])
{
    int i;
    lua_State *L;

    L = luaL_newstate();
    luaL_openlibs(L);
    luaL_dofile(L, "config.lua");
    lua_getglobal(L, "boards");

    i = 1;
    while (1) {
        lua_rawgeti(L, 1, i);
        const char *strurl = lua_tostring(L, -1);
        if (strurl == NULL) {
            break;
        } else {
            url_t *url = strtourl(strurl);

            printf("%s\n", url->host);
            printf("%s\n", url->path);

            free_url(url);
        }
        i++;
    }

    lua_close(L);
    return 0;
}

url_t *strtourl(const char *str)
{
    char *cs, *cp;
    char buf[strlen(str)];
    url_t *url;

    strcpy(buf, str);

    cs = strstr(buf, "//");
    cs += 2;

    cp = strchr(cs, '/');
    *cp++ = '\0';

    url = malloc(sizeof(url_t));
    url->host = malloc(strlen(cs) + 1);
    url->path = malloc(strlen(cp) + 2);

    sprintf(url->host, "%s", cs);
    sprintf(url->path, "/%s", cp);

    return url;
}

void free_url(url_t *url)
{
    free(url->host);
    free(url->path);
    free(url);
}
