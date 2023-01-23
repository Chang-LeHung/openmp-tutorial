

#include <stdio.h>
#include "omp.h"

int main()
{

#pragma omp parallel num_threads(4) default(none)
  {
#pragma omp task default(none)
    {
      int n = 1;
      while (n < 10)
      {
#pragma omp taskyield
        ++n;
      }
    }
  }
  return 0;
}