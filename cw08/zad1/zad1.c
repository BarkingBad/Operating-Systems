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
#include <errno.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#define BLOCK 0
#define INTERLEAVED 1

short ** inputArray;
double ** filterArray;
short ** outputArray;

struct Arguments{
    int id;
    int max;
    int mode;
    struct timeval tv;
    int H;
    int W;
    int c;
};

double diff(struct timeval start , struct timeval end){
    double start_ms, end_ms;

    start_ms = (double)start.tv_sec*1000000 + (double)start.tv_usec;
    end_ms = (double)end.tv_sec*1000000 + (double)end.tv_usec;

    return (double)end_ms - (double)start_ms;
}

int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a > b ? b : a;
}

double s(int x, int y, int c) {
    double acc = 0.0;
    for(int i = 0; i < c; i++) {
        for(int j = 0; j < c; j++) {
            acc += (inputArray[min(x, max(1, x - ceil(c/2) + floor(i/2)))][min(y, max(1, y - ceil(c/2) + floor(j/2)))] * filterArray[i][j]);
        }
    }
}

void *calculate(void *arguments) {
    struct Arguments * args = arguments;
    
    for(int i = 0; i < args->H; i++) {
        for(int j = 0; j < args->W; j++) {
            if(args->mode == INTERLEAVED && j % args->max != args->id) continue;
            if(args->mode == BLOCK && (j < ceil(args->W/args->max) * args->id || j >= ceil(args->W/args->max) * (args->id + 1))) continue;
            outputArray[i][j] = round(s(i, j, args->c));
        }
    }


    struct timeval tv;
    gettimeofday(&tv, NULL);
    double * result = (double *) malloc (sizeof(double));
    *result = diff(args->tv, tv);
    pthread_exit((void *) result);
}

int main(int argc, char * argv[]) {

    if(argc != 6) {
        printf("Wrong arguments count");
        return 1;
    }

    int threads_count, flag;
    FILE * input;
    FILE * output;
    FILE * filter;

    if(sscanf(argv[1], "%d", &threads_count) < 0) {
        printf("Wrong argument: thread count!");
        return 1;
    }
    if(strcmp(argv[2], "block") == 0) {
        flag = BLOCK;
    } else if(strcmp(argv[2], "interleaved") == 0) {
        flag = INTERLEAVED;
    } else {
        printf("Wrong argument: flag! Should be block or interleaved");
        return 1;
    }
    if((input = fopen(argv[3], "r")) == NULL) {
        printf("Wrong argument: input!");
        return 1;
    }
    if((output = fopen(argv[4], "w+")) == NULL) {
        printf("Wrong argument: output!");
        return 1;
    }
    if((filter = fopen(argv[5], "r")) == NULL) {
        printf("Wrong argument: filter!");
        return 1;
    }

    char buffer[71];
    int W, H, c;
    fgets(buffer, sizeof(buffer), input);
    fgets(buffer, sizeof(buffer), input);
    sscanf(buffer, "%d %d", &W, &H);
    fgets(buffer, sizeof(buffer), input);
    inputArray = (short **) malloc (sizeof(short *) * H);
    outputArray = (short **) malloc (sizeof(short *) * H);
    for(int i = 0; i < H; i++) {
        inputArray[i] = (short *) malloc (sizeof(short) * W);
        outputArray[i] = (short *) malloc (sizeof(short) * W);
    }
    fgets(buffer, sizeof(buffer), filter);
    sscanf(buffer, "%d", &c);
    filterArray = (double **) malloc (sizeof(double *) * c);
    for(int i = 0; i < c; i++) {
        filterArray[i] = (double *) malloc (sizeof(double) * c);
    }

    char delim[] = " ";
    int iterator = 0;
    int con = 0;
    while(fgets(buffer, sizeof(buffer), input)) {
        char *ptr = strtok(buffer, delim);
        while(ptr != NULL)
        {   
            if(iterator == W*H+1) break;
            if(sscanf(ptr, "%hd", &inputArray[iterator/W][iterator%W]) > 0) {
                iterator++;
            }
            ptr = strtok(NULL, delim);
        }
    }
    fclose(input);
    
    iterator = 0;
    while(fgets(buffer, sizeof(buffer), filter)) {
        sscanf(buffer, "%lf", &filterArray[iterator/c][iterator%c]);
        iterator++;
    }
    fclose(filter);

    struct Arguments * args[threads_count];

    pthread_t threads[threads_count];
    for(int i = 0; i < threads_count; i++) {
        args[i] = (struct Arguments *) malloc (sizeof(struct Arguments));
        args[i]->id = i;
        args[i]->max = threads_count;
        args[i]->mode = flag;
        gettimeofday(&args[i]->tv, NULL);
        args[i]->W = W;
        args[i]->H = H;
        args[i]->c = c;
        pthread_create(&threads[i], NULL, &calculate, (void *) args[i]);
    }
    
    double * result;
    for(int i = 0; i < threads_count; i++) {
        pthread_join(threads[i], (void **) &result);
        printf("%d %f\n", i, *result);
    }
    sprintf(buffer, "P2\n%d %d\n%d\n", W, H, 255);
    fputs(buffer, output);

    for(int i = 0; i < H; i++) {
        for(int j = 0; j < W; j++) {
            sprintf(buffer, "%hd\n", outputArray[i][j]);
            fputs(buffer, output);
        }
    }
    fclose(output);

    for(int i = 0; i < H; i++) {
        free(inputArray[i]);
        free(outputArray[i]);
    }

    for(int i = 0; i < c; i++) {
        free(filterArray[i]);
    }

    free(filterArray);
    free(inputArray);
    free(outputArray);
    for(int i = 0; i < threads_count; i++) {
        free(args[i]);
    }
    

    return 0;
}