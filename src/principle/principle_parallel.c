

#include <stdio.h>
#include "omp.h"

int main()
{
  printf("start\n");
#pragma omp parallel num_threads(4) default(none)
  {
    printf("tid = %d\n", omp_get_thread_num());
  }

  printf("finished\n");
  return 0;
}