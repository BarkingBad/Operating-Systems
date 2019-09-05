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

#define SIZE 30

static int friends_matrix[SIZE][SIZE];
static int client_list[SIZE];
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


void send_message_to_client(int client_id, struct Message msg){

  if(client_id < SIZE && client_list[client_id] != -1){
    printf("Sending message to queue: %d \n",client_list[client_id]);
    msgsnd(client_list[client_id], &msg, msg_size(),0);
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
  client_id = add_client(msg.id);
  printf("Received INIT from: %d\n",client_id);
  msg.mtype = INIT;
  msg.id = client_id;
  send_message_to_client(client_id, msg);
}

void list(struct Message msg){
  short client_id = msg.id;
  printf("Received LIST from: %d\n",msg.id);
  list_clients(&msg);
  printf("%s\n",msg.msg );
  msg.mtype = LIST;
  send_message_to_client(client_id, msg);
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
      send_message_to_client(i, msg);

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
      send_message_to_client(i, msg);
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

void echo(struct Message msg){
  char buff[100];
  time_t curr_time;
  time(&curr_time);
  printf("Received ECHO from: %d\n",msg.id);
  strftime(buff, 100, "%Y-%m-%d_%H-%M-%S ", localtime(&curr_time));
  strcat(buff,msg.msg);
  strcpy(msg.msg,buff);
  send_message_to_client(msg.id, msg);
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
  send_message_to_client(client_id, msg);
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
    return msgget(ftok(getenv("HOME"), 's'), 0777 | IPC_CREAT);
}

void handler(){
    __exit = 1;
}

int main(int argc, char** argv){

    srand(time(NULL));
    setbuf(stdout, NULL);

    signal(SIGINT, handler);
    struct Message msg;
    set_up_client_list();
    int queue_id = set_up_server_queue_id();

    while(1){

        if(msgrcv(queue_id, &msg, msg_size(),-20,IPC_NOWAIT) != -1) {
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
                msgsnd(client_list[i], &msg, msg_size(), 0);
                }
            }

            if(queue_id != -1)
            msgctl(queue_id, IPC_RMID, NULL);
        }
    }

    return 0;
}
