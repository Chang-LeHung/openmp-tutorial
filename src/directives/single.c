

#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main()
{
#pragma omp parallel num_threads(2) default(none)
   {
      printf("1> tid = %d\n", omp_get_thread_num());

#pragma omp single
      {
         printf("tid in single = %d\n", omp_get_thread_num());
         sleep(5);
      }

      printf("end tid = %d\n", omp_get_thread_num());
   }
   return 0;
}