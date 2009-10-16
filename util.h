#ifndef UTIL_H
#define UTIL_H

//#define USER_AGENT "Mozilla/5.0 (compatible; p2bot/0.1; +http://p2m.giox.org/)"
#define USER_AGENT "Mozilla/5.0"

typedef struct {
    FILE *fp;
    int sock;
    int length;
    int status;
} response_t;

typedef struct {
    char *host;
    char *path;
} url_t;

void *xmalloc(size_t size);
int http_get(url_t *url, FILE **fin);
int get_http_socket(char *host);
url_t *strtourl(const char *str);
void free_url(url_t *url);

#endif
