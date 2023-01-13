

#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main()
{
   int data = 0;
   #pragma omp parallel num_threads(10) shared(data)
   {
      #pragma omp critical
      {
         data++;
         if (data == 6)
            sleep(10);
      }
      printf("tid = %d\n", omp_get_thread_num());
   }
   return 0;
}