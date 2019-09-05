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

const char format[] = "_%Y-%m-%d_%H-%M-%S";

struct Block {
    char * table;
    int size;
    time_t mod;
} Block;

int loop = 1;
int sleep_flag = 0;

void copy(struct Block * block, char * destination) {
    FILE * fd_d;
    char dest[100] = "./archiwum/";
    strcat(dest, destination);
    fd_d = fopen(dest, "w+");
    if(fd_d < 0) {
        printf("Error during file creation");
        return;
    } else {
        fwrite(block->table, sizeof(char), block->size, fd_d);
    }
    fclose(fd_d);
}


struct Block * copy_result_to_memory(char * filename) {
    FILE *fp;
    int size;
    struct Block * block = (struct Block *) malloc (sizeof(Block));
    fp = fopen(filename, "r");
    if(fp == NULL) return NULL;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    block->table = (char*) calloc (size, sizeof(char));
    block->size = size;
    fread(block->table, sizeof(char), size, fp);
    fclose(fp);
    return block;
}

void free_memory(struct Block *block) {
    free(block->table);
    free(block);
}

void handle_sigterm(int signum) {
    loop = 0;
}

void handle_sleep(int signum) {
    sleep_flag = 1;
}

void handle_wakeup(int signum) {
    sleep_flag = 0;
}

int child_process_in_memory(FILE * file, struct stat attr, char * filename, int seconds) {
    signal(15, handle_sigterm);
    signal(10, handle_sleep);
    signal(12, handle_wakeup);
    struct Block * block = copy_result_to_memory(filename);
    stat(filename, &attr);
    block->mod = attr.st_mtime;
    int count = 0, j = 0;
    char * tmpfilename = (char *) calloc (100, sizeof(char));
    char * str = (char *) calloc (100, sizeof(char));
    while(loop) {
        if(!sleep_flag) {
            sleep(seconds);
            count++;
            stat(filename, &attr);
            if(attr.st_mtime > block->mod) {
                strcpy(tmpfilename, basename(filename));
                strftime(str, 100, format, localtime(&block->mod));
                strcat(tmpfilename, str);
                copy(block, tmpfilename);
                free_memory(block);
                block = copy_result_to_memory(filename);
                block->mod = attr.st_mtime;
                j++;
            }  
        }
    }
    free_memory(block);
    free(filename);
    free(str);
    free(tmpfilename);
    fclose(file);
    exit(j);
}

void print_raport(pid_t pid[], int count, char * filename, FILE * file) {
    int status;
    for(int i = 0; i < count; i++) {
        kill(pid[i], 15);
        waitpid(pid[i], &status, 0);
        printf("Proces %d utworzyl %d kopii pliku\n", pid[i], (status/256));
    }
    free(filename); 
    fclose(file);
    exit(0);
}

void list(pid_t pid[], int is_running[], int count) {
    for(int i = 0; i<count; i++) {
        if(is_running[i] == 1) {
            printf("Proces %d obserwuje\n", pid[i]);
        }
    }
}

void start(pid_t pid) {
    kill(pid, 12);
}

void stop(pid_t pid) {
    kill(pid, 10);
}

int main(int argc, char ** argv) {
    FILE * file;
    file = fopen(argv[1], "r");
    if(file == NULL) return -1;
    if(argc != 2) return -1;

    char * filename = (char *) calloc (100, sizeof(char));
    int seconds;
    char c;
    int count = 0;
    int i = 0;
    struct stat attr;

    pid_t pid[1000];
    int is_running[1000];

    while(fscanf(file,"%s %d", filename, &seconds)>0) {
        
        is_running[i] = 1;
        pid[i] = fork();
        if(pid[i] == 0 ) {
            
            child_process_in_memory(file, attr, filename, seconds);
        } else {
            printf("Proces %d obserwuje plik %s\n", pid[i], filename);
        }
        i++;
        count++;
    }

    char command[100];
    char parameter[100];
    while(1 == 1) {
        scanf("%s", command);
        char num[10];
        int value;
        if (strcmp(command, "LIST") == 0) {
            list(pid, is_running, count);
        } else if (strcmp(command, "STOP") == 0) {
            scanf("%s", parameter);
            if(strcmp(parameter, "ALL") == 0) {
                for(int i=0; i<count; i++) {
                    is_running[i] = 0;
                    stop(pid[i]);
                }
            } else {
                sscanf(parameter, "%d", &value);
                stop(value);
                for(int i=0; i<count; i++) {
                    if(pid[i] == value) is_running[i] = 0;
                }
            }
        } else if (strcmp(command, "START") == 0) {
            scanf("%s", parameter);
            if(strcmp(parameter, "ALL") == 0) {
                for(int i=0; i<count; i++) {
                    is_running[i] = 1;
                    start(pid[i]);
                }
            } else {
                sscanf(parameter, "%d", &value);
                start(value);
                for(int i=0; i<count; i++) {
                    if(pid[i] == value) is_running[i] = 1;
                }
            }
        } else if (strcmp(command, "END") == 0) {
            print_raport(pid, count, filename, file);
        } else {
            printf("Not supported operation!\n");
        }
    }

    return 0;
}