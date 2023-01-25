
#include <stdio.h>
#include <omp.h>

int main()
{
  int data = 0;
#pragma omp parallel num_threads(4) default(none) shared(data)
  {
#pragma omp critical(A)
    {
      data++;
    }
  }
  printf("data = %d\n", data);
  return 0;
}