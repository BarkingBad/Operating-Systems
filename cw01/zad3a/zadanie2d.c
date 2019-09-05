#define _POSIX_C_SOURCE 199309L

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include "zadanie1.h"

void *dl_handle;
typedef void *(*arbitrary)();

double calculate_time(clock_t start, clock_t end) {
    return (double) (end - start) / CLOCKS_PER_SEC;
}

int main(int argc, char ** argv) {

    dl_handle = dlopen("./zadanie1.so", RTLD_LAZY);
    if (!dl_handle) {
        printf("!!! %s\n", dlerror());
        return 0;
    }

    arbitrary fcreate;
    *(void **) (&fcreate) = dlsym(dl_handle, "create");

    arbitrary ffree_memory;
    *(void **) (&ffree_memory) = dlsym(dl_handle, "free_memory");
    
    arbitrary fset_directory;
    *(void **) (&fset_directory) = dlsym(dl_handle, "set_directory");
    
    arbitrary fset_filename;
    *(void **) (&fset_filename) = dlsym(dl_handle, "set_filename");

    arbitrary ffind;
    *(void **) (&ffind) = dlsym(dl_handle, "find");

    arbitrary fcopy_result_to_memory;
    *(void **) (&fcopy_result_to_memory) = dlsym(dl_handle, "copy_result_to_memory");

    arbitrary fremove_block_at_index;
    *(void **) (&fremove_block_at_index) = dlsym(dl_handle, "remove_block_at_index");


    struct tms tmsstart, tmsend;
    clock_t s = times(&tmsstart);
    int i = 1;
    int value;
    struct FilePath * fp;
    struct WrappedBlock * wp;

    fp = (struct FilePath*) calloc (1, sizeof(struct FilePath));
    wp = fcreate(0);

    while(i<argc) {
        if(strcmp(argv[i], "create_block") == 0 && i+1<argc) {
            ffree_memory(wp);
            sscanf(argv[++i], "%d", &value);
            wp = fcreate(value);
        }
        else if(strcmp(argv[i], "search_directory") == 0 && i+2<argc) {
            fset_directory(fp, argv[++i]);
            fset_filename(fp, argv[++i]);
            ffind(*fp);
            fcopy_result_to_memory(wp);
        }
        else if(strcmp(argv[i], "remove_block_at_index") == 0 && i+1<argc) {
            sscanf(argv[++i], "%d", &value);
            fremove_block_at_index(wp, value);
        }
        else {
            printf("Unsupported operation. Closing program!");
            break;
        }
        i++;
    }

    free(fp);
    ffree_memory(wp);
    clock_t e = times(&tmsend);

    printf("Czas rzeczywisty to: %f\n", calculate_time(s, e));
    printf("Czas uzytkownika to: %f\n", calculate_time(tmsstart.tms_cutime, tmsend.tms_cutime));
    printf("Czas systemu to: %f\n\n\n", calculate_time(tmsstart.tms_cstime, tmsend.tms_cstime));
    FILE *f = fopen("raport2.txt", "aw");
    fprintf(f, "Czas rzeczywisty to: %f\n", calculate_time(s, e));
    fprintf(f, "Czas uzytkownika to: %f\n", calculate_time(tmsstart.tms_cutime, tmsend.tms_cutime));
    fprintf(f, "Czas systemu to: %f\n\n\n", calculate_time(tmsstart.tms_cstime, tmsend.tms_cstime));
    fclose(f);
dlclose(dl_handle);
    return 0;
}
