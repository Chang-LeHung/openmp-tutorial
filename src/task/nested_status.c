

#include <stdio.h>
#include <omp.h>
#include <pthread.h>

int main()
{
//   omp_set_nested(1);
#pragma omp parallel num_threads(4) default(none)
   {
      int data = omp_get_thread_num();
      printf("outer from tid = %d tid = %ld\n", data, pthread_self());
#pragma omp parallel num_threads(1) default(none)
      {
         printf("outer from tid = %d tid = %ld\n", omp_get_thread_num(), pthread_self());
      }
   }
   return 0;
}