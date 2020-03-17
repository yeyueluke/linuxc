#include "include/wrap.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int  pfd[2];
    int  pid     = 0;
    char buf[3]  = {0};
    char buf1[3] = {0};
    int  len     = 0;

    pipe(pfd);
    pid = fork();

    if (pid == -1) { // error
        perror_exit("fork failed");
    }

    if (pid == 0) { // child
        close(pfd[0]);
        int j = 0;
        while (true) {
            sprintf(buf1, "%d\n", j++);
            write(pfd[1], buf1, 3);
            sprintf(buf1, "%d\n", j++);
            write(pfd[1], buf1, 3);
            sleep(3);
        }
        close(pfd[1]);
    } else { // parent
        close(pfd[1]);
        int                epfd = 0;
        struct epoll_event evts[1];
        struct epoll_event evt;

        int flag = 0;
        flag     = fcntl(pfd[0], F_GETFL);
        flag |= O_NONBLOCK;  //设置为非阻塞
        fcntl(pfd[0], F_SETFL, flag);

        epfd        = epoll_create(1);
        evt.data.fd = pfd[0];
        evt.events  = EPOLLIN | EPOLLET; //边沿
        // evt.events  = EPOLLIN;                   //水平（默认）
        epoll_ctl(epfd, EPOLL_CTL_ADD, pfd[0], &evt);
        while (true) {
            epoll_wait(epfd, evts, 1, -1);
            if (evts[0].data.fd == pfd[0]) {
                while ((len = (int)read(pfd[0], buf, 3)) > 0) { //体现非阻塞
                    write(STDOUT_FILENO, buf, (size_t)len);
                }
            }
        }
        close(pfd[0]);
    }

    return 0;
}
