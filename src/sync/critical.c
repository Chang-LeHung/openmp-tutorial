

#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main()
{
   int data = 0;
   #pragma omp parallel num_threads(10) shared(data) default(none)
   {
      #pragma omp critical
      {
         data++;
         if (data == 6)
            sleep(10);
      }
      printf("1> tid = %d\n", omp_get_thread_num());
      printf("2> tid = %d\n", omp_get_thread_num());
      printf("3> tid = %d\n", omp_get_thread_num());
   }
   return 0;
}