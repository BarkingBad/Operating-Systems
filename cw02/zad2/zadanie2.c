#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#define __USE_XOPEN
#include <time.h>
#include <ftw.h>


const char format[] = "%Y-%m-%d %H:%M:%S";
time_t GLOBAL_DATE;
char * GLOBAL_FLAG;

char * filetype(int i) {
    switch (i) {
        case DT_BLK:
            return "blk";
            break;
        case DT_CHR:
            return "char";
            break;
        case DT_DIR:
            return "dir";
            break;
        case DT_FIFO:
            return "fifo";
            break;
        case DT_LNK:
            return "lnk";
            break;
        case DT_REG:
            return "file";
            break;
        case DT_SOCK:
            return "sock";
            break;
        default:
            return "unknown"; 
            break;
    }
}

char * nftwfiletype(int i) {
    switch (i) {
        case FTW_D:
            return "dir";
            break;
        case FTW_SL:
            return "lnk";
            break;
        case FTW_F:
            return "file";
            break;
        default:
            return "unknown"; 
            break;
    }
}

static int nftwsearch(const char *actualpath, const struct stat *buf, int ft, struct FTW *ftwbuf) {
    char * atime = (char*) calloc (25, sizeof(char));
    char * mtime = (char*) calloc (25, sizeof(char));

    strcpy(atime, ctime(&buf->st_atime));
    strcpy(mtime, ctime(&buf->st_mtime));

    if(strcmp(GLOBAL_FLAG, "=") == 0 && (GLOBAL_DATE - buf->st_mtime) == 0)
        printf("%s %s %ld %s %s \n", actualpath, nftwfiletype(ft), buf->st_size, atime, mtime);
    else if(strcmp(GLOBAL_FLAG, "<") == 0 && (GLOBAL_DATE - buf->st_mtime) > 0)
        printf("%s %s %ld %s %s \n", actualpath, nftwfiletype(ft), buf->st_size, atime, mtime);
    else if(strcmp(GLOBAL_FLAG, ">") == 0 && (GLOBAL_DATE - buf->st_mtime) < 0)
        printf("%s %s %ld %s %s \n", actualpath, nftwfiletype(ft), buf->st_size, atime, mtime);
    
    free(atime);
    free(mtime);
    return 0;
}

void search(char * dirpath, char * flag, time_t date) {
    

    if(dirpath == NULL || flag == NULL) {
        printf("Unsupported operation. Closing program!\n");
        return;        
    }
    DIR * dir = opendir(dirpath);

    if(dir == NULL) {
        printf("Not found directory!\n%s\n", dirpath);
        return;
    }
    struct dirent * next;
    struct stat buf;

    char newpath[PATH_MAX+1];
    char actualpath[PATH_MAX+1];
    char * atime = (char*) calloc (25, sizeof(char));
    char * mtime = (char*) calloc (25, sizeof(char));

    while( (next = readdir(dir)) != NULL ) {
        strcpy(newpath, dirpath);
        strcat(newpath, "/");
        strcat(newpath, next->d_name);
        
        if (strcmp(next->d_name, ".") == 0 || strcmp(next->d_name, "..") == 0) {
            continue;
        }

        lstat(newpath, &buf);
        char * ft = filetype(next->d_type);

        if(strcmp(ft, "dir") == 0 && !S_ISLNK(buf.st_mode)) {
            search(newpath, flag, date);
        }
        
        strcpy(atime, ctime(&buf.st_atime));
        strcpy(mtime, ctime(&buf.st_mtime));

        realpath(newpath, actualpath);
        if(strcmp(flag, "=") == 0 && (date - buf.st_mtime) == 0)
            printf("%s %s %ld %s %s \n", actualpath, ft, buf.st_size, atime, mtime);
        else if(strcmp(flag, "<") == 0 && (date - buf.st_mtime) > 0)
            printf("%s %s %ld %s %s \n", actualpath, ft, buf.st_size, atime, mtime);
        else if(strcmp(flag, ">") == 0 && (date - buf.st_mtime) < 0)
            printf("%s %s %ld %s %s \n", actualpath, ft, buf.st_size, atime, mtime);


    }
    free(atime);
    free(mtime);

    free(next);
    closedir(dir);    
}


int main(int argc, char ** argv) {
    struct tm *timestamp = malloc(sizeof(struct tm));

    strptime(argv[3], format, timestamp);
    time_t date = mktime(timestamp);

    printf("\n\nDIR/STAT\n\n");
    search(argv[1], argv[2], date);

    printf("\n\nNFTW\n\n");


    char actualpath[PATH_MAX+1];
    realpath(argv[1], actualpath);
    GLOBAL_DATE = mktime(timestamp);
    GLOBAL_FLAG = argv[2];
    nftw(actualpath, nftwsearch, 1000, 1);

    free(timestamp);
}