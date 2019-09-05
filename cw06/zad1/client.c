#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ipc.h>
#include <limits.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include "header.h"
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#define SIZE 1024

int __exit = 0;

void delete_queue(int queue_id) {
  msgctl(queue_id, IPC_RMID, NULL);
}

void send_message(int server_queue_id, struct Message msg){
    msgsnd(server_queue_id, &msg, msg_size(), 0);
}

int set_up_server_queue_id(){
    return msgget(ftok(getenv("HOME"), 's'), 0777 | IPC_CREAT );
}

int set_up_own_queue(){
    return msgget(IPC_PRIVATE, 0777 | IPC_CREAT );
}

int init_mesg(int server_queue_id, int queue_id) {
    struct Message msg;
    msg.mtype = INIT;
    msg.id = queue_id;
    send_message(server_queue_id, msg);

    msgrcv(queue_id, &msg, msg_size(), 0, 0);

    if(msg.id == -1){
        exit(-1);
    }

    int id = msg.id;
    printf("RECEIVED ID %d\n",id );
    return id;
}

void exit_handle() {
    __exit = 1;
}

int main(int argc, char** argv) {
    signal(SIGINT, exit_handle);


    int server_queue_id = set_up_server_queue_id();
    int queue_id = set_up_own_queue();
    
    int id = init_mesg(server_queue_id, queue_id);
    int size = 256*sizeof(char);
    char buf[size];
    struct Message msg;
    int pip[2];
    pipe(pip);
    fcntl(pip[0], F_SETFL, O_NONBLOCK);
    pid_t pid = 1;
    char * tmp;
    pid = fork();
    while (1) {
        

        if(pid == 0) {
            read(STDIN_FILENO, buf, size);
            char * ptr = strdup(buf);
            tmp = ptr;
            char * token = strtok_r(ptr, " ", &ptr);

            if(strcmp(token,"READ") == 0) {

                ptr[strcspn(ptr, "\n")] = 0;

                FILE * f = fopen(ptr,"r");
                if(f == NULL) {
                    printf("FILE DOESNT EXIST\n");
                } else {
                    while (fgets(buf,sizeof(buf), f) != NULL) {
                        write(pip[1], buf, size);
                    }
                    fclose(f);
                }
                    
            } else {
                if(buf[0] != '\n')
                    write(pip[1], buf, size);
            }
        } else { 
            if(read(pip[0], buf, size) > 0) {
                char* the_rest = buf;
                char delimiters[3] = {' ','\n','\t'};
                char * token = strtok_r(the_rest,delimiters,&the_rest);
                msg.id = id;

                if(strcmp(token, "LIST") == 0 ){
                    token = strtok_r(the_rest, " ", &the_rest);
                    msg.mtype = LIST;
                    send_message(server_queue_id, msg);
                } else if(strcmp(token, "FRIENDS") == 0) {
                    if(the_rest == NULL){
                        msg.msg[0]='\0';
                    } else {
                        strcpy(msg.msg, the_rest);
                    }
                    msg.mtype = FRIENDS;
                    send_message(server_queue_id, msg);
                } else if(strcmp(token, "ADD") == 0) {
                    if(strcmp(the_rest, "\0") == 0 || strcmp(the_rest, "\n") == 0) {
                        printf("ADD COMMAND REQUIRES ARGUEMNTS\n");
                    } else {
                        strcpy(msg.msg, the_rest);
                        msg.mtype = ADD;
                        send_message(server_queue_id, msg);
                    }
                } else if(strcmp(token, "DEL") == 0) {
                    if(strcmp(the_rest, "\0") == 0 || strcmp(the_rest, "\n") == 0) {
                        printf("DEL COMMAND REQUIRES ARGUEMNTS\n");
                    } else {
                        msg.mtype = DEL;
                        strcpy(msg.msg, the_rest);
                        send_message(server_queue_id, msg);
                    }
                } else if(strcmp(token, "2ALL") == 0) {
                    msg.mtype = _2ALL;
                    strcpy(msg.msg, the_rest);
                    send_message(server_queue_id, msg);
                } else if(strcmp(token, "2FRIENDS") == 0) {
                    msg.mtype = _2FRIENDS;
                    strcpy(msg.msg, the_rest);
                    send_message(server_queue_id, msg);
                } else if(strcmp(token, "2ONE") == 0) {
                    msg.mtype = _2ONE;
                    strcpy(msg.msg, the_rest);
                    send_message(server_queue_id, msg);
                } else if(strcmp(token, "STOP") == 0) {
                    __exit = 1;
                } else if(strcmp(token, "ECHO") == 0) {
                    msg.mtype = ECHO;
                    strcpy(msg.msg, the_rest);
                    send_message(server_queue_id, msg);
                } else {
                    printf("MALFORMED INPUT: %s\n",buf);
                    printf("COMMANDS: [ECHO string] [LIST] [FRIENDS clients_id_list] [2ALL string] [2FRIENDS string] [2ONE client_id string] [STOP] \n");
                }
            }

            if( msgrcv(queue_id, &msg, msg_size(), -20, IPC_NOWAIT) != -1 ) {
                switch(msg.mtype) {
                    case LIST:
                        printf("LIST: %s\n", msg.msg);
                        break;

                    case ECHO:
                        printf("ECHO: %s", msg.msg);
                        break;

                    case _2ALL:
                        printf("2ALL: %s", msg.msg);
                        break;

                    case _2ONE:
                        printf("2ONE: %s", msg.msg);
                        break;

                    case _2FRIENDS:
                        printf("2FRIENDS: %s", msg.msg);
                        break;

                    case STOP:
                        break;
                }
            }
            if(__exit == 1) {
                printf("EXITING\n");
                msg.id = id;
                msg.mtype = STOP;
                send_message(server_queue_id, msg);
                delete_queue(queue_id);
                free(tmp);
                kill(pid, SIGKILL);
                return 0;
            }
        }
    }
    return 0;
}