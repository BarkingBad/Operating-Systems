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
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "line.h"

int __exit = 0;

double diff(struct timeval start , struct timeval end){
    double start_ms, end_ms;

    start_ms = (double)start.tv_sec*1000000 + (double)start.tv_usec;
    end_ms = (double)end.tv_sec*1000000 + (double)end.tv_usec;

    return (double)end_ms - (double)start_ms;
}

void handle(int sig){
    __exit = 1;
}

int main(int argc, char* argv[]) {

    if(argc != 4) {
        printf("Wrong arguments count!");
        return 1;
    }
    int X, M, K;
    if(sscanf(argv[1], "%d", &X) < 0) {
        printf("Wrong argument X!");
        return 1;
    }
    if(sscanf(argv[2], "%d", &K) < 0) {
        printf("Wrong argument K!");
        return 1;
    }
    if(sscanf(argv[3], "%d", &M) < 0) {
        printf("Wrong argument M!");
        return 1;
    }
    signal(SIGINT, handle);

    key_t key = ftok("/tmp", 'a');
    int semaphores_set = semget(key, 2, IPC_CREAT | 0666);
    

    int shared_line = shmget(key+1, K * sizeof(struct Line), IPC_CREAT | 0666);
    struct Line * line = (struct Line *) shmat (shared_line, NULL,  0);

    int shared_metadata = shmget(key+2, sizeof(struct Metadata), IPC_CREAT | 0666);
    struct Metadata * metadata = (struct Metadata *) shmat (shared_metadata, NULL,  0);

    metadata->current_m = 0;
    metadata->K = K;
    metadata->M = M;
    for(int i = 0; i < K; i++) {
        line[i].line = -1;
    }

    int load;
    struct sembuf sops;
    sops.sem_flg = SEM_UNDO;
    int i = 0;

    
    while(__exit == 0) {
        printf("Here comes that truck!\nReady to be loaded!\n");
        semctl(semaphores_set, 1, SETVAL, 1);

        sops.sem_num = 0;
        sops.sem_op = -1;
        semop(semaphores_set, &sops, 1);

        load = 0;
        struct timeval current_time;
        gettimeofday(&current_time,NULL);
        for(i = 0; i < K && load + line[i].line <= X && line[i].line > 0; i++) {
            load += line[i].line;
            printf("Loading package: ID of worker: %d Time difference %f Encumbrance %d/%d\n", line[i].pid, diff(line[i].tv, current_time), load, X);
        }

        metadata->current_m -= load;

        for(int j = i; j < K; j++) {
            line[j-i].line = line[j].line;
            line[j-i].pid = line[j].pid;
            line[j-i].tv = line[j].tv;
        }

        for(int j = K - i; j < K; j++) {
            line[j].line = -1;
        }

        printf("Truck deploys...\n");

        semctl(semaphores_set, 0, SETVAL, 0);
        sleep(1);
    }


    semctl(semaphores_set, 0, IPC_RMID, 0);
    shmctl(shared_line, IPC_RMID, NULL);
    shmctl(shared_metadata, IPC_RMID, NULL);
    return 0;
}
