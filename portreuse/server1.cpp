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
#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */
#include <unistd.h>
#include <wait.h>
#define LOCALIP "127.0.0.1"
#define PORT 6666

struct thread_param {
    int                connfd;
    struct sockaddr_in addr;
};

void handler(char *in, char *out) {
    for (int i = 0; i < (int)strlen(out) + 1; ++i) {
        out[i] = toupper(in[i]);
    }
}

void *workthread(void *param) {
    struct thread_param *clientinfo    = (struct thread_param *)param;
    int                  ret           = 0;
    int                  fd_client     = clientinfo->connfd;
    char                 recvbuf[2048] = {0};
    char                 sendbuf[2048] = {0};

    while (true) {
        ret = (int)Read(fd_client, recvbuf, 2048);
        if (ret == 0) {
            break;
        }

        printf("recv from %s:%d :  string: %s\n",
               inet_ntoa(clientinfo->addr.sin_addr),
               ntohs(clientinfo->addr.sin_port), recvbuf);
        handler(recvbuf, sendbuf);

        ret = (int)Write(fd_client, sendbuf, strlen(sendbuf) + 1);
        if (ret == 0) {
            break;
        }
    }
    close(fd_client);
    return nullptr;
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
    int                 fd_server = 0;
    int                 fd_client = 0;
    int                 ret       = 0;
    struct sockaddr_in  sock_client;
    struct sockaddr_in  sock_server;
    socklen_t           client_len = (socklen_t)sizeof(sock_client);
    int                 opt        = 0;
    struct thread_param thread_array[1024];
    int                 threadnum = 0;
    pthread_t           tid       = 0;

    startsock(fd_server, sock_server, LOCALIP, PORT);
    opt = 1;
    Setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &opt,
               (socklen_t)sizeof(opt));
    Bind(fd_server, (struct sockaddr *)&sock_server, sizeof(sock_server));
    Listen(fd_server, 5);

    while (true) {
        fd_client =
            Accept(fd_server, (struct sockaddr *)&sock_client, &client_len);
        printf("accept: %s: %d\n", inet_ntoa(sock_client.sin_addr),
               ntohs(sock_client.sin_port));

        thread_array[threadnum].connfd = fd_client;
        thread_array[threadnum].addr   = sock_client;
        ret = pthread_create(&tid, nullptr, workthread,
                             &(thread_array[threadnum]));
        threadnum++;

        pthread_detach(tid);
    }
}
