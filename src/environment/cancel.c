

#include <stdio.h>
#include <omp.h>

int main()
{

   int s = omp_get_cancellation();
   printf("%d\n", s);
#pragma omp parallel num_threads(8) default(none)
   {
      if (omp_get_thread_num() == 2)
      {
#pragma omp cancel parallel
      }
      printf("tid = %d\n", omp_get_thread_num());

   }
   return 0;
}