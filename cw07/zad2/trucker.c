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
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include "line.h"

#define key_0 "/trucker_s"
#define key_1 "/loader_s"
#define key_shared_metadata "/loader_m"
#define key_shared_line "/trucker_m"

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


    sem_t * semaphore_0 = sem_open(key_0, O_RDWR | O_CREAT, 0666, 0);
    sem_t * semaphore_1 = sem_open(key_1, O_RDWR | O_CREAT, 0666, 0);


    int shared_line = shm_open(key_shared_line, O_RDWR | O_CREAT, 0666);
    ftruncate(shared_line, K * sizeof(struct Line));
    struct Line * line = (struct Line *) mmap (NULL, K * sizeof(struct Line), PROT_READ | PROT_WRITE, MAP_SHARED , shared_line, 0);    

    int shared_metadata = shm_open(key_shared_metadata, O_RDWR | O_CREAT, 0666);
    ftruncate(shared_metadata, sizeof(struct Metadata));
    struct Metadata * metadata = (struct Metadata *) mmap (NULL, sizeof(struct Metadata), PROT_READ | PROT_WRITE, MAP_SHARED , shared_metadata, 0);    

    metadata->current_m = 0;
    metadata->K = K;
    metadata->M = M;
    for(int i = 0; i < K; i++) {
        line[i].line = -1;
    }

    int load;
    int i = 0;

    
    while(__exit == 0) {
        printf("Here comes that truck!\nReady to be loaded!\n");

        sem_post(semaphore_1);
        sem_wait(semaphore_0);

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

        sleep(1);
    }

    sem_unlink(key_0);
    sem_unlink(key_1);
    sem_close(semaphore_0);
    sem_close(semaphore_1);

    shm_unlink(key_shared_line);
    shm_unlink(key_shared_metadata);
    munmap(line,  K * sizeof(struct Line));
    munmap(metadata, sizeof(struct Metadata));
    return 0;
}