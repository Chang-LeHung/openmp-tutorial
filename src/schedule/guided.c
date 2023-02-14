
#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel for num_threads(4) schedule(guided, 4) default(none)
   for (int i = 0; i < 20; ++i)
   {
      printf("tid %d i = %d\n", omp_get_thread_num(), i);
   }
   return 0;
}