

#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel for num_threads(4) default(none)schedule(dynamic)
  for(int i = 0; i < 12; ++i)
  {
    printf("i = %d tid = %d\n", i, omp_get_thread_num());
  }
  return 0;
}