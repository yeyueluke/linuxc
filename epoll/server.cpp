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
#include <sys/epoll.h>
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
    int                epfd       = 0;
    int                nselect    = 0;
    int                i          = 0;
    struct epoll_event evts[FD_SETSIZE];
    struct epoll_event evt;
    startsock(fd_server, sock_server, LOCALIP, PORT);
    opt = 1;
    Setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &opt,
               (socklen_t)sizeof(opt));
    Bind(fd_server, (struct sockaddr *)&sock_server, sizeof(sock_server));
    Listen(fd_server, 5);
    epfd = epoll_create(FD_SETSIZE);
    if (epfd == -1) { perror_exit("epoll create failed"); }

    evt.events  = EPOLLIN;
    evt.data.fd = fd_server;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd_server, &evt);

    while (true) {
        printf("epolling...\n");
        nselect = epoll_wait(epfd, evts, FD_SETSIZE, -1);
        printf("get %d select\n", nselect);
        for (i = 0; i < nselect; ++i) {
            if (!(evts[i].events && EPOLLIN)) continue;
            if (evts[i].data.fd == fd_server) {
                fd_client = Accept(fd_server, (struct sockaddr *)&sock_client,
                                   &client_len);
                printf("accept: %s: %d\n", inet_ntoa(sock_client.sin_addr),
                       ntohs(sock_client.sin_port));
                evt.data.fd = fd_client;
                evt.events  = EPOLLIN;
                epoll_ctl(epfd, EPOLL_CTL_ADD, fd_client, &evt);
            } else {
                ret = workthread(evts[i].data.fd);
                if (ret <= 0) {
                    Close(evts[i].data.fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, evts[i].data.fd, nullptr);
                }
            }
        }
	}
    Close(fd_server);
}
