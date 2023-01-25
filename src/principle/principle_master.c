
#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel num_threads(4) default(none)
  {
#pragma omp master
    {
      printf("I am master and my tid = %d\n", omp_get_thread_num());
    }
  }
  return 0;
}