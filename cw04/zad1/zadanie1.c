#define _XOPEN_SOURCE 500
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


int running = 0;
int should_run = 1;
pid_t pid;

void handle_sigtstp(int signum){
    if(running == 1) {
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
        running = 0;
        should_run = 0;
        kill(pid, SIGINT);
    } else {
        should_run = 1;
    }
}

void handle_sigint(int signum) {
    if(running == 1) kill(pid, SIGINT);
    exit(0);
}

int main() {

    
    signal(SIGINT, handle_sigint);

    struct sigaction act;
    act.sa_handler = handle_sigtstp;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGTSTP, &act, NULL);

    while(1 == 1) {
        if(running == 0 && should_run == 1) {
            pid = fork();
            running = 1;
        }

        if(pid == 0) {
            execl("./script.sh", "script.sh", NULL);
        }
    }

    return 0;
}