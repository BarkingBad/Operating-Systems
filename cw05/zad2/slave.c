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

int main(int argc, char **argv){
    if(argc != 3) return -1;
    srand(time(NULL));
    int fd = open(argv[1],O_WRONLY);
    char buffer[128];
    char date_str[128];
    int N;
    sscanf(argv[2], "%d", &N);
    printf("%d\n", getpid());

    for(int i = 0; i < N; i++) {
        FILE * date = popen("date","r");
        sprintf(buffer, "%d ", getpid());
        fread(date_str, sizeof(char), 128, date);
        strcat(buffer, date_str);
        write(fd, buffer, 128*sizeof(char));
        pclose(date);
        sleep(rand()%3+2);
    }

    close(fd);
    return 0;
}