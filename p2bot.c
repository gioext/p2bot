#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include "util.h"
#include "gstack.h"

typedef struct {
    int dat_id;
    int no;
    char *thread_name;
    url_t *url;
} thread_t;

static void get_boards();
static void get_threads(url_t *board_url);
static void get_images(url_t *thread_url);
static void download_image(url_t *image_url);
static url_t *get_url(char **body);
static void write_file(FILE *fp, char *savefile);

static void *run_parse_threads(void *arg);
static void *run_download_images(void *arg);

gstack_t *boards;
gstack_t *threads;
gstack_t *images;

int main(int argc, char *argv[])
{
    url_t *url;
    pthread_t th_thread[2], th_image[2];

    boards = gstack_new();
    threads = gstack_new();
    images = gstack_new();

    // start worker thread
    pthread_create(&th_thread[0], NULL, run_parse_threads, NULL);
    pthread_create(&th_thread[1], NULL, run_parse_threads, NULL);
    pthread_create(&th_image[0], NULL, run_download_images, NULL);
    pthread_create(&th_image[1], NULL, run_download_images, NULL);

    while (1) {
        get_boards();
        while (boards->length != 0) {
            url = (url_t *)gstack_pop(boards);
            get_threads(url);
            printf("%s%s\n", url->host, url->path);
            free_url(url);
        }
        while (threads->length != 0 || images->length != 0) {
            printf("%d %d\n", threads->length, images->length);
            sleep(10);
        }
        //sleep(600);
    }

    return 0;
}

static void *run_parse_threads(void *arg)
{
    void *datap;
    url_t *url;
    while ((datap = gstack_pop(threads)) != NULL) {
        url = (url_t *)datap;
        get_images(url);
        printf("%s%s\n", url->host, url->path);
        free_url(url);
        usleep(200000);
    }

    return NULL;
}

static void *run_download_images(void *arg)
{
    void *datap;
    url_t *url;
    while ((datap = gstack_pop(images)) != NULL) {
        url = (url_t *)datap;
        //download_image(url);
        printf("%s%s\n", url->host, url->path);
        free_url(url);
        usleep(200000);
    }

    return NULL;
}


static void get_boards()
{
    FILE *fp;
    char buf[256];

    fp = fopen("config.dat", "r");
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        gstack_push(boards, (void *)strtourl(buf));
    }
    fclose(fp);
}

static void get_threads(url_t *url)
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
        return;
    }
    while (fgets(buf, sizeof(buf), res->fp) != NULL) {
        char *cp;
        cp = strchr(buf, '.');
        if (cp == NULL) {
            continue;
        }
        *cp = '\0';
        cp += 6; // thread name

        sprintf(thread_url_str, "http://bg20.2ch.net/test/r.so/%s%s/%s/", url->host, subdir, buf);
        gstack_push(threads, (void *)strtourl(thread_url_str));
    }
    free_response(res);
}

static void get_images(url_t *thread)
{
    char buf[2048], *bufp;
    url_t *url;
    response_t *res;

    res = get_http_response(thread);
    if (res == NULL) {
        fprintf(stderr, "thread#get_http_response()\n");
        return;
    }
    while (fgets(buf, sizeof(buf), res->fp) != NULL) {
        bufp = buf;
        while ((url = get_url(&bufp)) != NULL) {
            gstack_push(images, url);
        }
    }
    free_response(res);
}

/*
 * todo user dat_t
 */
static void download_image(url_t *url)
{
    response_t *res;
    res = get_http_response(url);
    if (res == NULL) {
        fprintf(stderr, "image#get_http_response(). %s%s\n", url->host, url->path);
        return;
    }
    if (res->status >= 200 && res->status < 300) {
        fprintf(stdout, "Image: %s%s\n", url->host, url->path);
        //write_file(ifp, outfile);
    } else {
        fprintf(stderr, "Bad status %s%s\n", url->host, url->path);
    }
    free_response(res);
}

static url_t *get_url(char **body)
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

static void write_file(FILE *fp, char *savefile) {
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
