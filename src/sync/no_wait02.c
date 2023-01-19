

#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main()
{
   double start = omp_get_wtime();
#pragma omp parallel num_threads(4) default(none) shared(start)
   {
#pragma omp single nowait
      sleep(5);
      printf("tid = %d spent %lf s\n", omp_get_thread_num(), omp_get_wtime() - start);
   }
   double end = omp_get_wtime();
   printf("execution time : %lf", end - start);
   return 0;
}