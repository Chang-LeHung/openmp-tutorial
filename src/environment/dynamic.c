

#include <stdio.h>
#include <omp.h>

int main()
{
//   omp_set_dynamic(1);
   omp_set_num_threads(4);
#pragma omp parallel default(none)
   {
      printf("tid = %d\n", omp_get_thread_num());
   }
   return 0;
}