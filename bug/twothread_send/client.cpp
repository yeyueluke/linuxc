#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
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
int main() {
    int   fd_client = 0;
    int   ret       = 0;
    myint buf[32 * 1000];
    fd_client = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_client == -1) { perror_exit("socket"); }

    struct sockaddr_in sock;
    memset(&sock, 0, sizeof(sock));
    sock.sin_family      = AF_INET;
    sock.sin_addr.s_addr = inet_addr("127.0.0.1");
    sock.sin_port        = htons(6666);

    ret = connect(fd_client, (struct sockaddr *)&sock, sizeof(sock));
    if (ret == -1) { perror_exit("connect"); }
    int i = 0;
    int j = 0;
    while (true) {
        memset(buf, 0, 4 * 1000 * sizeof(myint));
        ret = (int)read(fd_client, buf, 4 * 1000 * sizeof(myint));
        for (i = 0; i < ret/sizeof(myint); i++) {
            if (buf[i].buf == 'b' || ret != 4 * 1000 * sizeof(myint)) {
                printf("ret=%d\n", ret);
                for (i = 0; i < ret/sizeof(myint); i++)
                    printf("%c, %d\n", buf[i].buf, buf[i].index);
                break;
            }
        }

        j += ret;
    }
    close(fd_client);
    return 0;
}

