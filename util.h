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
int get_http_socket(char *host);
url_t *strtourl(const char *str);
void free_url(url_t *url);

response_t *get_http_response(url_t *url);
void free_response(response_t *res);

int read_header(response_t *res);

#endif
