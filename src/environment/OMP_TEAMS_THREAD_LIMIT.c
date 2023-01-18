
#include <stdio.h>
#include <omp.h>

int main()
{

#pragma omp parallel num_threads(8) default(none)
   {
      printf("tid = %d\n", omp_get_thread_num());
   }
   return 0;
}