#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char *host;
    char *path;
} url_t;

url_t *getURL(char **body);

url_t *getURL(char **body)
{
    url_t *url;
    char ch, *cs, *ce, *cp;
    cs = *body;

    /* search url */
    while((ce = strstr(cs, "//")) != NULL) {
        ce += 2;
        cs = ce;

        ch = *ce;
        while (isalnum(ch) || strchr(".?/", ch) != NULL) {
            ch = *(++ce);
        }
        *ce = '\0';

        // valid
        if (strchr(cs, '/') == NULL || strstr(cs, ".jpg") == NULL) {
//            fprintf(stderr, "%s\n", cs);
            continue;
        }
        
        cp = strchr(cs, '/');
        
        url = malloc(sizeof(url_t));
        url->host = malloc(cp - cs + 1);
        strncpy(url->host, cs, cp - cs);
        url->path = malloc(strlen(cp) + 1);
        strcpy(url->path, cp);

        *body = ce + 1;
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
        while ((url = getURL(&bufp)) != NULL) {
            fprintf(stdout, "Host: %s, Path: %s\n", url->host, url->path);
        }
    }

    return 0;
}
