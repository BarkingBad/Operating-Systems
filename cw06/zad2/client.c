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
#include <mqueue.h>

#define SIZE 1024

int __exit = 0;

void delete_queue(int queue_id, int server_queue_id, char * name) {
  mq_close(queue_id);
  mq_close(server_queue_id);
  mq_unlink(name);
}

void send_message(int server_queue_id, struct Message msg, int priority){
    mq_send(server_queue_id, (char *) &msg, sizeof(struct Message), priority);
}

mqd_t set_up_server_queue_id(){
    return mq_open(HOME, O_WRONLY | O_NONBLOCK );
}

mqd_t set_up_own_queue(char * name) {
    struct mq_attr attr;
    attr.mq_flags = O_NONBLOCK;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct Message);
    attr.mq_curmsgs = 0;
    mq_unlink(name);
    return mq_open(name, O_CREAT | O_NONBLOCK, 0777, &attr);
}

int init_mesg(mqd_t server_queue_id, mqd_t queue_id, char * name) {
    struct Message msg;
    msg.mtype = INIT;
    msg.id = queue_id;
    strcpy(msg.msg, name);
    send_message(server_queue_id, msg, 0);
    sleep(1);
    mq_receive(queue_id, (char *) &msg, sizeof(struct Message), NULL);

    if(msg.id == -1) {
        __exit = 1;
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

    char name[10];
    name[0] = '/';
    sprintf(name+1, "%d", getpid());
    mqd_t server_queue_id = set_up_server_queue_id();
    mqd_t queue_id = set_up_own_queue(name);
    int id = init_mesg(server_queue_id, queue_id, name);
    int size = 256;
    char buf1[size];
    char buf2[size];
    struct Message msg;
    int pip[2];
    pipe(pip);
    fcntl(pip[0], F_SETFL, O_NONBLOCK);
    pid_t pid = 1;
    char * tmp;
    
    pid = fork();
    while (1) {
        if(pid == 0) {
            read(STDIN_FILENO, buf1, size);
            char * ptr = strdup(buf1);
            tmp = ptr;
            char * token = strtok_r(ptr, " ", &ptr);

            if(strcmp(token,"READ") == 0) {

                ptr[strcspn(ptr, "\n")] = 0;

                FILE * f = fopen(ptr,"r");
                if(f == NULL) {
                    printf("FILE DOESNT EXIST\n");
                } else {
                    while (fgets(buf1, size, f) != NULL) {
                        write(pip[1], buf1, size);
                    }
                    fclose(f);
                }
                    
            } else {
                if(buf1[0] != '\n')
                    write(pip[1], buf1, size);
            }
        } else {  
            if(read(pip[0], buf2, size) > 0) {
                printf("%s", buf2);
                char* the_rest = buf2;
                char delimiters[3] = {' ','\n','\t'};
                char * token = strtok_r(the_rest,delimiters,&the_rest);
                msg.id = id;
                if(strcmp(token, "LIST") == 0 ){
                    token = strtok_r(the_rest, " ", &the_rest);
                    msg.mtype = LIST;
                    send_message(server_queue_id, msg, 1);
                } else if(strcmp(token, "FRIENDS") == 0) {
                    if(the_rest == NULL){
                        msg.msg[0]='\0';
                    } else {
                        strcpy(msg.msg, the_rest);
                    }
                    msg.mtype = FRIENDS;
                    send_message(server_queue_id, msg, 2);
                } else if(strcmp(token, "ADD") == 0) {
                    if(strcmp(the_rest, "\0") == 0 || strcmp(the_rest, "\n") == 0) {
                        printf("ADD COMMAND REQUIRES ARGUEMNTS\n");
                    } else {
                        strcpy(msg.msg, the_rest);
                        msg.mtype = ADD;
                        send_message(server_queue_id, msg, 3);
                    }
                } else if(strcmp(token, "DEL") == 0) {
                    if(strcmp(the_rest, "\0") == 0 || strcmp(the_rest, "\n") == 0) {
                        printf("DEL COMMAND REQUIRES ARGUEMNTS\n");
                    } else {
                        msg.mtype = DEL;
                        strcpy(msg.msg, the_rest);
                        send_message(server_queue_id, msg, 3);
                    }
                } else if(strcmp(token, "2ALL") == 0) {
                    msg.mtype = _2ALL;
                    strcpy(msg.msg, the_rest);
                    send_message(server_queue_id, msg, 3);
                } else if(strcmp(token, "2FRIENDS") == 0) {
                    msg.mtype = _2FRIENDS;
                    strcpy(msg.msg, the_rest);
                    send_message(server_queue_id, msg, 3);
                } else if(strcmp(token, "2ONE") == 0) {
                    msg.mtype = _2ONE;
                    strcpy(msg.msg, the_rest);
                    send_message(server_queue_id, msg, 3);
                } else if(strcmp(token, "STOP") == 0) {
                    __exit = 1;
                } else if(strcmp(token, "ECHO") == 0) {
                    msg.mtype = ECHO;
                    strcpy(msg.msg, the_rest); 
                    send_message(server_queue_id, msg, 3);
                } else {
                    printf("MALFORMED INPUT: %s\n", token);
                    printf("COMMANDS: [ECHO string] [LIST] [FRIENDS clients_id_list] [2ALL string] [2FRIENDS string] [2ONE client_id string] [STOP] \n");
                }
            }

            if( mq_receive(queue_id, (char *) &msg, sizeof(struct Message), NULL) >= 0 ) {
                switch(msg.mtype) {
                    case LIST:
                        printf("LIST: %s\n", msg.msg);
                        break;

                    case ECHO:
                        printf("ECHO: %s", msg.msg);
                        break;            printf("%d %d %d %d", pip[0], pip[1], server_queue_id, queue_id);


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
                        __exit = 1;
                        break;
                }
            }
            if(__exit == 1) {
                printf("EXITING\n");
                msg.id = id;
                msg.mtype = STOP;
                send_message(server_queue_id, msg, 0);
                delete_queue(queue_id, server_queue_id, name);
                free(tmp);
                kill(pid, SIGKILL);
                return 0;
            }
        }
    }
    return 0;
}