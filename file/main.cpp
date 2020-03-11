#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/* read and write string
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
}*/

/* 读写文件
int main() {
    int fd1 = open("testfile1", O_RDONLY);
    int fd2 = open("testfile2", O_CREAT | O_WRONLY, S_IRWXU);
    if ((fd1==-1)||(fd2==-1)) {
        perror("oprn failed");
        exit(1);
    }
    char buf[2048] = {0};
    while (true) {
        int nbytes = (int) read(fd1, buf, sizeof(buf));
        if (nbytes==-1) {
            perror("read failed");
            exit(1);
        } else if (nbytes==0) {
            break;
        }
        nbytes =(int) write(fd2, buf, (size_t)nbytes);
        if(nbytes==-1){
            perror("write failed");
            exit(1);
        }
    }

    close(fd1);
    close(fd2);
    return 0;
} */

/* 文件拓展
int main() {
    int fd = open("testlseek", O_RDWR);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }
    int ret = (int) lseek(fd, 0, SEEK_END);
    printf("file length: %d\n", ret);
    ret = (int) lseek(fd, 2000, SEEK_END);

    printf("file length: %d\n", ret);
    ret = (int) write(fd, "!", 1);
    if (ret == -1) {
        perror("write failed");
        exit(1);
    }
    close(fd);
    return 0;
}*/

/* ls -l
void rwxhandler(__mode_t mt, unsigned *mask) {
    char flag[ 3 ] = {'r', 'w', 'x'};
    for (unsigned i = 0; i < 3; ++i) {
        if (mt & (*(mask + i))) {
            printf("%c", flag[ i ]);
        } else {
            printf("-");
        }
    }
}

int filehandler(struct stat filestat) {
    // rwx
    unsigned mask[ 3 ][ 3 ] = {{S_IRUSR, S_IWUSR, S_IXUSR},
                               {S_IRGRP, S_IWGRP, S_IXGRP},
                               {S_IROTH, S_IWOTH, S_IXOTH}};
    rwxhandler(filestat.st_mode, mask[ 0 ]);
    rwxhandler(filestat.st_mode, mask[ 1 ]);
    rwxhandler(filestat.st_mode, mask[ 2 ]);
    printf(" ");
    // hard link
    printf("%d ", (int) filestat.st_nlink);

    // username
    printf("%s %s ", getpwuid(filestat.st_uid)->pw_name,
           getgrgid(filestat.st_gid)->gr_name);

    // file size
    printf("%d ", (int) filestat.st_size);

    // last time
    printf("%s ", ctime(&(filestat.st_mtim.tv_sec)));

    return 0;
}

int dirhandler([>struct stat filestat<]) {
	//todo
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: \n./main filepath\n");
        return 0;
    }

    struct stat filestat;
    memset(&filestat, 0, sizeof(struct stat));

    int ret = stat(argv[ 1 ], &filestat);
    if (ret == -1) {
        perror("stat failed");
        exit(1);
    }

    __mode_t mt = filestat.st_mode;
    // normal file
    if ((mt & S_IFMT) == S_IFDIR) {
        dirhandler([>filestat<]);
    } else {
        filehandler(filestat);
    }

    printf("%s\n", argv[ 1 ]);
    return 0;
}
*/
