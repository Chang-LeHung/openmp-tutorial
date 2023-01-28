

#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel num_threads(4) default(none)
  {
#pragma omp single
    {
      printf("Hello World\n");
    }

    printf("tid = %d\n", omp_get_thread_num());
  }
  return 0;
}