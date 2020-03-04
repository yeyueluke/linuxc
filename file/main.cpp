#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h> 
#include <string.h>
int main() {
    int fd, ret;
    //fd = open("bucunzai", O_RDWR);
	// 创建文件
	//	fd= 	open("myhello.txt", O_CREAT, S_IRWXG);
	// 判断文件是否存在 
	//fd = open("Makefile", O_CREAT | O_EXCL, S_IRWXG);
	fd = open("123", O_CREAT|O_RDWR, S_IRWXU);
	if (fd == -1) {
        perror("open failed");
        if (errno == EEXIST) { 
            printf("err: EEXIST\n"); 
        } 
		exit(1);
    }
	const char * hello = "hello world";
	ret = (int)write(fd, hello, strlen(hello)+1);
	if (ret ==-1) { 
	    perror("write failed");
		exit(1);
	} 
	printf("write %d bytes\n", ret);
	
	ret = (int)lseek(fd, 0, SEEK_SET);
	if (ret ==-1) { 
	    perror("lseek faild");
		exit(1);
	} 
	char * buf = new char[250];
	memset(buf, 0, 250);
	ret = (int)read(fd, buf, 250);
	if (ret ==-1) { 
	    perror("read faild");
		exit(1);
	}
	printf("read %d bytes, string: %s\n",ret,  buf);


    ret = close(fd);
    if (ret == -1) {
        perror("close failed");
        exit(1);
    }
    return 0;
}
