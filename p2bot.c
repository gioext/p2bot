#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include "util.h"
#include "gstack.h"

#define CONFIG_FILE "config.lua"
#define CONFIG_BOARDS_KEY "boards"

typedef struct {
    int dat_id;
    char *thread_name;
    url_t *url;
} thread_t;

static void get_board();
static void get_thread(url_t *url);
url_t *get_url(char **body);
void write_file(FILE *fp, char *savefile);
static void download_thread_images(url_t *thread_url);

gstack_t *boards;
gstack_t *threads;
gstack_t *images;

int main(int argc, char *argv[])
{
    void *datap;
    boards = gstack_new();
    threads = gstack_new();
    images = gstack_new();

    get_board();

    while ((datap = gstack_pop(boards)) != NULL) {
        get_thread((url_t *)datap);
        free_url(datap);
    }
    gstack_destroy(boards);

    while ((datap = gstack_pop(threads)) != NULL) {
        url_t *url = (url_t *)datap;
        printf("%s%s\n", url->host, url->path);
        free_url(url);
    }
    gstack_destroy(threads);

    return 0;
}

static void get_board()
{
    int i = 1;
    lua_State *L;

    L = luaL_newstate();
    luaL_openlibs(L);
    if (luaL_dofile(L, CONFIG_FILE)) {
        fprintf(stderr, "luaL_dofile()\n");
        exit(1);
    }
    lua_getglobal(L, CONFIG_BOARDS_KEY);
    while (1) {
        lua_rawgeti(L, 1, i++);
        const char *strurl = lua_tostring(L, -1);
        if (strurl == NULL) {
            break;
        } else {
            gstack_push(boards, (void *)strtourl(strurl));
        }
    }
    lua_close(L);
}

static void get_thread(url_t *url)
{
    response_t *res;
    char buf[1024];
    char subdir[strlen(url->path)];
    char thread_url_str[128];
    strcpy(subdir, url->path);
    char *subdire = strchr(subdir + 1, '/');
    *subdire = '\0';

    res = get_http_response(url);
    if (res == NULL) {
        fprintf(stderr, "board#get_http_respnse()\n");
        gstack_destroy(threads);
        return;
    }
    while (fgets(buf, sizeof(buf), res->fp) != NULL) {
        char *cp;
        cp = strstr(buf, "<>");
        if (cp == NULL) {
            printf("%s\n", cp);
            continue;
        }
        *cp = '\0';
        cp += 2; // thread name

        sprintf(thread_url_str, "%s%s/dat/%s", url->host, subdir, buf);
        gstack_push(threads, (void *)strtourl(thread_url_str));
    }
    free_response(res);
//
//    while ((datap = gstack_pop(threads)) != NULL) {
//        //download_thread_images((url_t *)datap);
//        free_url(datap);
//    }
}

/*
 * todo user dat_t
 */
static void download_thread_images(url_t *thread_url)
{
    int out;
    char outfile[20], buf[2048], *bufp;
    url_t *url;
    response_t *res, *res_image;

    res = get_http_response(thread_url);
    if (res == NULL) {
        fprintf(stderr, "thread#get_http_response()\n");
        return;
    }
    out = 0;
    while (fgets(buf, sizeof(buf), res->fp) != NULL) {
        bufp = buf;
        while ((url = get_url(&bufp)) != NULL) {
            sprintf(outfile, "test/%d.jpg", out);

            res_image = get_http_response(url);
            if (res_image == NULL) {
                fprintf(stderr, "image#get_http_response(). %s%s\n", url->host, url->path);
                free_url(url);
                continue;
            }
            if (res_image->status >= 200 && res_image->status < 300) {
                fprintf(stdout, "Image: %s%s\n", url->host, url->path);
                //write_file(ifp, outfile);
                out++;
                usleep(500000);
            } else {
                fprintf(stderr, "Bad status %s%s\n", url->host, url->path);
            }
            free_url(url);
            free_response(res_image);
        }
    }
    free_response(res);
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

void write_file(FILE *fp, char *savefile) {
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
