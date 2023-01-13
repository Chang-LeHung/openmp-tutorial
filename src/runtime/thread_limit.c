
#include <stdio.h>
#include <omp.h>
#include <stdint.h>


int main()
{
   omp_set_num_threads(5);
   printf("Hello World\n");
   #pragma omp parallel num_threads(2) default(none)
   {
      printf("limit = %d max_threads = %d\n", omp_get_thread_limit(),
             omp_get_max_threads());
   }

   printf("%d\n", INT32_MAX);
   return 0;
}