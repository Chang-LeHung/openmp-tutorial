
#include <stdio.h>
#include <omp.h>

int main()
{
   omp_set_nested(1);
   int level = omp_get_level();
   int size = omp_get_team_size(level);
   printf("Non parallel region : level = %d size = %d\n", level, size);

#pragma omp parallel num_threads(5) default(none)
   {
      int level = omp_get_level();
      int size = omp_get_team_size(level);
      printf("level = %d size = %d\n", level, size);

#pragma omp parallel num_threads(2) default(none)
      {
         int level = omp_get_level();
         int size = omp_get_team_size(level);
         printf("level = %d size = %d\n", level, size);

         printf("Up one level team size = %d\n", omp_get_team_size(level - 1));
      }
   }
   return 0;
}