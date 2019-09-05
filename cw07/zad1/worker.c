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

void handler(int sig) {
    __exit = 1;
}

void spawn(int M, int N, int C) {
    
    key_t key = ftok("/tmp", 'a');
    int shared_placed = shmget(key+3, sizeof(int) * M, IPC_CREAT | 0666);
    int * placed = (int *) shmat (shared_placed, NULL,  0);

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
    shmctl(shared_placed, IPC_RMID, NULL);
    for(int i = 0; i < M; i++) {
        kill(loaders[i], SIGINT);
    }
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

    key_t key = ftok("/tmp", 'a');
    int semaphores_set = semget(key, 2, 0666);

    if(semaphores_set < 0) {
        printf("Execute trucker first!");
        return 1;
    }
    srand(time(NULL));
    int shared_line = shmget(key+1, 0, 0666);
    struct Line * line = (struct Line *) shmat (shared_line, NULL, 0);
    
    int shared_metadata = shmget(key+2, 0, 0666);
    struct Metadata * metadata = (struct Metadata *) shmat (shared_metadata, NULL,  0);

    int shared_placed = shmget(key+3, 0, 0666);
    int * placed = (int *) shmat (shared_placed, NULL,  0);

    struct sembuf sops;
    int sample_package = (rand()%N + 1);
    while(C && __exit2 == 0) {

        struct sembuf sops;
        sops.sem_flg = SEM_UNDO;
        sops.sem_num = 0;
        sops.sem_op = 0;

        semop(semaphores_set, &sops, 1);

        

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
                        sops.sem_num = 0;
                        sops.sem_op = 1;
                        semop(semaphores_set, &sops, 1);
                    } else {
                        sops.sem_num = 1;
                        sops.sem_op = 1;
                        semop(semaphores_set, &sops, 1);
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
                sops.sem_num = 0;
                sops.sem_op = 1;
                semop(semaphores_set, &sops, 1);
            } else {
                sops.sem_num = 1;
                sops.sem_op = 1;
                semop(semaphores_set, &sops, 1);
            }
        }

        if(C > 0) C--;
    }

    shmdt((void *) line);
    shmdt((void *) metadata);
    shmdt((void *) placed);
    exit(0);
}