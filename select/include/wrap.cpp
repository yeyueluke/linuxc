#include "wrap.h"

noreturn void perror_exit(const char *err) {
    perror(err);
    exit(1);
}

int Socket(int domain, int type, int protocol) {
    int fd = socket(domain, type, protocol);
    if (fd == -1) {
        perror_exit("socket");
    }
    return fd;
}

int Bind(int socket, const struct sockaddr *address, socklen_t address_len) {
    int ret = bind(socket, address, address_len);
    if (ret == -1) {
        perror_exit("bind failed");
    }
    return ret;
}

int Listen(int socket, int backlog) {
    int ret = listen(socket, backlog);
    if (ret == -1) {
        perror_exit("listen failed");
    }
    return ret;
}

int Accept(int socket, struct sockaddr *addr, socklen_t *addr_len) {
    int fd = accept(socket, addr, addr_len);
    if (fd == -1) {
        perror_exit("accept failed");
    }
    return fd;
}

int Connect(int socket, const struct sockaddr *address, socklen_t address_len) {
    int ret = connect(socket, address, address_len);
    if (ret == -1) {
        perror_exit("connect failed");
    }
    return ret;
}

ssize_t Read(int fildes, void *buf, size_t nbyte) {
readagain:
    ssize_t ret = read(fildes, buf, nbyte);
    if (ret == -1) {
        if (errno == EINTR) {
            goto readagain;
        } else {
            perror_exit("read failed");
        }
    }
    return ret;
}

ssize_t Write(int fildes, const void *buf, size_t nbyte) {
writeagain:
    ssize_t ret = write(fildes, buf, nbyte);

    if (ret == -1) {
        if (errno == EINTR) {
            goto writeagain;
        } else {
            perror_exit("read failed");
        }
    }
    return ret;
}

int Setsockopt(int sockfd, int level, int optname, const void *optval,
               socklen_t optlen) {
    int ret = setsockopt(sockfd, level, optname, optval, optlen);
    if (ret == -1) {
        perror_exit("setsockopt failed");
    }
    return ret;
}

int Close(int fd){ 
    int ret = close(fd);
	if (ret==-1) { 
	    perror_exit("close failed"); 
	} 
	return ret;
} 
