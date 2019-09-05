#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
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

int main(int argc, char ** argv) {
    if(argc != 2) return -1;
    FILE * file;
    file = fopen(argv[1], "r");
    int fd_s[2], fd_n[2];
    char *args[5][11];
    char line[200];
    int i = 0;
    while (fgets(line, 200*sizeof(char), file) != NULL) {
        char *p = strtok (line, " ");
        
        int j = 0;
        while (p != NULL)
        {
             
            if(j == 10) return -2;
            if(p[0] == '|' && p[1] == '\0') {
                args[i][j] = NULL;
                i++;
                j = 0;
            } else {
                args[i][j] = p;
                j++;
            }
            
            p = strtok (NULL, " \n");           
        }
        args[i][j] = NULL;
        pid_t pid[i];
        for(int k = 0; k <= i; k++) {
            pipe(fd_n);
            
            
            pid[k] = fork();
            if(pid[k] != 0) {
                if(k > 0){
                    dup2(fd_s[0],STDIN_FILENO);
                    close(fd_s[1]);
                    close(fd_s[0]);
                }
                if(k < i){
                    dup2(fd_n[1],STDOUT_FILENO);
                    close(fd_n[0]);
                    close(fd_n[1]);
                }
                execvp(args[k][0],args[k]);
                exit(0);
            } else {
                if(k > 0){
                    close(fd_s[0]);
                    close(fd_s[1]);
                }
                if(k < i){
                    fd_s[0] = fd_n[0];
                    fd_s[1] = fd_n[1];
                }
            }

        }
        int status;
        for(int pi = 0; pi < i; pi++) waitpid(pid[pi], &status, 0);
    }


    fclose(file);
    return 0;
}