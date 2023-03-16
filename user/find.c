#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../kernel/fs.h"
#include "user.h"


void find(char *path, char *file) {
    int fd;
    struct stat st;
    struct dirent de;
    char buf[512];

    fd = open(path, 0);
    fstat(fd, &st);

    strcpy(buf, path);
    int pathLen = strlen(buf);
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0) 
            continue;
        char *p = buf + pathLen;
        *p++ = '/';

        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if (strcmp(de.name, file) == 0) { 
            printf("%s\n", buf);
        } 

        int subFD = open(de.name, 0);
        struct stat subST;
        fstat(subFD, &subST);
        close(subFD);
        if (subST.type == T_DIR && 
            strcmp(de.name, ".") != 0 && 
            strcmp(de.name, "..") != 0){ 

            find(buf, file);
        }
    }
    close(fd);
}


int main(int argc, char *argv[]) {
    if (argc < 3) { 
        printf("Usage: find dir file\n");
        exit(1);
    }
    char *path = argv[1];
    char *file = argv[2];

    find(path, file); 
    exit(0);
}
