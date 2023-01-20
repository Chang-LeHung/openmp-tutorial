
#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main() {

#pragma omp parallel num_threads(4) default(none)
  {
#pragma omp sections
    {
#pragma omp section
      {
#pragma omp critical(A)
        {
          printf("tid = %d time stamp = %lf\n", omp_get_thread_num(), omp_get_wtime());
          sleep(2);
        }
      }
#pragma omp section
      {
#pragma omp critical(B)
        {
          printf("tid = %d time stamp = %lf\n", omp_get_thread_num(), omp_get_wtime());
          sleep(2);
        }
      }

#pragma omp section
      {
#pragma omp critical(C)
        {
          printf("tid = %d time stamp = %lf\n", omp_get_thread_num(), omp_get_wtime());
          sleep(2);
        }
      }

#pragma omp section
      {
#pragma omp critical(D)
        {
          printf("tid = %d time stamp = %lf\n", omp_get_thread_num(), omp_get_wtime());
          sleep(2);
        }
      }
    }
  }
  return 0;
}