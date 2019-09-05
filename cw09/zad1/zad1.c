#define _OPEN_THREADS
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

int riders_count, carts_count, c, n;
int rides_left;
int current_loading = 0;
int *cart_current_size;
int current_unloading = 0;
pthread_t *riders_threads;
pthread_t *carts_threads;
int loading_state = 0; 
int unloading_state = 0;
int carts_count_tmp;

pthread_mutex_t load_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t can_load_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t can_ride_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t can_let_in_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t can_unload_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t unload_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_load_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_enter_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_leave_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_ride_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_let_in_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_unload_condition = PTHREAD_COND_INITIALIZER;

void * cart(void * args) {
    int id = *((int *)args);
    while(1) {
        
        pthread_mutex_lock(&load_mutex);
        if(rides_left == 0) {
            break;
        } 
        while(id != current_loading) {
            pthread_cond_wait(&can_load_condition, &load_mutex);
        }
        printf("Cart %d arrives and opens doors\n", id);
        loading_state = 1;
        pthread_cond_broadcast(&can_enter_condition);
        while(cart_current_size[id] < c){
            pthread_cond_wait(&can_ride_condition, &can_ride_mutex);
        }
        if(current_loading == carts_count-1) rides_left--;
        current_loading = (current_loading+1)%carts_count;
        printf("Cart %d closes doors. Departures...\n", id);
        pthread_mutex_unlock(&load_mutex);
        pthread_cond_broadcast(&can_load_condition);
        usleep(5);
        pthread_mutex_lock(&unload_mutex);
        while(id != current_unloading) {
            pthread_cond_wait(&can_unload_condition, &unload_mutex);
        }
        printf("Cart %d finishes ride and opens doors.\n", id);
        unloading_state = 1;
        pthread_cond_broadcast(&can_leave_condition);
        while(cart_current_size[id] > 0){
            pthread_cond_wait(&can_let_in_condition, &can_let_in_mutex);
        }
        current_unloading = (current_unloading+1)%carts_count;
        printf("Cart %d closes doors. Moves slowly to loading point...\n", id);
        pthread_mutex_unlock(&unload_mutex);
        pthread_cond_broadcast(&can_unload_condition);
    }
    printf("Cart %d ends duty\n", id);
    pthread_mutex_unlock(&load_mutex);
    pthread_exit(NULL);
}

void * rider(void * args) {
    int id = *((int *)args);
    int cart = -1;


    while(1) {

        pthread_mutex_lock(&can_load_mutex);
        while(loading_state < 1) {
            pthread_cond_wait(&can_enter_condition, &can_load_mutex);
        }

        cart = current_loading;
        cart_current_size[cart]++;
        printf("Rider %d enters the cart %d. %d/%d \n", id, cart, cart_current_size[cart], c);
        if(cart_current_size[cart] < c) {
            pthread_mutex_unlock(&can_load_mutex);
            pthread_cond_broadcast(&can_enter_condition);
        } else {
            loading_state = 0;
            printf("Start button was pressed by %d\n", id);
            pthread_mutex_unlock(&can_load_mutex);
            pthread_cond_broadcast(&can_ride_condition);
        }

        pthread_mutex_lock(&can_unload_mutex);
        while(unloading_state < 1 || cart != current_unloading) {
            
            pthread_cond_wait(&can_leave_condition, &can_unload_mutex);
        }

            
        cart_current_size[cart]--;
        printf("Rider %d leaves the cart %d. %d/%d \n", id, cart, cart_current_size[cart], c);
        if(cart_current_size[cart] > 0) {
            cart = -1;
            pthread_mutex_unlock(&can_unload_mutex);
            pthread_cond_broadcast(&can_leave_condition);
        } else {
            cart = -1;
            unloading_state = 0;
            pthread_mutex_unlock(&can_unload_mutex);
            pthread_cond_broadcast(&can_let_in_condition);
        }

    }
}

int main(int argc, char * argv[]) {
    if(argc != 5) {
        printf("Wrong arguments count");
        return 1;
    }
    if(sscanf(argv[1], "%d", &riders_count) < 0) {
        printf("Wrong argument: riders count!");
        return 1;
    }
    if(sscanf(argv[2], "%d", &carts_count) < 0) {
        printf("Wrong argument: carts count!");
        return 1;
    }
    if(sscanf(argv[3], "%d", &c) < 0) {
        printf("Wrong argument: cart size!");
        return 1;
    }
    if(sscanf(argv[4], "%d", &n) < 0) {
        printf("Wrong argument: rides count!");
        return 1;
    }
    carts_count_tmp = carts_count;
    carts_threads = malloc(sizeof(pthread_t)*carts_count);
    riders_threads = malloc(sizeof(pthread_t)*riders_count);
    rides_left = n;
    int *carts = malloc(sizeof(int) * carts_count);
    int *riders = malloc(sizeof(int) * riders_count);
    cart_current_size = malloc(sizeof(int) * carts_count);

    for(int i = 0; i < carts_count; i++){
        carts[i] = i;
        cart_current_size[i] = 0;
        pthread_create(&carts_threads[i], NULL, cart, (void*)&carts[i]);
    }
    
    for(int i = 0; i < riders_count; i++){
        riders[i] = i;
        pthread_create(&riders_threads[i], NULL, rider, (void*)&riders[i]);
    }

    for(int i = 0; i < carts_count; i++){
        pthread_join(carts_threads[i], NULL);
    }

    for(int i = 0; i < riders_count; i++){
        pthread_cancel(riders_threads[i]);
        printf("Rider %d ends duty\n", i);
    }

    free(carts_threads);
    free(riders_threads);
    free(carts);
    free(riders);
    free(cart_current_size);

    return 0;
}