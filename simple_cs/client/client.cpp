#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
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

int main() {
    int  fd_client     = 0;
    int  ret           = 0;
    char sendbuf[2048] = {0};
    char recvbuf[2048] = {0};

    fd_client = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_client == -1) {
        perror_exit("socket");
    }

    struct sockaddr_in sock;
    memset(&sock, 0, sizeof(sock));
    sock.sin_family      = AF_INET;
    sock.sin_addr.s_addr = inet_addr("192.168.153.132");
    sock.sin_port        = htons(6666);

    ret = connect(fd_client, (struct sockaddr *)&sock, sizeof(sock));
    if (ret == -1) {
        perror_exit("connect");
    }
    while (true) {
		// fgets得到的是字符串+\n+\0
        //fgets(sendbuf, sizeof(sendbuf), stdin);
		printf("input:");
		scanf("%s", sendbuf);
	

    writeagain:
        ret = (int)write(fd_client, sendbuf, strlen(sendbuf) + 1);
		printf("send : %d nbytes\n", (int)strlen(sendbuf)+1);
        if (ret == -1) {
            if (errno == EINTR) {
                goto writeagain;
            } else {
                break;
            }
        } else if (ret == 0) {
            break;
        }

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
    }
    close(fd_client);

    return 0;
}
