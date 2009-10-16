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
 */
int http_get(url_t *url, FILE **fp)
{
    int sock;

    sock = get_http_socket(url->host);
    if (sock < 0) {
        fprintf(stderr, "get_http_socket(). %s %s\n", url->host, url->path);
        return sock;
    }

    *fp = fdopen(sock, "r+");
    if (*fp == NULL) {
        fprintf(stderr, "fdopen(). %s %s", url->host, url->path);
        return -1;
    }

    fprintf(*fp, "GET %s HTTP/1.1\r\n", url->path);
    fprintf(*fp, "HOST: %s\r\n", url->host);
    fprintf(*fp, "User-Agent: %s\r\n", USER_AGENT);
    fprintf(*fp, "Connection: close\r\n");
    fprintf(*fp, "\r\n");
    fflush(*fp);

    // create response
    return sock;
}
