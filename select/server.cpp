#include "include/wrap.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */
#include <unistd.h>
#include <wait.h>
#define LOCALIP "127.0.0.1"
#define PORT 6666

void handler(char *in, char *out) {
    for (int i = 0; i < (int)strlen(out) + 1; ++i) {
        out[i] = toupper(in[i]);
    }
}

int workthread(const int &fd_client) {
    char recvbuf[2048] = {0};
    char sendbuf[2048] = {0};
    int  ret           = 0;

    ret = (int)Read(fd_client, recvbuf, 2048);
    if (ret <= 0) {
        printf("ret==0\n");
        return ret;
    }

    handler(recvbuf, sendbuf);

    ret = (int)Write(fd_client, sendbuf, strlen(sendbuf) + 1);
    return ret;
}

void startsock(int &fd, struct sockaddr_in &addr, const char *ip,
               const int port) {
    fd = Socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port        = htons(port);
}
int main() {
    int                fd_server = 0;
    int                fd_client = 0;
    int                ret       = 0;
    struct sockaddr_in sock_client;
    struct sockaddr_in sock_server;
    socklen_t          client_len = (socklen_t)sizeof(sock_client);
    int                opt        = 0;
    fd_set             readfds;
    fd_set             allfds;
    int                fd_clients[FD_SETSIZE];
    memset(fd_clients, -1, sizeof(fd_clients));
    int maxi    = 0;
    int fd_max  = 0;
    int nselect = 0;
    int i       = 0;

	printf("sizeof fd_set : %d\n", (int)sizeof(fd_set));

    startsock(fd_server, sock_server, LOCALIP, PORT);
    opt = 1;
    Setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &opt,
               (socklen_t)sizeof(opt));
    Bind(fd_server, (struct sockaddr *)&sock_server, sizeof(sock_server));
    Listen(fd_server, 5);

    FD_ZERO(&allfds);
    FD_ZERO(&readfds);
    FD_SET(fd_server, &allfds);
    fd_max = fd_server;

    while (true) {
        readfds = allfds;
        printf("selecting...\n");
        nselect = select(fd_max + 1, &readfds, nullptr, nullptr, nullptr);
        if (nselect == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror_exit("select failed");
            }
        }
        printf("get %d select\n", nselect);
        if (FD_ISSET(fd_server, &readfds)) {
            fd_client =
                Accept(fd_server, (struct sockaddr *)&sock_client, &client_len);
            printf("accept: %s: %d\n", inet_ntoa(sock_client.sin_addr),
                   ntohs(sock_client.sin_port));
            for (i = 0; i < FD_SETSIZE; ++i) {
                if (fd_clients[i] != -1) continue;
                fd_clients[i] = fd_client;
                break;
            }
            printf("i: %d, FD_SETSIZE: %d\n", i, FD_SETSIZE);
            if (i == FD_SETSIZE) perror_exit("too many clients");
            if (i > maxi) maxi = i;
            if (fd_client > fd_max) fd_max = fd_client;
            FD_SET(fd_client, &allfds);
            nselect--;
        }
        printf("going to find client, maxi: %d, nselect: %d\n", maxi, nselect);
        for (i = 0; (i <= maxi) && (nselect > 0); ++i) {
            if (FD_ISSET(fd_clients[i], &readfds) == 0) continue;
            printf("find client %d\n", i + 1);
           ret = workthread(fd_clients[i]);
            if (ret <= 0) {
                Close(fd_clients[i]);
                FD_CLR(fd_clients[i], &allfds);
                fd_clients[i] = -1;
            }
            nselect--;
        }
        sleep(3);
    }
    Close(fd_server);
    for (i = 0; i < FD_SETSIZE; ++i) {
        if (fd_clients[i] == -1) continue;
        Close(fd_clients[i]);
    }
}
