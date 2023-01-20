
#include <stdio.h>
#include <omp.h>

int main()
{
   int n;
   scanf("%d", &n);
#pragma omp parallel num_threads(n) default(none)
   {
      printf("tid = %d\n", omp_get_thread_num());
   }
   return 0;
}