#include "myepoll.h"

void event_set(struct myevent_s *myevt, int fd, uint32_t events,
               void (*callback)(void *arg)) {
    myevt->fd     = fd;
    myevt->events = events;
    myevt->status = 1;
    myevt->arg    = myevt;
    memset(myevt->buf, 0, sizeof(myevt->buf));
    myevt->len      = 0;
    myevt->callback = callback;
}

void event_add(int epfd, struct myevent_s *myevt) {
    struct epoll_event evt;
    evt.events   = myevt->events;
    evt.data.ptr = myevt;
    epoll_ctl(epfd, EPOLL_CTL_ADD, myevt->fd, &evt);
}
void event_del(int epfd, struct myevent_s *myevt) {
    myevt->status = 0;
    epoll_ctl(epfd, EPOLL_CTL_DEL, myevt->fd, nullptr);
}

void event_mod(int epfd, struct myevent_s *myevt) {
	epoll_ctl(epfd, EPOLL_CTL_DEL, myevt->fd, nullptr);

	struct epoll_event evt;
	if (myevt->events & EPOLLIN_ET) {
        myevt->events = EPOLLOUT;
    } else {
        myevt->events = EPOLLIN_ET;
    }
    evt.events   = myevt->events;
    evt.data.ptr = myevt;
	
    epoll_ctl(epfd, EPOLL_CTL_ADD, myevt->fd, &evt);
}
