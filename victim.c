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

#define SIZE 131072
#define NUM_SLOTS 1024
#define SLOT_SIZE 128

int main(void) {
  int fd = open("shared_file", O_RDWR);
  if (fd < 0) {
    perror("open shared_file");
    return 1;
  }

  void *map = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    perror("mmap");
    close(fd);
    return 1;
  }
  close(fd);

  /* pick a random slot in [0, NUM_SLOTS) */
  srand((unsigned)time(NULL) ^ (unsigned)getpid());
  int flag = rand() % NUM_SLOTS;
  printf("flag=%d\n", flag);
  fflush(stdout);

  volatile unsigned char *buf = (volatile unsigned char *)map;

  /* continuously load from the chosen slot */
  for (;;) {
    volatile unsigned char v = buf[flag * SLOT_SIZE];
    (void)v; /* prevent optimizing away the load */
  }

  /* never reached, but keep for completeness */
  munmap(map, SIZE);
  return 0;
}