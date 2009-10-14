#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include "p2bot.h"

static void download_thread_images(FILE *threadp);

int main(int argc, char *argv[])
{
    FILE *fp;

    if (argc < 2) {
        fprintf(stderr, "usage: p2bot $thread_file\n");
        exit(1);
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "file open error %s\n", "test/thread.txt");
        exit(1);
    }

    download_thread_images(fp);

    return 0;
}

static void download_thread_images(FILE *threadp)
{
    int out;
    char outfile[20], buf[2048], *bufp;
    url_t *url;

    out = 0;
    while (fgets(buf, sizeof(buf), threadp) != NULL) {
        bufp = buf;
        while ((url = get_url(&bufp)) != NULL) {
            sprintf(outfile, "test/%d.jpg", out);
            fprintf(stdout, "Download: %s%s\n", url->host, url->path);
            save_url(url, outfile);
            out++;
            usleep(200000);
        }
    }
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
            fprintf(stderr, "bad image: %s\n", cs);
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


char *save_url(url_t *url, char *savefile) {
    int sock;
    char *p;
    FILE *fin, *fout;
    char buf[1024];

    sock = open_connection(url->host, "80");
    if (sock < 0) {
        fprintf(stderr, "open_connection failed\n");
        return NULL;
    }
    fin = fdopen(sock, "r");
    fout = fdopen(sock, "w");

    fprintf(fout, "GET %s HTTP/1.1\r\n", url->path);
    fprintf(fout, "HOST: %s\r\n", url->host);
    fprintf(fout, "User-Agent: %s\r\n", USER_AGENT);
    fprintf(fout, "Connection: close\r\n");
    fprintf(fout, "\r\n");
    fflush(fout);

    int length = 0;
    while (1) {
        if (fgets(buf, sizeof(buf), fin) == NULL) break;
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
        a = fgetc(fin);
        fputc(a, out);
    }

    fclose(fin);
    fclose(fout);
    fclose(out);

    return NULL;
}

int open_connection(char *host, char *service)
{
    int sock;
    struct addrinfo hints, *res, *ai;
    int err;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((err = getaddrinfo(host, service, &hints, &res)) != 0) {
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

void *xmalloc(size_t size)
{
    void *p;
    p = malloc(size);
    if (!p) {
        fprintf(stderr, "failed malloc()\n");
        exit(1);
    }

    return p;
}

