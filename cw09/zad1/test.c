#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

volatile unsigned int cur_trolley = 0;
unsigned int trolley_no;
unsigned int passenger_no;
volatile unsigned int capacity;
volatile unsigned int cur_capacity;
int n = 0;


pthread_mutex_t * trolley_mutex = NULL;
pthread_mutex_t load_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t empty_mutex = PTHREAD_MUTEX_INITIALIZER;

int * trolley_arg = NULL;
int * passenger_arg = NULL;
pthread_t *trolley_thread;
pthread_t *passenger_thread;

pthread_cond_t empty_cond;
pthread_cond_t* load_cond = NULL;
pthread_mutex_t load_to_trolley;

void exit_fun(){
  if(trolley_mutex != NULL){
    for(int i = 0 ; i < trolley_no ; ++i)
      pthread_mutex_destroy(&trolley_mutex[i]);

    free(trolley_mutex);
  }

  pthread_mutex_destroy(&load_mutex);
  pthread_mutex_destroy(&empty_mutex);


  if(trolley_arg!=NULL){
    free(trolley_arg);
  }

  if(passenger_arg!=NULL){
    free(passenger_arg);
  }

  if(trolley_thread!=NULL){
    free(trolley_thread);
  }

  if(passenger_thread!=NULL){
    free(passenger_thread);
  }

  if(load_cond != NULL){
    free(load_cond);
  }
}


void init_dynamic_memory(){
  if((trolley_mutex = (pthread_mutex_t*)malloc(trolley_no*sizeof(pthread_mutex_t))) == NULL){
    exit(1);
  }

    for( int i = 0 ; i < trolley_no ; ++i){
      pthread_mutex_init(&trolley_mutex[i],NULL);
    }

    if((trolley_thread = (pthread_t *)malloc(trolley_no*sizeof(pthread_t))) == NULL){
      exit(1);
    }

    if((passenger_thread = (pthread_t*)malloc(passenger_no*sizeof(pthread_t))) == NULL){
      exit(1);
    }

    if((trolley_arg = (int *)malloc(trolley_no*sizeof(int))) == NULL){
      exit(1);
    }

    if((passenger_arg = (int *)malloc(passenger_no*sizeof(int))) == NULL){
      exit(1);
    }

    if((load_cond = (pthread_cond_t *)malloc(trolley_no * sizeof(pthread_cond_t))) == NULL){
      exit(1);
    }
}


void * trolley_fun(void* arg){
  int no = *(int*)arg;
  if(no == 0){
    pthread_mutex_lock(&load_to_trolley);
  }

  for(int i = 0 ; i < n ; ++i){
    pthread_mutex_lock(&load_mutex);

    if( no != cur_trolley){
      pthread_cond_wait(&load_cond[no],&load_mutex);
    }
    /***/

    printf("TROLLEY %d ARRIVES\n",no );
    fflush(stdout);

    //unload the trolley
    if( i != 0){
      cur_capacity = capacity;
      cur_trolley=no;

      pthread_mutex_unlock(&trolley_mutex[no]);
      //the last one should
      pthread_cond_wait(&empty_cond,&empty_mutex);
    }

    printf("TROLLEY %d STARTS LOADING PASSENGERS\n",no );
    fflush(stdout);


    //load the trolley last passenger will own it
    pthread_mutex_lock(&trolley_mutex[no]);
    pthread_mutex_unlock(&load_to_trolley);



    //;ast passener should give a notice
    pthread_cond_wait(&empty_cond,&empty_mutex);
    //closing the doors


    ++cur_trolley;
    cur_trolley%=trolley_no;


    cur_capacity=0;
    printf("TROLLEY %d DEPARTURE\n",no );
    fflush(stdout);

    pthread_cond_signal(&load_cond[cur_trolley]);
    pthread_mutex_unlock(&load_mutex);

    //weeeeeee
    usleep(10000);
  }
  //unload
  pthread_mutex_lock(&load_mutex);

  if( no != cur_trolley){
    pthread_cond_wait(&load_cond[no],&load_mutex);
  }
  /***/

  printf("TROLLEY %d ARRIVES\n",no );
  fflush(stdout);
  //unload the trolley

  cur_capacity = capacity;
  cur_trolley=no;

  pthread_mutex_unlock(&trolley_mutex[no]);
  //the last one should
  pthread_cond_wait(&empty_cond,&empty_mutex);

  ++cur_trolley;
  cur_trolley%=trolley_no;


  printf("TROLLEY %d ENDS HIS SHIFT\n",no );
  fflush(stdout);
  pthread_cond_signal(&load_cond[cur_trolley]);
  pthread_mutex_unlock(&load_mutex);


  //end of shift

  return NULL;
}

void * passenger_fun(void* arg){
  int no = *(int*)arg;

  while(1){
    //wait for room
    pthread_mutex_lock(&load_to_trolley);
    //pack
    printf("PASSENGER %d LOADING\n",no );
    fflush(stdout);
    ++cur_capacity;

    if(cur_capacity == capacity){
      printf("WAS LAST PASSENGER %d\n",no );
      fflush(stdout);
      pthread_cond_signal(&empty_cond);
      pthread_mutex_unlock(&empty_mutex);
    }else{
      pthread_mutex_unlock(&load_to_trolley);
    }

    pthread_mutex_lock(&trolley_mutex[cur_trolley]);
    //unpack

    printf("UNPACKING %d %d\n",no,cur_capacity);
    fflush(stdout);
    if(--cur_capacity == 0){
      pthread_cond_signal(&empty_cond);
      pthread_mutex_unlock(&empty_mutex);
    }
    pthread_mutex_unlock(&trolley_mutex[cur_trolley]);
    usleep(1000);
  }

  return NULL;
}

void init_trolley(){
  for( int i = 0 ; i < trolley_no ; ++i){
    trolley_arg[i] = i;
    pthread_create(&trolley_thread[i],NULL,trolley_fun,(void*)&trolley_arg[i]);
  }
}

void init_passengers(){
  for( int i = 0 ; i < passenger_no ; ++i){
    passenger_arg[i] = i;
    pthread_create(&passenger_thread[i],NULL,passenger_fun,(void*)&passenger_arg[i]);
  }
}

void wait_for_trolley(){
  for(int i = 0 ; i < trolley_no ; ++i){
    pthread_join(trolley_thread[i],NULL);
  }
}

int main(int argc, char** argv){
  trolley_no = 4;
  passenger_no = 9;
  capacity = 2;
  n = 3;

  init_dynamic_memory();
  init_trolley();
  init_passengers();

  wait_for_trolley();

  return 0;
}