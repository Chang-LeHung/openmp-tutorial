

#include <stdio.h>
#include <omp.h>

int factorial(int n)
{
   int s = 1;
   for(int i = 1; i <= n; ++i)
   {
      s *= i;
   }
   return s;
}

int main()
{
   int data[16];
#pragma omp parallel num_threads(16) default(none) shared(data)
   {
      int id = omp_get_thread_num();
      data[id] = factorial(id + 1);
      // 等待上面所有的线程都完成的阶乘的计算
#pragma omp barrier
      long sum = 0;
#pragma omp single
      {
         for(int i = 0; i < 16; ++i)
         {
            sum += data[i];
         }
         printf("final value = %lf\n", (double) sum / 16);
      }
   }
   return 0;
}