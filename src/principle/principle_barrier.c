

#include <stdio.h>
#include <omp.h>
#include <bits/pthreadtypes.h>

int main()
{
#pragma omp parallel num_threads(4) default(none)
  {
    printf("tid = %d start\n", omp_get_thread_num());
#pragma omp barrier
    printf("tid = %d end\n", omp_get_thread_num());
  }

  printf("sizeof(unsigned) = %ld\n", sizeof (unsigned));
  return 0;
}