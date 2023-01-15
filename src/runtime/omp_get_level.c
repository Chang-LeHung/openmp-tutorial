
#include <stdio.h>
#include <omp.h>
#include <pthread.h>

int main()
{
   omp_set_nested(1);
   omp_set_max_active_levels(3);

#pragma omp parallel num_threads(2) default(none)
   {
      printf("1> omp_get_level = %d omp_get_active_level = %d pthread_self = %ld\n", omp_get_level(), omp_get_active_level(), pthread_self());
#pragma omp parallel num_threads(2) default(none)
      {
         printf("2> omp_get_level = %d omp_get_active_level = %d pthread_self = %ld\n", omp_get_level(), omp_get_active_level(), pthread_self());
#pragma omp parallel num_threads(2) default(none)
         {
            printf("3> omp_get_level = %d omp_get_active_level = %d pthread_self = %ld\n", omp_get_level(), omp_get_active_level(), pthread_self());

         }
      }
   }
   return 0;
}