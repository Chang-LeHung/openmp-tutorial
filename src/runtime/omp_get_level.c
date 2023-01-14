
#include <stdio.h>
#include <omp.h>

int main()
{
   omp_set_nested(1);
   omp_set_max_active_levels(2);

#pragma omp parallel num_threads(2) default(none)
   {
      printf("1> omp_get_level = %d omp_get_active_level = %d\n", omp_get_level(), omp_get_active_level());
#pragma omp parallel num_threads(2) default(none)
      {
         printf("2> omp_get_level = %d omp_get_active_level = %d\n", omp_get_level(), omp_get_active_level());
#pragma omp parallel num_threads(2) default(none)
         {
            printf("3> omp_get_level = %d omp_get_active_level = %d\n", omp_get_level(), omp_get_active_level());

         }
      }
   }
   return 0;
}