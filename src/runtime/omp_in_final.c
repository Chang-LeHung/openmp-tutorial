
#include <stdio.h>
#include <omp.h>

int echo(int n)
{
   int ret;
#pragma omp task shared(ret, n) final(n <= 10)
   {
      if(omp_in_final())
      {
         printf("In final n = %d\n", n);
         ret = n;
      }
      else
      {
         ret = 1 + echo(n - 1);
      }
   }
   return ret;
}

int main()
{
   printf("echo(100) = %d\n", echo(100));
   return 0;
}