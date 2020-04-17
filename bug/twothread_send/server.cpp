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
noreturn void perror_exit(const char *err) {
    perror(err);
    exit(1);
}
typedef struct {
    char buf;
    int  index;
} myint;

static int fd_client;
void *     athread(void *) {
    int   i   = 0;
    int   ret = 0;
    myint buf[1000 * 4 * sizeof(myint)];
    for (i = 0; i < 4 * 1000; i++) {
        buf[i].buf = 'a';
        buf[i].index = i;
    }

    while (true) {
        ret = write(fd_client, buf, 1000 * 4 * sizeof(myint));
    }
    sleep(3);
    if (fd_client == 0) {
        close(fd_client);
        fd_client = 0;
    }
    return nullptr;
}
void *bthread(void *) {
    int   ret = 0;
	int i=0;
    myint buf[100 * sizeof(myint)];
    for (i = 0; i < 100; i++) {
        buf[i].buf = 'b';
        buf[i].index = i;
    }

    // usleep(1);
    while (true) {
        ret = 0;
        for (int i = 0; i < 10; ++i) {
            ret += write(fd_client, buf, 100*sizeof(myint));
        }
        sleep(1);
    }
    sleep(3);

    return nullptr;
}
int main() {
	printf("sizeof myint: %ld....\n", sizeof(myint));
    int                fd_server      = 0;
    int                ret            = 0;
    struct sockaddr_in sock_client;
    struct sockaddr_in sock_server;

    socklen_t client_len = (socklen_t)sizeof(sock_client);

    fd_server = socket(AF_INET, SOCK_STREAM, 0);

    if (fd_server == -1) { perror_exit("socket"); }

    memset(&sock_server, 0, sizeof(sock_server));
    sock_server.sin_family      = AF_INET;
    sock_server.sin_addr.s_addr = inet_addr("127.0.0.1");
    sock_server.sin_port        = htons(6666);

    ret = bind(fd_server, (struct sockaddr *)&sock_server, sizeof(sock_server));
    if (ret == -1) { perror_exit("bind"); }

    ret = listen(fd_server, 5);
    if (ret == -1) { perror_exit("listen failed"); }
    fd_client = accept(fd_server, (struct sockaddr *)&sock_server, &client_len);
    if (fd_client == -1) { perror_exit("bind"); }
    printf("accept: %s: %d\n", inet_ntoa(sock_server.sin_addr),
           ntohs(sock_server.sin_port));

    pthread_t pid[2];
    pthread_create(&pid[0], nullptr, bthread, nullptr);
    pthread_create(&pid[1], nullptr, athread, nullptr);
    pthread_join(pid[0], nullptr);
    pthread_join(pid[1], nullptr);

    sleep(5);
    close(fd_client);
    close(fd_server);
    return 0;
}
