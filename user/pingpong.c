#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    int p[2];
    char buf[1];

    pipe(p);

    if (fork() == 0) {
        read(p[0], buf, 1);
        printf("%d: received ping\n", getpid());
        write(p[1], "1", 1);
        exit(0);
    } else {
        write(p[1], "1", 1);
        wait((int*) 0x0);
        read(p[0], buf, 1);
        printf("%d: received pong\n", getpid());
    }
    exit(0);
}
