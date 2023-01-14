
#include <stdio.h>
#include <omp.h>

int main()
{
   double s = omp_get_wtime();
   #pragma omp parallel num_threads(1) default(none)
   {
      int arr[1000000];
      for (int i = 0; i < 1000000; ++i) {arr[i] = i;}
   }
   double e = omp_get_wtime();
   printf("cost time = %lf\n", e - s);
   return 0;
}