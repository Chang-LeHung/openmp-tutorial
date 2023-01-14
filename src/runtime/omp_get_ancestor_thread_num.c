
#include <stdio.h>
#include <omp.h>

int main()
{
   omp_set_nested(1);
#pragma omp parallel num_threads(2) default(none)
   {
      printf("1> tid = %d omp_get_ancestor_thread_num = %d\n",
             omp_get_thread_num(), omp_get_ancestor_thread_num(omp_get_active_level() - 1));
#pragma omp parallel num_threads(2) default(none)
      {
         printf("2> tid = %d omp_get_ancestor_thread_num = %d\n",
                omp_get_thread_num(), omp_get_ancestor_thread_num(omp_get_active_level() - 1));
#pragma omp parallel num_threads(2) default(none)
         {
            printf("3> tid = %d omp_get_ancestor_thread_num = %d\n",
                   omp_get_thread_num(), omp_get_ancestor_thread_num(omp_get_active_level() - 1));
         }
      }

   }
   return 0;
}