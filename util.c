#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char *host;
    char *path;
} url_t;

url_t *searchURL(char **body);

url_t *searchURL(char **body)
{
    url_t *url;
    char ch, *chs;

    /* search '//' */
    while((chs = strstr(*body, "//")) != NULL) {
        chs += 2;
        *body = chs;

        ch = *chs;
        while (isalnum(ch) || strchr(".?/", ch) != NULL) {
            ch = *(++chs);
        }
        *chs = '\0';

        // valid
        
        url = malloc(sizeof(url_t));
        url->host = malloc(strlen(*body) + 1);
        strcpy(url->host, *body);
        url->path = malloc(strlen(*body) + 1);
        strcpy(url->path, *body);

        *body = ++chs;
        return url;
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    url_t *url;
    FILE *fp;
    char buf[2048];
    char *bufp;
    if ((fp = fopen("test/thread.txt", "r")) == NULL) {
        fprintf(stderr, "file open error %s\n", "test/thread.txt");
        exit(1);
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        bufp = buf;
        while ((url = searchURL(&bufp)) != NULL) {
            printf("Host: %s\n", url->host);
            printf("Path: %s\n", url->path);
            puts("");
        }
    }

    return 0;
}
