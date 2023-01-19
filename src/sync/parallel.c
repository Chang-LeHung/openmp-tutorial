

#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main()
{
   double start = omp_get_wtime();
#pragma omp parallel num_threads(4) default(none)
   {
      int tid = omp_get_thread_num();
      sleep(tid);
   }
   double end = omp_get_wtime();
   printf("execution time : %lf", end - start);
   return 0;
}