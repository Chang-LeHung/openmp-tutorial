

#include <stdio.h>
#include <omp.h>


typedef struct
{
    /* Make sure total/generation is in a mostly read cacheline, while
       awaited in a separate cacheline.  */
    unsigned total __attribute__((aligned (64)));
    unsigned generation;
    unsigned awaited __attribute__((aligned (64)));
} gomp_barrier_t;

int main()
{
#pragma omp parallel num_threads(4) default(none)
  {
    printf("tid = %d start\n", omp_get_thread_num());
#pragma omp barrier
    printf("tid = %d end\n", omp_get_thread_num());
  }

  printf("sizeof(gomp_barrier_t) = %ld\n", sizeof (gomp_barrier_t));
  return 0;
}