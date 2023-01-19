
#include <stdio.h>
#include <omp.h>

int main()
{
   int data = 0;
#pragma omp parallel num_threads(4) default(none) shared(data)
   {
#pragma omp atomic
      data -= omp_get_thread_num();
   }
   printf("data = %d\n", data);
   return 0;
}