

#include <stdio.h>
#include <omp.h>


int main()
{
   omp_set_nested(1);
   #pragma omp parallel for num_threads(2) default(none)
   for (int i = 0; i < 2; ++i)
   {
      printf("Outer i = %d tid = %d leve = %d num_threads = %d\n",
             i, omp_get_thread_num(), omp_get_active_level(),
             omp_get_num_threads());
      #pragma omp parallel num_threads(3) default(none)
      {
         printf("Inner tid = %d leve = %d num_threads = %d\n",
                omp_get_thread_num(), omp_get_active_level(),
                omp_get_num_threads());
      }
   }
   return 0;
}