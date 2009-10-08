#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>

#define USER_AGENT "Mozilla/5.0 (compatible; p2bot/0.1; +http://p2m.giox.org/)"

static char *save_url(char *host, char *path, char *savefile);
static int open_connection(char *host, char *service);

int main(int argc, char *argv[])
{
    int opt;
    int header = 0;

    while ((opt = getopt(argc, argv, "s")) != -1) {
        switch (opt) {
        case 's':
            header = 1;
            break;
        }
    }
    argc -= optind;
    argv += optind;
    int i;
    for (i = 0; i < 10; i++) {
        save_url(argv[0], argv[1], argv[2]);
        usleep(500000);
    }
    return 0;
}

char *save_url(char *host, char *path, char *savefile) {
    int sock;
    char *p;
    FILE *fin, *fout;
    char buf[1024];

    sock = open_connection(host, "80");
    if (sock < 0) {
        fprintf(stderr, "open_connection failed\n");
        return NULL;
    }
    fin = fdopen(sock, "r");
    fout = fdopen(sock, "w");

    fprintf(fout, "GET %s HTTP/1.1\r\n", path);
    fprintf(fout, "HOST: %s\r\n", host);
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

    printf("Content-Length: %d\n", length); 

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

static int
open_connection(char *host, char *service)
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
