#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "util.h"

#define CONFIG_FILE "config.lua"
#define CONFIG_BOARDS_KEY "boards"

typedef struct {
    int dat_id;
    char *thread_name;
    url_t *url;
} thread_t;


void get_board();
void get_thread(url_t *url);
url_t *get_url(char **body);
void save_url(FILE *fp, char *savefile);
static void download_thread_images(url_t *thread_url);

int main(int argc, char *argv[])
{
    get_board();
    return 0;
}

void get_board()
{
    int i;
    lua_State *L;
    url_t *url;

    L = luaL_newstate();
    luaL_openlibs(L);
    if (luaL_dofile(L, CONFIG_FILE)) {
        fprintf(stderr, "luaL_dofile()\n");
        exit(1);
    }
    lua_getglobal(L, CONFIG_BOARDS_KEY);

    i = 1;
    while (1) {
        lua_rawgeti(L, 1, i++);
        const char *strurl = lua_tostring(L, -1);
        if (strurl == NULL) {
            break;
        } else {
            url = strtourl(strurl);
            get_thread(url);
            printf("Download board: %s%s\n", url->host, url->path);
            free_url(url);
        }
    }
    lua_close(L);
}

void get_thread(url_t *url)
{
    int sock;
    char buf[1024];
    FILE *fp;
    char subdir[strlen(url->path)];
    char thread_url_str[128];
    strcpy(subdir, url->path);
    char *subdire = strchr(subdir + 1, '/');
    *subdire = '\0';

    sock = http_get(url, &fp);
    if (sock < 0 || fp == NULL) {
        fprintf(stderr, "http_get()\n");
        return;
    }

    int body = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (body == 0 && strcmp(buf, "\r\n") == 0) {
            body = 1;
            continue;
        }
        if (body == 0) continue;

        char *cp;
        cp = strstr(buf, "<>");
        if (cp == NULL) {
            printf("%s\n", cp);
            continue;
        }
        *cp = '\0';
        cp += 2; // thread name

        sprintf(thread_url_str, "%s%s/dat/%s", url->host, subdir, buf);
        url_t *thread_url = strtourl(thread_url_str);
        download_thread_images(thread_url);
        printf("Download thread: %s%s/dat/%s\n", url->host, subdir, buf);
        free_url(thread_url);
    }
    fclose(fp);
    close(sock);
}

static void download_thread_images(url_t *thread_url)
{
    int out;
    int sock;
    char outfile[20], buf[2048], *bufp;
    url_t *url;
    FILE *fp;

    sock = http_get(thread_url, &fp);
    if (sock < 0) {
        fprintf(stderr, "http_get()\n");
    }
    out = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        bufp = buf;
        while ((url = get_url(&bufp)) != NULL) {
            sprintf(outfile, "test/%d.jpg", out);
            fprintf(stdout, "Download: %s%s\n", url->host, url->path);

            FILE *ifp;
            int is = http_get(url, &ifp);
            if (is < 0) {
                fprintf(stderr, "http_get()\n");
                free_url(url);
                continue;
            }
            //save_url(ifp, outfile);
            out++;
            free_url(url);
            fclose(ifp);
            close(is);
            //usleep(500000);
        }
    }
    fclose(fp);
    close(sock);
}

url_t *get_url(char **body)
{
    url_t *url;
    char ch, *cs, *ce, *cp;
    cs = *body;

    /* search url */
    while((ce = strstr(*body, "//")) != NULL) {
        ce += 2;
        cs = ce;

        ch = *ce;
        while (isalnum(ch) || strchr(".?/=_:-~", ch) != NULL) {
            ch = *(++ce);
        }
        *ce++ = '\0';
        *body = ce;

        // valid
        if (strchr(cs, '/') == NULL || strcasestr(cs, ".jpg") == NULL) {
            //fprintf(stderr, "bad image: %s\n", cs);
            continue;
        }

        cp = strchr(cs, '/');
        *cp++ = '\0';
        
        url = xmalloc(sizeof(url_t));
        url->host = xmalloc(strlen(cs) + 1);
        sprintf(url->host, "%s", cs);
        url->path = xmalloc(strlen(cp) + 2);
        sprintf(url->path, "/%s", cp);

        return url;
    }
    return NULL;
}

void save_url(FILE *fp, char *savefile) {
    int length = 0;
    char buf[1024];
    char *p;

    while (1) {
        if (fgets(buf, sizeof(buf), fp) == NULL) break;
        if (strcmp(buf, "\r\n") == 0) break;

        if (strncasecmp(buf, "content-length:", 15) == 0) {
            p = buf;
            p += 15;
            length = atoi(p);
        }
    }

    FILE *out;
    int a, i;
    out = fopen(savefile, "w");

    for (i = 0; i < length; i++) {
        a = fgetc(fp);
        fputc(a, out);
    }
    fclose(out);
}
