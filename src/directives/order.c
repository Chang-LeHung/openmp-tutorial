

#include <stdio.h>
#include <omp.h>

int main()
{
   int arr[10];
   arr[0] = 0;
#pragma omp parallel for num_threads(10) shared(arr) ordered default(none)
   for (int i = 1; i < 10; i++) {
#pragma omp ordered
      arr[i] = arr[i - 1] - 1;
       printf("tid = %d i = %d arr[%d] = %d\n",
              omp_get_thread_num(), i, i, arr[i]);
   }

   return 0;
}