#ifndef MyEPOLL_H_
#define MyEPOLL_H_

#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#define EPOLLIN_ET (EPOLLIN | EPOLLET)
//#define EPOLLIN_ET EPOLLIN
struct myevent_s {
    int      fd;
    uint32_t events;
    int      status;
    char     buf[1024];
    int      len;
    void *   arg;
    void (*callback)(void *);
};

void event_set(struct myevent_s *myevt, int fd, uint32_t events,
               void (*callback)(void *));

void event_add(int epfd, struct myevent_s *myevt);
void event_del(int epfd, struct myevent_s *myevt);
void event_mod(int epfd, struct myevent_s *myevt);

#endif
