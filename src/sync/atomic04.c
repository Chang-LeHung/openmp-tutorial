
#include <stdio.h>
#include <omp.h>

int main()
{
   int data = 100;
#pragma omp parallel num_threads(4) default(none) shared(data)
   {
      int a = omp_get_thread_num() + 1;
#pragma omp atomic
      data /= a;
   }
   printf("data = %d\n", data);
   return 0;
}