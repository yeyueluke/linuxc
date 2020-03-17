#include "include/myepoll.h"
#include "include/wrap.h"
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */
#include <unistd.h>
#define MAX_CLIENT 1024
#define PORT 6666

static int                g_epfd;
static struct epoll_event g_epoll_evts[MAX_CLIENT];
static struct myevent_s   g_my_evts[MAX_CLIENT];

void callback_write(void *arg);
void callback_read(void *arg);
void handler(char *in, char *out) {
    for (int i = 0; i < (int)strlen(out) + 1; ++i) {
        out[i] = toupper(in[i]);
    }
}

void callback_write(void *arg) {
    printf("callback_write...\n");
    struct myevent_s *myevt = (struct myevent_s *)arg;
    Write(myevt->fd, myevt->buf, (size_t)myevt->len);
    printf("send: %s\n", myevt->buf);
    memset(myevt->buf, 0, sizeof(myevt->buf));

    event_mod(g_epfd, myevt);
    myevt->callback = callback_read;
    printf("change to read\n");
}

void callback_read(void *arg) {
    printf("callback_read...\n");
    struct myevent_s *myevt = (struct myevent_s *)arg;
    int               ret   = 0;
    myevt->len              = 0;

    while (true) {
        if (myevt->len >= (int)sizeof(myevt->buf)) break;

        ret = (int)Read(myevt->fd, myevt->buf + myevt->len, 2);
        // ret = (int)Read(myevt->fd, myevt->buf + ret,
        // sizeof(myevt->buf) - (unsigned long)myevt->len);
        printf("ret: %d\n", ret);
        if (ret > 0) {
            myevt->len += ret;
        } else if (ret == 0) {
            Close(myevt->fd);
            event_del(g_epfd, myevt);
            return;
        } else if ((ret == -1) && (myevt->len == 0)) {
            return;
        } else if ((ret == -1) && (myevt->len > 0)) {
            break;
        }
    }

    printf("recv: %s\n", myevt->buf);
    handler(myevt->buf, myevt->buf);

    event_mod(g_epfd, myevt);
    myevt->callback = callback_write;
    printf("change to write\n");
}

void callback_accept(void *arg) {
    printf("callback_accept...\n");
    struct myevent_s * myevt = (struct myevent_s *)arg;
    struct sockaddr_in addr;
    int                cfd = -1;
    int                i   = 0;
    socklen_t          len = sizeof(addr);

    cfd = Accept(myevt->fd, (struct sockaddr *)&addr, &len);
    printf("=========new client: socket %d %s:%d=============\n", cfd,
           inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	fcntl(cfd, F_SETFL, O_NONBLOCK);
    for (i = 0; i < MAX_CLIENT; ++i) {
        if (g_my_evts[i].status == 0) { break; }
    }
    event_set(&g_my_evts[i], cfd, EPOLLIN_ET, callback_read);
    event_add(g_epfd, &g_my_evts[i]);
}

int initsocket(int &lfd, int port) {
    int opt = 0;
    g_epfd  = epoll_create(MAX_CLIENT);

    lfd = Socket(AF_INET, SOCK_STREAM, 0);
    fcntl(lfd, F_SETFL, O_NONBLOCK);
    opt = 1;
    Setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, (socklen_t)sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((uint16_t)port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    Bind(lfd, (struct sockaddr *)&addr, sizeof(addr));

    event_set(&g_my_evts[MAX_CLIENT - 1], lfd, EPOLLIN_ET, callback_accept);
    event_add(g_epfd, &g_my_evts[MAX_CLIENT - 1]);

    Listen(lfd, 20);
    return 0;
}

int main() {
    int               lfd     = 0;
    int               i       = 0;
    int               nselect = 0;
    struct myevent_s *myevt   = nullptr;

    for (i = 0; i < MAX_CLIENT; ++i) {
        g_my_evts[i].status = 0;
    }

    initsocket(lfd, PORT);
    while (true) {
        printf("waiting...\n");
        nselect = epoll_wait(g_epfd, g_epoll_evts, MAX_CLIENT + 1, 1000);
        for (i = 0; i < nselect; ++i) {
            myevt = (struct myevent_s *)(g_epoll_evts[i].data.ptr);

            if (myevt->fd == lfd) {
                myevt->callback(myevt->arg);
                continue;
            }

            if ((g_epoll_evts[i].events & EPOLLIN_ET) &&
                (myevt->events & EPOLLIN_ET)) {
                myevt->callback(myevt->arg);
            }
            if ((g_epoll_evts[i].events & EPOLLOUT) &&
                (myevt->events & EPOLLOUT)) {
                myevt->callback(myevt->arg);
            }
        }
        sleep(1);
    }
    return 0;
}
