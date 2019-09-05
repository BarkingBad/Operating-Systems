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
int flag = 0;


void send(pid_t pid, int mode, int i) {
    union sigval c;
    switch (mode) {
        case 0:
            kill(pid, 10);
            break;
        case 1:
            
            c.sival_int = i;
            sigqueue(pid, 10, c);
            break;
        case 2:
            kill(pid, SIGRTMIN);
            break;
    }
}

void end_of_transmision(pid_t pid, int mode) {
    union sigval c;
    switch (mode) {
        case 0:
            kill(pid, 12);
            break;
        case 1:
            sigqueue(pid, 12, c);
            break;
        case 2:
            kill(pid, SIGRTMIN+1);
            break;
    }
}

void count(int sig, siginfo_t *siginfo, void *context) {
    counter++;
    flag = 0;
    if(siginfo->si_code == SI_QUEUE) {
        printf("Received package number %d\n", siginfo->si_value.sival_int);
    }
}

void receive(int mode) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = count;
    act.sa_flags = SA_SIGINFO;
    switch (mode) {
        case 0:
            sigaction(10, &act, NULL);
            break;
        case 1:
            sigaction(10, &act, NULL);
            break;
        case 2:
            sigaction(SIGRTMIN, &act, NULL);
            break;
    }
}

void terminate(int sig,siginfo_t *info, void *void_ptr) {
    printf("Received %d packages", counter);
    sigusr2 = 1;
} 

void end(int mode) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = terminate;
    act.sa_flags = SA_SIGINFO;
    switch (mode) {
        case 0:
            sigaction(12, &act, NULL);
            break;
        case 1:
            sigaction(12, &act, NULL);
            break;
        case 2:
            sigaction(SIGRTMIN+1, &act, NULL);
            break;
    }
}

int main(int argc, char ** argv) {
    if(argc != 4) return -1;

    pid_t pid;
    sscanf(argv[1], "%d", &pid);
    int packages;
    sscanf(argv[2], "%d", &packages);

    sigset_t allsigs;
    sigfillset(&allsigs);
    sigdelset(&allsigs, 10);
    sigdelset(&allsigs, 12);
    sigdelset(&allsigs, SIGRTMIN);
    sigdelset(&allsigs, SIGRTMIN+1);

    int mode = -1;
    if(strcmp(argv[3], "KILL") == 0) {
        mode = 0;
    } else if(strcmp(argv[3], "SIGQUEUE") == 0) {
        mode = 1;
    } else if(strcmp(argv[3], "SIGRT") == 0) {
        mode = 2;
    } else {
        printf("Unsupported operation!\n");
        return 0;
    }
    int i = 0;

    receive(mode);
    while(i < packages) {
        send(pid, mode, i);
        flag = 1;
        while(flag == 1) pause();
        i++;
    }
    end(mode);
    end_of_transmision(pid, mode);
    pause();
    printf(" out of %d packages sent\n", packages);
    
    return 0;
}