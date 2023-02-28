

#include <stdio.h>

int main()
{

#pragma omp parallel num_threads(4) default(none)
  {
#pragma omp task default(none)
    {
       printf("Hello World\n");
    }
  }
  return 0;
}