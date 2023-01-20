

#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main()
{
#pragma omp parallel num_threads(4) default(none)
  {
    double start = omp_get_wtime();
#pragma omp single
    {
      printf("In single tid = %d ", omp_get_thread_num());
      sleep(5);
      printf("cost time = %lf\n", omp_get_wtime() - start);
    }

    printf("Out single tid = %d cost time = %lf\n", omp_get_thread_num(), omp_get_wtime() - start);
  }
  return 0;
}