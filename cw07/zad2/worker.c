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
#define key_shared_placed "/placed_m"

int __exit = 0;

void handler(int sig) {
    __exit = 1;
}

void spawn(int M, int N, int C) {
    
    int shared_placed = shm_open(key_shared_placed, O_RDWR | O_CREAT, 0666);
    ftruncate(shared_placed, M * sizeof(int));
    int * placed = (int *) mmap (NULL, M * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED , shared_placed, 0);    


    char buf[8];
    pid_t loaders[M];
    for(int i = 0; i < M; i++) {
        loaders[i] = fork();
        if(loaders[i] == 0) {
            loader(N, C, i, M);
        }
    }
    while(__exit == 0) {

    }

    for(int i = 0; i < M; i++) {
        kill(loaders[i], SIGINT);
    }
    shm_unlink(key_shared_placed);
}

int main(int argc, char* argv[]) {

    signal(SIGINT, handler);

    if(argc == 3) {
        int M, N;
        if(sscanf(argv[1], "%d", &M) < 0) {
            printf("Wrong argument M!");
            return 1;
        }
        if(sscanf(argv[2], "%d", &N) < 0) {
            printf("Wrong argument N!");
            return 1;
        }
        spawn(M, N, -1);
    } else if(argc == 4) {
        int M, N, C;
        if(sscanf(argv[1], "%d", &M) < 0) {
            printf("Wrong argument M!");
            return 1;
        }
        if(sscanf(argv[2], "%d", &N) < 0) {
            printf("Wrong argument N!");
            return 1;
        }
        if(sscanf(argv[3], "%d", &C) < 0) {
            printf("Wrong argument C!");
            return 1;
        }
        spawn(M, N, C);
    } else {
        printf("Wrong arguments count!");
        return 1;
    }

    return 0;
}

int __exit2 = 0;

void handler2(int sig) {
    __exit2 = 1;
}

int loader(int N, int C, int ID, int MAX) {
    signal(SIGINT, handler2);


    sem_t *semaphore_0 = sem_open(key_0, O_RDWR, 0666, 0);
    sem_t * semaphore_1 = sem_open(key_1, O_RDWR, 0666, 0);

    if(semaphore_0 == SEM_FAILED) {
        printf("Execute trucker first!");
        return 1;
    }
    srand(time(NULL));

    int shared_metadata = shm_open(key_shared_metadata, O_RDWR, 0666);
    struct Metadata * metadata = (struct Metadata *) mmap (NULL, sizeof(struct Metadata), PROT_READ | PROT_WRITE, MAP_SHARED , shared_metadata, 0);

    int K = metadata->K;
    
    int shared_line = shm_open(key_shared_line, O_RDWR, 0666);
    struct Line * line = (struct Line *) mmap (NULL, K * sizeof(struct Line), PROT_READ | PROT_WRITE, MAP_SHARED , shared_line, 0);    

    int shared_placed = shm_open(key_shared_placed, O_RDWR | O_CREAT, 0666);
    int * placed = (int *) mmap (NULL, MAX * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED , shared_placed, 0);    

    int sample_package = (rand()%N + 1);
    while(C && __exit2 == 0) {

        sem_wait(semaphore_1);

        

        if(metadata->current_m + sample_package <= metadata->M) {
            metadata->current_m += sample_package;
            printf("Loader %d put package that weighs %d\n", getpid(), sample_package);
            
            for(int i = 0; i < metadata->K; i++) {
                if(line[i].line == -1) {
                    line[i].line = sample_package;
                    line[i].pid = getpid();
                    struct timeval current_time;
                    gettimeofday(&current_time,NULL);
                    line[i].tv = current_time;
                    

                    if(i == metadata->K - 1 || metadata->current_m == metadata->M) {
                        sem_post(semaphore_0);
                    } else {
                        sem_post(semaphore_1);
                    }

                break;
                }
            }
            sample_package = (rand()%N + 1);
        } else {  
            placed[ID] = 1;
            int res = 0;
            for(int i = 0; i < metadata->K; i++) res += placed[i];
            if(res == MAX) {
                sem_post(semaphore_0);
            } else {
                sem_post(semaphore_1);
            }
        }

        if(C > 0) C--;
    }

    munmap(line, K * sizeof(struct Line));
    munmap(metadata, sizeof(struct Metadata));
    sem_close(semaphore_0);
    sem_close(semaphore_1);
    exit(0);
}