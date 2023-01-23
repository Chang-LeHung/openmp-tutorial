

#include <stdio.h>
#include "omp.h"

int main()
{
  int data = 100;
  int two  = -100;
  printf("start\n");
#pragma omp parallel num_threads(4) default(none) shared(data, two)
  {
    printf("tid = %d data = %d two = %d\n", omp_get_thread_num(), data, two);
  }

  printf("finished\n");
  return 0;
}