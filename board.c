#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include "p2bot.h"

void get_board(url_t *url);
int http_get(url_t *url);
int get_http_socket(char *host);
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
        lua_rawgeti(L, 1, i++);
        const char *strurl = lua_tostring(L, -1);
        if (strurl == NULL) {
            break;
        } else {
            url_t *url = strtourl(strurl);

            printf("%s\n", url->host);
            printf("%s\n", url->path);

            get_board(url);

            free_url(url);
        }
    }

    lua_close(L);
    return 0;
}

void get_board(url_t *url)
{
    http_get(url);
}

int http_get(url_t *url)
{
    int sock, ch;
    FILE *fout;

    sock = get_http_socket(url->host);
    if (sock < 0) {
        fprintf(stderr, "get_http_socket()\n");
        return;
    }

    fout = fdopen(sock, "w");
    FILE *fin = fdopen(sock, "r");

    fprintf(fout, "GET %s HTTP/1.1\r\n", url->path);
    fprintf(fout, "HOST: %s\r\n", url->host);
    fprintf(fout, "User-Agent: %s\r\n", USER_AGENT);
    fprintf(fout, "Connection: close\r\n");
    fprintf(fout, "\r\n");
    fflush(fout);

    fin = fdopen(sock, "r");
    if (fin == NULL) {
        fprintf(stderr, "get_board()\n");
        exit(1);
    }

    int c;
    FILE *fp;

    fp = fopen("sample.txt", "w");
    while ((c = fgetc(fin)) != EOF) {
        fputc(c, fp);
    }
    fclose(fout);
    fclose(fin);
    fclose(fp);
    close(sock);
    return sock;
}

url_t *strtourl(const char *str)
{
    char *cs, *cp;
    char buf[strlen(str)];
    url_t *url;

    strcpy(buf, str);

    cs = strstr(buf, "//");
    if (cs == NULL) {
        cs = buf;
    }
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

int get_http_socket(char *host)
{
    int sock;
    struct addrinfo hints, *res, *ai;
    int err;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((err = getaddrinfo(host, "80", &hints, &res)) != 0) {
        return -1;
    }
    for (ai = res; ai; ai = ai->ai_next) {
        sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sock < 0) {
            continue;
        }
        if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
            close(sock);
            continue;
        }
        /* success */
        freeaddrinfo(res);
        return sock;
    }
    freeaddrinfo(res);
    return -2;
}
