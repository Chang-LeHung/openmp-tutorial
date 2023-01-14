

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
      }
      #pragma omp critical
      {
         data--;
      }
   }
   printf("data = %d\n", data);
   return 0;
}