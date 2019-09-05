#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include "zadanie1.h"

double calculate_time(clock_t start, clock_t end) {
    return (double) (end - start) / CLOCKS_PER_SEC;
}

int main(int argc, char ** argv) {

    struct tms tmsstart, tmsend;
    clock_t s = times(&tmsstart);
    int i = 1;
    int value;
    struct FilePath * fp;
    struct WrappedBlock * wp;

    fp = (struct FilePath*) calloc (1, sizeof(struct FilePath));
    wp = create(0);

    while(i<argc) {
        if(strcmp(argv[i], "create_block") == 0 && i+1<argc) {
            free_memory(wp);
            sscanf(argv[++i], "%d", &value);
            wp = create(value);
        }
        else if(strcmp(argv[i], "search_directory") == 0 && i+2<argc) {
            set_directory(fp, argv[++i]);
            set_filename(fp, argv[++i]);
            find(*fp);
            copy_result_to_memory(wp);
        }
        else if(strcmp(argv[i], "remove_block_at_index") == 0 && i+1<argc) {
            sscanf(argv[++i], "%d", &value);
            remove_block_at_index(wp, value);
        }
        else {
            printf("Unsupported operation. Closing program!");
            break;
        }
        i++;
    }

    free(fp);
    free_memory(wp);
    clock_t e = times(&tmsend);

    printf("Czas rzeczywisty to: %f\n", calculate_time(s, e));
    printf("Czas uzytkownika to: %f\n", calculate_time(tmsstart.tms_cutime, tmsend.tms_cutime));
    printf("Czas systemu to: %f\n\n\n", calculate_time(tmsstart.tms_cstime, tmsend.tms_cstime));
    FILE *f = fopen("raport2.txt", "aw");
    fprintf(f, "Czas rzeczywisty to: %f\n", calculate_time(s, e));
    fprintf(f, "Czas uzytkownika to: %f\n", calculate_time(tmsstart.tms_cutime, tmsend.tms_cutime));
    fprintf(f, "Czas systemu to: %f\n\n\n", calculate_time(tmsstart.tms_cstime, tmsend.tms_cstime));
    fclose(f);
    return 0;
}
