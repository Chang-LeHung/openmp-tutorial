

#include <stdio.h>
#include <omp.h>


int main() {
   omp_set_nested(1);
   #pragma omp parallel num_threads(2) default(none)
   {
      printf("1> tid = %d level = %d\n", omp_get_thread_num(),
             omp_get_active_level());
      #pragma omp parallel num_threads(2) default(none)
      {
         printf("2> tid = %d level = %d\n", omp_get_thread_num(),
                omp_get_active_level());
         #pragma omp parallel num_threads(2) default(none)
         {
            printf("3> tid = %d level = %d\n", omp_get_thread_num(),
                   omp_get_active_level());
            #pragma omp parallel num_threads(2) default(none)
            {
               printf("4> tid = %d level = %d\n", omp_get_thread_num(),
                      omp_get_active_level());
               #pragma omp parallel num_threads(2) default(none)
               {
                  printf("5> tid = %d level = %d\n", omp_get_thread_num(),
                         omp_get_active_level());
               }
            }
         }
      }
      printf("6> tid = %d level = %d\n", omp_get_thread_num(),
             omp_get_active_level());
   }
   return 0;
}