

#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel num_threads(4) default(none)
  {
#pragma omp single nowait
    {
      printf("Hello World\n");
    }
  }
  return 0;
}