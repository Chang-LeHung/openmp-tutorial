
#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel default(none)
   {
   omp_set_default_device(16);
      while (1);
   }
   return 0;
}