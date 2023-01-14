
#include <stdio.h>
#include <omp.h>


int main() {
#pragma omp parallel sections num_threads(2) default(none)
   {
#pragma omp section
      {
         printf("section 1 tid = %d\n", omp_get_thread_num());
      }

#pragma omp section
      {
         printf("section 2 tid = %d\n", omp_get_thread_num());
      }
#pragma omp section
      {
         printf("section 3 tid = %d\n", omp_get_thread_num());
      }
#pragma omp section
      {
         printf("section 4 tid = %d\n", omp_get_thread_num());
      }
   }
   return 0;
}