#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "zadanie1.h"

struct WrappedBlock *create(int size) {
    struct WrappedBlock *wp;
    wp = (struct WrappedBlock*) calloc (1, sizeof(struct WrappedBlock));
    wp->table = (char**) calloc (size, sizeof(char*));
    wp->size = size;
    wp->index = 0;
    return wp;
}

void set_directory(struct FilePath * fp, char * directory) {
    fp->dir = directory;
}

void set_filename(struct FilePath * fp, char * filename) {
    fp->filename = filename;
}

void find(struct FilePath fp) {
    char * function;
    if(fp.dir == NULL || fp.filename == NULL) return;
    function = (char*) calloc (34 + strlen(fp.dir) + strlen(fp.filename), sizeof(char));
    strcpy(function, "find ");
    strcat(function, fp.dir);
    strcat(function, " ");
    strcat(function, fp.filename);
    strcat(function, " > /tmp/result 2>/dev/null");
    system(function);
    free(function);
    return;
}

int copy_result_to_memory(struct WrappedBlock *wp) {
    FILE *fp;
    int size;
    if(wp->table == NULL) return -1;
    if(wp->index >= wp->size) return -1;
    fp = fopen("/tmp/result", "r");
    if(fp == NULL) return -1;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    wp->table[wp->index] = (char*) calloc (size+1, sizeof(char));
    fread(wp->table[wp->index], sizeof(char), size, fp);
    fclose(fp);
    wp->index++;
    return wp->index-1;
}

void remove_block_at_index(struct WrappedBlock *wp, int index) {
    if(wp->size < index || index < 0) {
        printf("Cannot remove membory block on given index");
        return;
    }
    free(wp->table[index]);
    wp->table[index] = NULL;
}

void free_memory(struct WrappedBlock *wp) {
    if(wp == NULL) return;
    int i;
    for(i = 0; i < wp->index; i++) {
        free(wp->table[i]);
    }

    if(wp->table != NULL) free(wp->table);
    free(wp);
}


