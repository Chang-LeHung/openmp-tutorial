

#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel num_threads(2) default(none)
   {
#pragma omp for
      for(int i = 0; i < 12; ++i)
      {
         printf("i = %d tid = %d\n", i, omp_get_thread_num());
      }
   }

   return 0;
}