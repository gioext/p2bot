#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

int main()
{
    int sock;
    struct addrinfo hints, *res;
    char *host = "www.google.co.jp";
    char *port = "80";

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        return -1;
    }
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        return -1;
    }
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        close(sock);
        return -1;
    }
    freeaddrinfo(res);

    char buf[256];
    FILE *f = fdopen(sock, "r+");
    fprintf(f, "GET / HTTP/1.1\r\n");
    fprintf(f, "Host: www.google.co.jp\r\n");
    fprintf(f, "\r\n");
    fgets(buf, sizeof(buf), f);
    std::cout << buf << std::endl;
    return 0;
}
