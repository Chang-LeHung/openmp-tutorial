
#include <stdio.h>
#include <omp.h>

int main()
{
  double start = omp_get_wtime();
  for(long i = 0; i < 1000000000; ++i)
  {
    __sync_synchronize();
  }
  printf("time = %lf\n", omp_get_wtime() - start);
  return 0;
}