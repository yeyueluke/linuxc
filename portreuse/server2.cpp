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
#define PUBLICIP "192.168.153.132"
#define PORT 6666
#define myflag "123"

struct thread_param {
    int                connfd;
    struct sockaddr_in addr;
    int                localfd;
};

void handler(char *in, char *out) {
    for (int i = 0; i < (int)strlen(out) + 1; ++i) {
        out[i] = tolower(in[i]);
    }
}

void *workthread(void *param) {
    struct thread_param *clientinfo    = (struct thread_param *)param;
    int                  ret           = 0;
    int                  fd_client     = clientinfo->connfd;
    int                  fd_local      = clientinfo->localfd;
    char                 recvbuf[2048] = {0};
    char                 sendbuf[2048] = {0};
    char                 cmpstr[2048]  = {0};

    while (true) {
        ret = (int)Read(fd_client, recvbuf, sizeof(recvbuf));
        if (ret == 0) {
            break;
        }
        strcpy(cmpstr, recvbuf);
        cmpstr[strlen(myflag)] = '\0';
        /*看看是不是我的数据包，如果不是，则交给localhost:6666端口处理*/
        if (strcmp(cmpstr, myflag) != 0) {
            printf("going to send to local\n");
            Write(fd_local, recvbuf, strlen(recvbuf) + 1);
            Read(fd_local, sendbuf, sizeof(sendbuf));
            Write(fd_client, sendbuf, strlen(sendbuf) + 1);
            continue;
        }

        //是我的,我来处理
        printf("recv from %s:%d :  string: %s\n",
               inet_ntoa(clientinfo->addr.sin_addr),
               ntohs(clientinfo->addr.sin_port), recvbuf);

        handler(recvbuf + strlen(myflag), sendbuf);
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
    int                fd_server = 0;
    int                fd_client = 0;
    int                fd_local;
    int                ret = 0;
    struct sockaddr_in sock_client;
    struct sockaddr_in sock_server;
    struct sockaddr_in sock_local;
    int                opt        = 0;
    socklen_t          client_len = (socklen_t)sizeof(sock_client);

    struct thread_param thread_array[1024];
    int                 threadnum = 0;
    pthread_t           tid       = 0;
    /*connect to local*/
    startsock(fd_local, sock_local, LOCALIP, PORT);
    opt = 1;
    Setsockopt(fd_local, SOL_SOCKET, SO_REUSEADDR, &opt,
               (socklen_t)sizeof(opt));

    Connect(fd_local, (struct sockaddr *)&sock_local, sizeof(sock_local));

    /*server socket*/
    startsock(fd_server, sock_server, PUBLICIP, PORT);
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

        thread_array[threadnum].connfd  = fd_client;
        thread_array[threadnum].addr    = sock_client;
        thread_array[threadnum].localfd = fd_local;
        ret = pthread_create(&tid, nullptr, workthread,
                             &(thread_array[threadnum]));
        threadnum++;

        pthread_detach(tid);
    }
}
