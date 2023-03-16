#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    int p[2];
    pipe(p);

    for (int i = 2; i <= 35 ; ++i) {
        write(p[1], &i, 4);
    }
    
    int flag = 0;
    int pid = fork();
    while (pid == 0) {
        int buf[35];
        int n = read(p[0], buf, sizeof(buf));
        printf("prime %d\n", buf[0]);
        for (int i = 1; i < n / 4; ++i) {
            if (buf[i] % buf[0])
                write(p[1], &buf[i], 4);
        }
        if (n == 4) {
            flag = 1;
            break;
        }
        pid = fork();
    } 
    
    close(p[0]);
    close(p[1]);
    if (flag != 1)
        wait((int*)0x0);

    exit(0);
}

