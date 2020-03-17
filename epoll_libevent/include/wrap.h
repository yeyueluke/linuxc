#ifndef WRAP_H_
#define WRAP_H_
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */
#include <unistd.h>

noreturn void perror_exit(const char *err);
int           Socket(int domain, int type, int protocol);
int           Accept(int socket, struct sockaddr *addr, socklen_t *addr_len);
int Connect(int socket, const struct sockaddr *address, socklen_t address_len);
int Listen(int socket, int backlog);
int Bind(int socket, const struct sockaddr *address, socklen_t address_len);
ssize_t Read(int fildes, void *buf, size_t nbyte);
ssize_t Write(int fildes, const void *buf, size_t nbyte);
int     Setsockopt(int sockfd, int level, int optname, const void *optval,
                   socklen_t optlen);
int     Close(int fd);
#endif
