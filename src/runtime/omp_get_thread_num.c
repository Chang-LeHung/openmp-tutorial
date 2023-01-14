
#include <stdio.h>
#include <omp.h>

int main()
{
   printf("Non parallel region: omp_get_thread_num() = %d\n", omp_get_thread_num());

#pragma omp parallel num_threads(5) default(none)
   {
      printf("In parallel region: omp_get_thread_num() = %d\n", omp_get_thread_num());
#pragma omp master
      {
         printf("In parallel region master: omp_get_thread_num() = %d\n", omp_get_thread_num());
      }
   }

   return 0;
}