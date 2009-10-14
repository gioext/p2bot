#ifndef P2BOT_H
#define P2BOT_H




#define USER_AGENT "Mozilla/5.0 (compatible; p2bot/0.1; +http://p2m.giox.org/)"

typedef struct {
    char *host;
    char *path;
} url_t;



char *save_url(url_t *url, char *savefile);
int open_connection(char *host, char *service);



url_t *getURL(char **body);
void *xmalloc(size_t size);


#endif
