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

int sigusr2 = 0;
int counter = 0;
pid_t pid;

void count_kill(int sig, siginfo_t *siginfo, void *context) {
    counter++;
    pid = siginfo->si_pid;
    kill(pid, 10);
}

void count_sigqueue(int sig, siginfo_t *siginfo, void *context) {
    counter++;
    pid = siginfo->si_pid;
    union sigval c;
    c.sival_int = counter;
    sigqueue(pid, 10, c);
    
}

void count_rt(int sig, siginfo_t *siginfo, void *context) {
    counter++;
    pid = siginfo->si_pid;
    kill(pid, SIGRTMIN);
}

void receive(int mode) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    switch (mode) {
        case 0:
            act.sa_sigaction = count_kill;
            sigaction(10, &act, NULL);
            break;
        case 1:
            act.sa_sigaction = count_sigqueue;
            sigaction(10, &act, NULL);
            break;
        case 2:
            act.sa_sigaction = count_rt;
            sigaction(SIGRTMIN, &act, NULL);
            break;
    }
}

void end_of_transmision_kill(int sig,siginfo_t *info, void *void_ptr) {
    printf("Received %d packages\n", counter);
    kill(pid, 12);
    sigusr2 = 1;
} 

void end_of_transmision_queue(int sig,siginfo_t *info, void *void_ptr) {
    printf("Received %d packages\n", counter);
    union sigval c;
    sigqueue(pid, 12, c);
    sigusr2 = 1;
} 

void end_of_transmision_rt(int sig,siginfo_t *info, void *void_ptr) {
    printf("Received %d packages\n", counter);
    kill(pid, SIGRTMIN+1);
    sigusr2 = 1;
} 

void end_of_transmision(int mode) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    switch (mode) {
        case 0:
            act.sa_sigaction = end_of_transmision_kill;
            sigaction(12, &act, NULL);
            break;
        case 1:
            act.sa_sigaction = end_of_transmision_queue;
            sigaction(12, &act, NULL);
            break;
        case 2:
            act.sa_sigaction = end_of_transmision_rt;
            sigaction(SIGRTMIN+1, &act, NULL);
            break;
    }
}

int main(int argc, char ** argv) {
    if(argc != 2) return -1;
    

    sigset_t allsigs;
    sigfillset(&allsigs);
    sigdelset(&allsigs, 10);
    sigdelset(&allsigs, 12);
    sigdelset(&allsigs, SIGRTMIN);
    sigdelset(&allsigs, SIGRTMIN+1);

    int mode = -1;
    if(strcmp(argv[1], "KILL") == 0) {
        mode = 0;
    } else if(strcmp(argv[1], "SIGQUEUE") == 0) {
        mode = 1;
    } else if(strcmp(argv[1], "SIGRT") == 0) {
        mode = 2;
    } else {
        printf("Unsupported operation!\n");
        return 0;
    }
    printf("PID: %d\n", getpid());
    
    
    receive(mode);
    end_of_transmision(mode);


    while(sigusr2 == 0) {
        pause();
    }
    return 0;
}