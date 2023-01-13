

#include <stdio.h>
#include <omp.h>

int main()
{
   int data = 10;
#pragma omp parallel num_threads(10) shared(data) default(none)
   {
#pragma omp flush(data)
      if (data & 1)
         data++;
   }
   return 0;
}