#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <time.h>


void generate(char * filename, int rows_count, int byte_size) {
    if(filename == NULL || rows_count <= 0 || byte_size <= 0) {
        printf("Unsupported operation. Closing program!\n");
        return;        
    }
    int fd;
    srand(time(NULL));
    fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC);
    if(fd < 0) {
        printf("Error during file creation");
        return;
    } else {
        int i;
        char * str = malloc(byte_size * sizeof(char));
        char * ptr = str;
        for(i = 0; i < rows_count; i++) {
            str = ptr;
            int j;
            for(j = 0; j < byte_size; j++) {
                str[j] = rand() % 26 + 97;
            }
            //str[byte_size - 1] = '\n';
            if(write(fd, ptr, byte_size) < 0) {
                printf("Error during file writing");
                return;
            }
        }
        free(str);
    }
    close(fd);
    return;
}



void sort_sys(char * filename, int rows_count, int byte_size) {

    int i;
    int j;
    int fd;
    char * potential_min = malloc(sizeof(char));
    
    int position_of_current_min = 0;
    char * current_min = malloc(sizeof(char));
    char * ptr1 = malloc(byte_size * sizeof(char));
    char * ptr2 = malloc(byte_size * sizeof(char));
    fd = open(filename, O_RDWR);
    if(fd < 0) {
        printf("Error during file creation");
        return;
    } else {
        for(i = 0; i < rows_count; i++) {
            lseek(fd, byte_size*i, SEEK_SET);
            position_of_current_min = i;
            read(fd, current_min, 1);
            for(j = i + 1; j < rows_count; j++) {
                lseek(fd, byte_size-1, SEEK_CUR);
                read(fd, potential_min, 1);
                if(current_min[0] > potential_min[0]) {
                    *current_min = *potential_min;
                    position_of_current_min = j;
                }
            }
            lseek(fd, byte_size*i, SEEK_SET);
            read(fd, ptr1, byte_size);
            lseek(fd, byte_size*position_of_current_min, SEEK_SET);
            read(fd, ptr2, byte_size);
            lseek(fd, byte_size*i, SEEK_SET);
            write(fd, ptr2, byte_size);
            lseek(fd, byte_size*position_of_current_min, SEEK_SET);
            write(fd, ptr1, byte_size);
        }
    }
    free(potential_min);
    free(current_min);
    free(ptr1);
    free(ptr2);
    close(fd);
    return;
}

void sort_lib(char * filename, int rows_count, int byte_size) {
    
    FILE * file;
    file = fopen(filename, "r+");

    int i;
    int j;
    char * potential_min = malloc(sizeof(char));
    int position_of_current_min = 0;
    char * current_min = malloc(sizeof(char));
    char * ptr1 = malloc(byte_size * sizeof(char));
    char * ptr2 = malloc(byte_size * sizeof(char));

    for(i = 0; i < rows_count; i++) {
        fseek(file, byte_size*i, SEEK_SET);
        position_of_current_min = i;
        fread(current_min, sizeof(char), 1, file);
        for(j = i + 1; j < rows_count; j++) {
            fseek(file, byte_size-1, SEEK_CUR);
            fread(potential_min, sizeof(char), 1, file);
            if(current_min[0] > potential_min[0]) {
                *current_min = *potential_min;
                position_of_current_min = j;
            }
        }
        fseek(file, byte_size*i, SEEK_SET);
        fread(ptr1, sizeof(char), byte_size, file);
        fseek(file, byte_size*position_of_current_min, SEEK_SET);
        fread(ptr2, sizeof(char), byte_size, file);
        fseek(file, byte_size*i, SEEK_SET);
        fwrite(ptr2, sizeof(char), byte_size, file);
        fseek(file, byte_size*position_of_current_min, SEEK_SET);
        fwrite(ptr1, sizeof(char), byte_size, file);
    }

    free(potential_min);
    free(current_min);
    free(ptr1);
    free(ptr2);
    fclose(file);
    return;
}

void sort(char * filename, int rows_count, int byte_size, char * flag) {

    if(filename == NULL || rows_count <= 0 || byte_size <= 0) {
        printf("Unsupported operation. Closing program!\n");
        return;        
    }

    if(strcmp(flag, "sys") == 0) {
        sort_sys(filename, rows_count, byte_size);
    }
    else if(strcmp(flag, "lib") == 0) {
        sort_lib(filename, rows_count, byte_size);
    }
    else {
        printf("Unsupported operation. Closing program!\n");
        return;
    }
}


void copy_sys(char * source, char * destination, int rows_count, int byte_size) {

    int fd_s = open(source, O_RDWR);
    int fd_d;
    int i;
    if(fd_s < 0) {
        printf("Error during file creation");
        return;
    } else {
        fd_d = open(destination, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
        if(fd_d < 0) {
            printf("Error during file creation");
            return;
        } else {
            char * buffer = malloc(byte_size * sizeof(char));
            for(i = 0; i < rows_count ; i++) {
                if(read(fd_s, buffer, byte_size) > 0)
                    write(fd_d, buffer, byte_size);
            }
            free(buffer);
        }
    }
    close(fd_s);
    close(fd_d);
    return;
}

void copy_lib(char * source, char * destination, int rows_count, int byte_size) {
    
    FILE * fd_s = fopen(source, "r");
    FILE * fd_d;
    int i;
    if(fd_s < 0) {
        printf("Error during file creation");
        return;
    } else {
        fd_d = fopen(destination, "w+");
        if(fd_d < 0) {
            printf("Error during file creation");
            return;
        } else {
            char * buffer = malloc(byte_size * sizeof(char));
            
            for(i = 0; i < rows_count ; i++) {
                if(fread(buffer, sizeof(char), byte_size, fd_s) > 0)
                    fwrite(buffer, sizeof(char), byte_size, fd_d);
            }
            free(buffer);
        }
    }
    fclose(fd_s);
    fclose(fd_d);
    return;
    
    return;
}

void copy(char * source, char * destination, int rows_count, int byte_size, char * flag) {

    if(source == NULL || destination == NULL || rows_count <= 0 || byte_size <= 0) {
        printf("Unsupported operation. Closing program!\n");
        return;        
    }
    if(strcmp(flag, "sys") == 0) {
        copy_sys(source, destination, rows_count, byte_size);
    }
    else if(strcmp(flag, "lib") == 0) {
        copy_lib(source, destination, rows_count, byte_size);
    }
    else {
        printf("Unsupported operation. Closing program!\n");
        return;
    }
}

int main(int argc, char **argv) {

    int rows_count;
    int byte_size;

    if(5 == argc && strcmp(argv[1], "generate") == 0) {
        sscanf(argv[3], "%d", &rows_count);
        sscanf(argv[4], "%d", &byte_size);
        generate(argv[2], rows_count, byte_size);
    }
    else if(6 == argc && strcmp(argv[1], "sort") == 0) {
        sscanf(argv[3], "%d", &rows_count);
        sscanf(argv[4], "%d", &byte_size);
        sort(argv[2], rows_count, byte_size, argv[5]);
    }
    else if(7 == argc && strcmp(argv[1], "copy") == 0) {
        sscanf(argv[4], "%d", &rows_count);
        sscanf(argv[5], "%d", &byte_size);
        copy(argv[2], argv[3], rows_count, byte_size, argv[6]);
    }
    else {
        printf("Unsupported operation. Closing program!\n");
    }
    return 0;
}