#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


/* 计算目录中文件数量
static int num;
void myread(const char *dirpath) {
    // printf("dirpath: %s\n", dirpath);
    DIR *          dir = nullptr;
    struct dirent *ent = nullptr;
    char subdir[256] = {0};

	dir                = opendir(dirpath);
    if (dir == nullptr) {
        perror("opendir failed");
        exit(1);
    }
    while ((ent = readdir(dir)) != nullptr) {
        if (ent->d_type == DT_REG) {
            num++;
            continue;
        } else if (ent->d_type == DT_DIR) {
            if ((!strcmp(ent->d_name, ".")) || (!strcmp(ent->d_name, ".."))) {
                continue;
            }
            printf("going to sub dir: %s\n", ent->d_name);
			strcpy(subdir, dirpath);
			strcat(subdir, "/");
			strcat(subdir, ent->d_name);
			myread(subdir);
        } else if (ent->d_type == DT_REG) {
            num++;
        }
    }
	closedir(dir);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("\nusage: ./main dirname");
        exit(1);
    }

    myread(argv[ 1 ]);

    printf("num: %d\n", num);
    return 0;
}*/
