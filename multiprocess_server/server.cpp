#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */
#include <unistd.h>
#include <wait.h>

noreturn void perror_exit(const char *err) {
    perror(err);
    exit(1);
}

void handler(char *in, char *out) {
    for (int i = 0; i < (int)strlen(out) + 1; ++i) {
        out[i] = toupper(in[i]);
    }
}

void wait_child(int) {
    while (waitpid(0, nullptr, WNOHANG) > 0) {
    }
    return;
}
int main() {
    pid_t              pid;
    int                fd_server     = 0;
    int                fd_client     = 0;
    int                ret           = 0;
    char               recvbuf[2048] = {0};
    char               sendbuf[2048] = {0};
    struct sockaddr_in sock_client;
    struct sockaddr_in sock_server;

    socklen_t client_len = (socklen_t)sizeof(sock_client);

    fd_server = socket(AF_INET, SOCK_STREAM, 0);

    if (fd_server == -1) {
        perror_exit("socket");
    }

    memset(&sock_server, 0, sizeof(sock_server));
    sock_server.sin_family      = AF_INET;
    sock_server.sin_addr.s_addr = inet_addr("127.0.0.1");
    sock_server.sin_port        = htons(6666);

    ret = bind(fd_server, (struct sockaddr *)&sock_server, sizeof(sock_server));
    if (ret == -1) {
        perror_exit("bind");
    }

    ret = listen(fd_server, 5);
    if (ret == -1) {
        perror_exit("listen failed");
    }

    while (true) {
        fd_client =
            accept(fd_server, (struct sockaddr *)&sock_server, &client_len);
        if (fd_client == -1) {
            perror_exit("bind");
        }
        printf("accept: %s: %d\n", inet_ntoa(sock_server.sin_addr),
               ntohs(sock_server.sin_port));
        pid = fork();
        if (pid == -1) {
            perror_exit("fork failed");
        } else if (pid == 0) {
            break;
        }
        // parent thread
        close(fd_client);
        signal(SIGCHLD, wait_child);
    }

    // parrent exit
    if (pid != 0) {
        return 0;
    }

    // child thread
    close(fd_server);
    while (true) {
    readagain:
        ret = (int)read(fd_client, recvbuf, 2048);
        if (ret == -1) {
            if (errno == EINTR) {
                goto readagain;
            } else {
                break;
            }
        } else if (ret == 0) {
            break;
        }

        printf("recv: %d bytes, string: %s\n", ret, recvbuf);
        handler(recvbuf, sendbuf);

    writeagain:
        ret = (int)write(fd_client, sendbuf, strlen(sendbuf) + 1);

        if (ret == -1) {
            if (errno == EINTR) {
                goto writeagain;
            } else {
                break;
            }
        } else if (ret == 0) {
            break;
        }

        printf("send\n");
    }
    close(fd_client);
    return 0;
}
