#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include "util.h"

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

url_t *strtourl(const char *str)
{
    char *cs, *cp;
    char buf[strlen(str)];
    url_t *url;

    strcpy(buf, str);

    cs = strstr(buf, "//");
    if (cs == NULL) {
        cs = buf;
    } else {
        cs += 2;
    }

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

/*
 * ex1.) pl-loader.net
 * url      : http://pl-loader.net/dl.php?la=1365361110.jpg
 * referer  : http://pl-loader.net/dl.php
 * image url: http://pl-loader.net/pic/1241318438.jpg
 *
 *
 * ex2.) dotup
 */
response_t *get_http_response(url_t *url)
{
    int sock;
    FILE *fp;

    sock = get_http_socket(url->host);
    if (sock < 0) {
        return NULL;
    }

    fp = fdopen(sock, "r+");
    if (fp == NULL) {
        close(sock);
        return NULL;
    }

    fprintf(fp, "GET %s HTTP/1.1\r\n", url->path);
    fprintf(fp, "HOST: %s\r\n", url->host);
    fprintf(fp, "User-Agent: %s\r\n", USER_AGENT);
    fprintf(fp, "Connection: close\r\n");
    fprintf(fp, "\r\n");
    fflush(fp);

    response_t *res;
    res = xmalloc(sizeof(response_t));
    res->fp = fp;
    res->sock = sock;
    if (read_header(res) < 0) {
        free(res);
        close(sock);
        return NULL;
    }
    return res;
}

int read_header(response_t *res)
{
    int length;
    char buf[1024];
    char *p;

    if (fgets(buf, sizeof(buf), res->fp) == NULL) {
        return -1;
    }
    p = strchr(buf, ' ');
    res->status = atoi(++p);

    length = 0;
    while (fgets(buf, sizeof(buf), res->fp) != NULL) {
        if (strcmp(buf, "\r\n") == 0) break;

        if (strncasecmp(buf, "content-length:", 15) == 0) {
            p = buf + 15;
            length = atoi(p);
        }
    }
    if (length == 0) {
        return -1;
    }
    res->length = length;
    return 0;
}

void free_response(response_t *res)
{
    close(res->sock);
    free(res);
}
