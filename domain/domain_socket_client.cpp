#include <stddef.h>
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
#define CLIE_ADDR "clie.socket"

int main()
{
    int cfd, len;
    struct sockaddr_un servaddr, cliaddr;
    char buf[4096];
    
    cfd = Socket(AF_UNIX, SOCK_STREAM, 0);
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sun_family = AF_UNIX;
    strcpy(cliaddr.sun_path, CLIE_ADDR);
    
    len = offsetof(struct sockaddr_un, sun_path) + strlen(cliaddr.sun_path);
    
    unlink(CLIE_ADDR);
    Bind(cfd, (struct sockaddr *)&cliaddr, len);
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, SERV_ADDR);
    
    len = offsetof(struct sockaddr_un, sun_path) + strlen(servaddr.sun_path);
    
    Connect(cfd, (struct sockaddr *)&servaddr, len);
    while(fgets(buf, sizeof(buf),stdin) != NULL)
    {
    	
    	write(cfd, buf, strlen(buf));
    	len = read(cfd, buf, sizeof(buf));
    	write(STDOUT_FILENO, buf, len);
    	
    }
    close(cfd);
    return 0;
}
