
#include <stdio.h>
#include <omp.h>

int main()
{
   printf("0> Not In parallel region value = %d\n",
          omp_in_parallel());

   // 在这里函数返回的 false 在 C 语言当中返回值等于 int 类型的 0
   if (omp_in_parallel())
   {
      printf("1> In parallel region value = %d\n",
             omp_in_parallel());
   }
   #pragma omp parallel num_threads(1) default(none)
   {
      // 这里函数的返回值是 1 在 C 语言当中对应 int 类型的 1
      if (omp_in_parallel())
      {
         printf("2> In parallel region value = %d\n",
                omp_in_parallel());
      }
   }
   return 0;
}