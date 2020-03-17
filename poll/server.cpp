#include "include/wrap.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
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
    struct pollfd      fds[FD_SETSIZE];
    int                maxi    = 0;
    int                nselect = 0;
    int                i       = 0;

    startsock(fd_server, sock_server, LOCALIP, PORT);
    opt = 1;
    Setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &opt,
               (socklen_t)sizeof(opt));
    Bind(fd_server, (struct sockaddr *)&sock_server, sizeof(sock_server));
    Listen(fd_server, 5);

    for (i = 0; i < FD_SETSIZE; ++i) {
        fds[i].fd     = -1;
        fds[i].events = POLLIN;
    }
    fds[0].fd = fd_server;
    maxi   = 0;
    
	while (true) {
        printf("polling...\n");
        nselect = poll(fds, (unsigned long)maxi+1, -1);
        if (nselect == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror_exit("poll failed");
            }
        }
        printf("get %d select\n", nselect);
        if (fds[0].revents & POLLIN) {
            fd_client =
                Accept(fd_server, (struct sockaddr *)&sock_client, &client_len);
            printf("accept: %s: %d\n", inet_ntoa(sock_client.sin_addr),
                   ntohs(sock_client.sin_port));
            for (i = 0; i < FD_SETSIZE; ++i) {
                if (fds[i].fd != -1) continue;
                fds[i].fd = fd_client;
                break;
            }
            printf("i: %d, FD_SETSIZE: %d\n", i, FD_SETSIZE);
            if (i == FD_SETSIZE) perror_exit("too many clients");
            if (i > maxi) maxi = i;
            nselect--;
        }
        printf("going to find client, maxi: %d, nselect: %d\n", maxi, nselect);
        for (i = 0; (i <= maxi) && (nselect > 0); ++i) {
            if (!( fds[i].revents & POLLIN )) continue;
            printf("find client %d\n", i + 1);
            ret = workthread(fds[i].fd);
            if (ret <= 0) {
                Close(fds[i].fd);
                fds[i].fd = -1;
            }
            nselect--;
        }
        sleep(3);
    }
    Close(fd_server);
    for (i = 0; i < FD_SETSIZE; ++i) {
        if (fds[i].fd == -1) continue;
        Close(fds[i].fd);
    }
}
