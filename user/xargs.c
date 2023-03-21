#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"


int main(int argc, char *argv[]) {
    char *buf[MAXARG + 1];
    int bufLen = argc - 1;
    for (int i = 1; i < argc; ++i) 
        buf[i - 1] = argv[i];

    char word[512];
    char *p = word;
    while (1) {
        int n = read(0, p, 1);
        if (n == 0) {
            break;
        }
        if (*p == '\n') {
            *p = 0;
            buf[bufLen] = (char*) malloc(sizeof(char)* (strlen(word) + 1));
            strcpy(buf[bufLen++], word);
            p = word;
        } else {
            p += 1;
        }
    }

    buf[bufLen] = 0;

    if (fork() == 0) {
        exec(argv[1], buf);      
    } else {
        wait((int*) 0x0);
    }
    
    for (int i = argc - 2; i < bufLen; ++i) 
        free(buf[i]);
    exit(0);
}


