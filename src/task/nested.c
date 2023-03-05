

#include <stdio.h>
#include <omp.h>

int main()
{
//   omp_set_nested(1);
#pragma omp parallel num_threads(4) default(none)
   {
      int data = omp_get_thread_num();
      printf("outer from tid = %d\n", data);
#pragma omp parallel num_threads(4) default(none)
      {
//         printf("hello from tid = %d\n", omp_get_thread_num());
      }
      printf("outer from tid = %d\n", data);
   }
   return 0;
}