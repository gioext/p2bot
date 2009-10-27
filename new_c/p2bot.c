#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

static int get_http_socket(char *host);

int main(int argc, char *argv[])
{
    daemon(0, 0);
    while (1) {
        int socket = get_http_socket(argv[1]);
        if (socket < 0) {
            fprintf(stderr, "get_http_socket\n");
            exit(1);
        }
        FILE *f = fdopen(socket, "r+");
        char buf[1024];

        fprintf(f, "GET / HTTP/1.1\r\n");
        fprintf(f, "Host: %s\r\n", argv[1]);
        fprintf(f, "Connection: close\r\n");
        fprintf(f, "\r\n");

        printf("%s", fgets(buf, sizeof(buf), f) ? buf : "");

        fclose(f);
        close(socket);
        sleep(1);
    }
    return 0;
}

static int get_http_socket(char *host)
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
