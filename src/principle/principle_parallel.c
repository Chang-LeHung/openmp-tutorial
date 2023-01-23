

#include <stdio.h>
#include "omp.h"

int main()
{
  int data = 100;
  printf("start\n");
#pragma omp parallel num_threads(4) default(none) shared(data)
  {
    printf("tid = %d data = %d\n", omp_get_thread_num(), data);
  }

  printf("finished\n");
  return 0;
}