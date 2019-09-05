#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <limits.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include "header.h"
#include <string.h>
#include <mqueue.h>

#define SIZE 30

static int friends_matrix[SIZE][SIZE];
static mqd_t client_list[SIZE];
int __exit = 0;

void delete_client_friends_list(int client_id){

  if(client_id>=0 && client_id < SIZE)
    for(int i = 0 ; i < SIZE ; ++i)
      friends_matrix[client_id][i] = 0;
}


void print_friend_list(int client_id){
  printf("Your friends: %d\n",client_id );

  for(int i = 0 ; i < SIZE ; ++i){
    if(friends_matrix[client_id][i]==1)
      printf("%d ",i);
    }

    printf("\n");
}

void list_clients(struct Message * msg){
  char list[MSGMAX];
  list[0] = '\0';
  char buff[14];

  for(int i = 0 ; i < SIZE; ++i)
    if(client_list[i] != -1){
      sprintf(buff,"%d ",i);
      strcat(list,buff);
    }

  strcpy(msg->msg, list);
}


void send_message_to_client(int client_id, struct Message msg, int priority){

  if(client_id < SIZE && client_list[client_id] != -1){
    printf("Sending message to queue: %d \n",client_list[client_id]);
    mq_send(client_list[client_id], (char *) &msg, sizeof(struct Message), priority);
  }
}


int add_client(int queue_id){
  for( int i = 0 ; i < SIZE ; ++i)
    if(client_list[i] == -1){
      client_list[i]=queue_id;
      return i;
    }

  return -1;
}


void add_to_friend_list(int client_id,char* str){

  char * ptr = str;
  char * token = strtok_r(ptr," ",&ptr);
  while(ptr!=NULL && token!=NULL ){
    if(strcmp(token,"\0")!= 0 && strcmp(token,"\n")!=0){
      int index = strtol(token,NULL,10);
      if(index >=0 && index < SIZE){
        friends_matrix[client_id][index] = 1;
      }
    }
    token = strtok_r(ptr," ",&ptr);
  }
}

void delete_from_friend_list(int client_id, char* str){
  char * ptr = str;
  char * token = strtok_r(ptr," ",&ptr);

  while(ptr!=NULL && token!=NULL ){
    if(strcmp(token,"\0")!= 0 && strcmp(token,"\n")!=0){
      int index = strtol(token,NULL,10);

      if(index >=0 && index < SIZE)
        friends_matrix[client_id][index] = 0;
      }
    token = strtok_r(ptr," ",&ptr);
  }
}

void init(struct Message msg){
  short client_id;
  mqd_t queue = mq_open(msg.msg, O_WRONLY);
  client_id = add_client(queue);
  printf("Received INIT from: %d\n",client_id);
  msg.mtype = INIT;
  msg.id = client_id;
  send_message_to_client(client_id, msg, 0);
}

void list(struct Message msg){
  short client_id = msg.id;
  printf("Received LIST from: %d\n",msg.id);
  list_clients(&msg);
  printf("%s\n",msg.msg );
  msg.mtype = LIST;
  send_message_to_client(client_id, msg, 1);
}

void all2(struct Message msg){
  char buff[100];
  char buff2[14];
  printf("Received 2ALL from: %d\n",msg.id);
  sprintf(buff2," ID: %d :: ", msg.id);

  time_t curr_time;
  time(&curr_time);
  strftime(buff, 100, "%Y-%m-%d_%H-%M-%S ", localtime(&curr_time));

  strcat(buff,buff2);
  strcat(buff,msg.msg);
  strcpy(msg.msg,buff);

  for(int i = 0 ; i < SIZE ; ++i)
    if(client_list[i]!= -1)
      send_message_to_client(i, msg, 3);

}

void friends2(struct Message msg){
  char buff[100];
  char buff2[14];
  sprintf(buff2," ID: %d :: ", msg.id);
  printf("Received 2FRIENDS from: %d\n",msg.id);
  time_t curr_time;
  time(&curr_time);
  strftime(buff, 100, "%Y-%m-%d_%H-%M-%S ", localtime(&curr_time));

  strcat(buff,buff2);
  strcat(buff,msg.msg);
  strcpy(msg.msg,buff);

  for(int i = 0 ; i < SIZE ; ++i)
    if(friends_matrix[msg.id][i])
      send_message_to_client(i, msg, 3);
}


void add(struct Message msg){
  printf("Received ADD from: \n");
  add_to_friend_list(msg.id, msg.msg);
  print_friend_list(msg.id);
}

void del(struct Message msg){
  printf("Received DELETE from: \n");
  delete_from_friend_list(msg.id,msg.msg);
  print_friend_list(msg.id);
}

void stop(struct Message msg){
  printf("Received STOP from: %d\n",msg.id);
  delete_client_friends_list(msg.id);
  client_list[msg.id]=-1;
}

void echo(struct Message msg) {
  char buff[100];
  time_t curr_time;
  time(&curr_time);
  printf("Received ECHO from: %d\n",msg.id);
  strftime(buff, 100, "%Y-%m-%d_%H-%M-%S ", localtime(&curr_time));
  strcat(buff,msg.msg);
  strcpy(msg.msg,buff);
  send_message_to_client(msg.id, msg, 3);
}

void one2(struct Message msg){
  char buff[100];
  char buff2[14];
  time_t curr_time;
  time(&curr_time);
  printf("Received 2ONE from: %d\n",msg.id);
  strftime(buff, 100, "%Y-%m-%d_%H-%M-%S ", localtime(&curr_time));
  sprintf(buff2," ID: %d :: ", msg.id);
  char* ptr = msg.msg;
  char* client = strtok_r(ptr," ",&ptr);
  short client_id = strtol(client,NULL,10);
  strcat(buff,buff2);
  strcat(buff,ptr);
  strcpy(msg.msg,buff);
  send_message_to_client(client_id, msg, 3);
}

void friends(struct Message msg){
  delete_client_friends_list(msg.id);
  if(msg.msg[0] != '\0'){
    add_to_friend_list(msg.id, msg.msg);
  }
  print_friend_list(msg.id);
}


void check_client_list(struct Message msg){
  for( int i = 0 ; i < SIZE ; ++i)
    printf("%d %d\n",i,client_list[i] );
}

void set_up_client_list(){
  for( int i = 0 ; i < SIZE ; ++i){
    client_list[i]= -1;

    for(int j = 0 ; j < SIZE ; ++j)
      friends_matrix[i][j]=0;
  }
}


int set_up_server_queue_id() {
    struct mq_attr attr;
    attr.mq_flags = O_NONBLOCK;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct Message);
    attr.mq_curmsgs = 0;
    mq_unlink(HOME);
    return mq_open(HOME, O_CREAT | O_NONBLOCK, 0777, &attr);
}

void handler() {
    __exit = 1;
}

int main(int argc, char** argv){

    srand(time(NULL));
    signal(SIGINT, handler);
    struct Message msg;
    set_up_client_list();
    setbuf(stdout, NULL);
    mqd_t queue_id = set_up_server_queue_id();
    while(1) {
        if(mq_receive(queue_id, (char *) &msg, sizeof(struct Message), NULL) >= 0) {
            switch (msg.mtype) {
                case INIT:
                    init(msg);
                    break;

                case LIST:
                    list(msg);
                    break;

                case ECHO:
                    echo(msg);
                    break;

                case _2ALL:
                    all2(msg);
                    break;

                case _2FRIENDS:
                    friends2(msg);
                    break;

                case _2ONE:
                    one2(msg);
                    break;

                case FRIENDS:
                    friends(msg);
                    break;

                case STOP:
                    stop(msg);
                    break;

                case ADD:
                    add(msg);
                    break;

                case DEL:
                    del(msg);
                    break;

                default:
                    break;
            }
        }
        
        if(__exit == 1) {
            msg.mtype = STOP;

            for(int i = 0 ; i < SIZE ; ++i){
                if(client_list[i]!= -1){
                  mq_send(client_list[i], (char *) &msg, sizeof(struct Message), 0);
                }
            }

            if(queue_id != -1)
              mq_close(queue_id);
              mq_unlink(HOME);
            
            return 0;
        }
    }

    return 0;
}
