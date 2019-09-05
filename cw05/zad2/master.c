#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    if(argc != 2) return -1;
    char buf[100];
    mkfifo(argv[1], S_IRWXU);

    int fd = open(argv[1], O_RDONLY);
    while(1 == 1) {
        if(read(fd, &buf, 100) != 0) {
            printf("%s", buf);
        }
    }

    return 0;
}