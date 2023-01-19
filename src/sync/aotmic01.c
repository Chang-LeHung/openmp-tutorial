
#include <stdio.h>
#include <omp.h>

int main()
{
   int data = 0;
   int c = 1;
   int b = 2;
#pragma omp parallel num_threads(4) default(none) shared(data) private(c, b)
   {
      c = omp_get_thread_num();
      b = omp_get_thread_num() + 1;
#pragma omp atomic
      data += c + b;
   }
   printf("data = %d\n", data);
   return 0;
}