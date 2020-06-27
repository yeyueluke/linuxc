﻿#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include "wrap.h"

#define SERV_ADDR "serv.socket"

int main()
{
    int lfd, cfd, len, size, i;
    struct sockaddr_un servaddr, cliaddr;
    char buf[4096];
    
    lfd = Socket(AF_UNIX, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, SERV_ADDR);
    
    len = offsetof(struct sockaddr_un, sun_path) + strlen(servaddr.sun_path);
    
    unlink(SERV_ADDR);
    Bind(lfd, (struct sockaddr *)&servaddr, len);
    Listen(lfd, 10);
    while(1)
    {
    	len = sizeof(cliaddr);
    	cfd = Accept(lfd, (struct sockaddr *)&cliaddr, (socklen_t *)&len);
    	len -= offsetof(struct sockaddr_un, sun_path);
    	cliaddr.sun_path[len] = '\0';
    	while((size = read(cfd, buf, sizeof(buf))) > 0)
    	{
    		for(i = 0; i < size; i++)
    			buf[i] = toupper(buf[i]);
    		write(cfd, buf, size);
    	}
    	close(cfd);
    }
    close(lfd);
    return 0;
}